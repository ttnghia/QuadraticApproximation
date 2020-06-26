/**
 * Copyright 2020 Nghia Truong <nghiatruong.vn@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Corrade/Utility/Assert.h>
#include <Magnum/Math/Color.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/Primitives/UVSphere.h>

#include "DrawableObjects/PickableObject.h"
#include "DrawableObjects/FlatShadeObject.h"
#include "DrawableObjects/Curves/Polyline.h"
#include "DrawableObjects/Curves/CubicBezier.h"
#include "DrawableObjects/Curves/QuadraticApproximatingCubic.h"

#include <fstream>
#include <sstream>

#include "QuadraticCurveApproximation.h"

/****************************************************************************************************/
QuadraticCurveApproximation::QuadraticCurveApproximation(Scene3D* const                     scene,
                                                         SceneGraph::DrawableGroup3D* const drawables) :
    m_Scene(scene), m_Drawables(drawables) {
    /* Curves config */
    cubicBezierConfig.color     = Color3{ 0.0f, 0.0f, 1.0f };
    cubicBezierConfig.thickness = 10.0f;
    cubicBezierConfig.bRenderControlPoints = false;

    quadC1BezierConfig.color     = Color3{ 1.0f, 0.0f, 0.0f };
    quadC1BezierConfig.thickness = 5.0f;
    quadC1BezierConfig.bEnabled  = false; /* disable by default */

    m_Polylines = new Polyline(m_Scene, Color3{ 1.0f, 1.0f, 0.0f });

    /* Variable for pickable control points */
    m_MeshSphere = MeshTools::compile(Primitives::uvSphereSolid(8, 16));

    /* Update drawable control points and generate curve data */
    loadControlPoints();
}

/****************************************************************************************************/
QuadraticCurveApproximation& QuadraticCurveApproximation::draw(SceneGraph::Camera3D& camera,
                                                               const Vector2i&       viewport) {
    auto drawCurves = [&](auto& curves) {
                          for(auto& curve: curves) {
                              curve->draw(camera, viewport);
                          }
                      };
    m_Polylines->draw(camera, viewport);
    drawCurves(m_CubicBezierCurves);
    drawCurves(m_QuadraticC1Curves);
    return *this;
}

/****************************************************************************************************/
void QuadraticCurveApproximation::updateCurveConfigs() {
    auto update = [] (auto& curves, const auto& config) {
                      for(auto& curve :curves) {
                          curve->color()               = config.color;
                          curve->thickness()           = config.thickness;
                          curve->controlPointRadius()  = config.controlPointRadius;
                          curve->renderControlPoints() = config.bRenderControlPoints;
                          curve->enabled()             = config.bEnabled;
                      }
                  };

    update(m_CubicBezierCurves, cubicBezierConfig);
    update(m_QuadraticC1Curves, quadC1BezierConfig);
}

/****************************************************************************************************/
void QuadraticCurveApproximation::setDataPoint(uint32_t selectedIdx, const Vector3& point) {
    const auto it = m_mDrawableIdxToPointIdx.find(selectedIdx);
    CORRADE_INTERNAL_ASSERT(it != m_mDrawableIdxToPointIdx.end()
                            && it->second < m_DataPoints.size());
    m_DataPoints[it->second] = point;
}

/****************************************************************************************************/
void QuadraticCurveApproximation::resetDataPoints() {
    m_DataPoints = m_DataPoints_t0;
    computeBezierControlPoints();
    generateCurves();
}

/****************************************************************************************************/
void QuadraticCurveApproximation::computeBezierControlPoints() {
    if(m_bBezierFromCatmullRom) {
        computeBezierControlPointsFromCatmullRom();
        cubicBezierConfig.bRenderControlPoints = true;
    } else {
        m_BezierControlPoints = m_DataPoints;
        cubicBezierConfig.bRenderControlPoints = false;
    }
}

/****************************************************************************************************/
void QuadraticCurveApproximation::generateCurves() {
    const auto nCurrentCurves = m_CubicBezierCurves.size();
    const auto nCurves        = m_BezierControlPoints.size() / 4;

    for(size_t i = nCurrentCurves; i < nCurves; ++i) {
        m_CubicBezierCurves.push_back(new CubicBezier(m_Scene, m_Subdivision,
                                                      cubicBezierConfig.color,
                                                      cubicBezierConfig.thickness,
                                                      cubicBezierConfig.bRenderControlPoints,
                                                      false,
                                                      cubicBezierConfig.controlPointRadius));
        m_QuadraticC1Curves.push_back(new QuadraticApproximatingCubic(m_Scene, m_Subdivision >> 1,
                                                                      quadC1BezierConfig.color,
                                                                      quadC1BezierConfig.thickness,
                                                                      quadC1BezierConfig.bRenderControlPoints,
                                                                      false,
                                                                      quadC1BezierConfig.controlPointRadius));
        m_QuadraticC1Curves.back()->enabled() = quadC1BezierConfig.bEnabled;
    }

    /* Reduce number of curves, if applicable */
    m_CubicBezierCurves.resize(nCurves);
    m_QuadraticC1Curves.resize(nCurves);

    /* Update polyline and quadratic curves */
    updatePolylines();
    updateCurveControlPoints();

    /* Update drawable points */
    updateDrawablePoints();
}

/****************************************************************************************************/
void QuadraticCurveApproximation::updatePolylines() {
    m_Polylines->setControlPoints(m_BezierControlPoints);
}

/****************************************************************************************************/
void QuadraticCurveApproximation::updateDrawablePoints() {
    size_t oldSize = m_DrawablePoints.size();
    m_DrawablePoints.resize(m_DataPoints.size());

    for(size_t i = oldSize; i < m_DataPoints.size(); ++i) {
        auto& newPoint = m_DrawablePoints[i];
        newPoint = new PickableObject(m_SphereShader,
                                      cubicBezierConfig.color,
                                      m_MeshSphere,
                                      m_Scene,
                                      m_Drawables);
        newPoint->setColor(cubicBezierConfig.color);
        m_mDrawableIdxToPointIdx[newPoint->idx()] = i;
    }

    for(size_t i = 0; i < m_DrawablePoints.size(); ++i) {
        m_DrawablePoints[i]->setTransformation(Matrix4::translation(m_DataPoints[i]) *
                                               Matrix4::scaling(Vector3(cubicBezierConfig.controlPointRadius * 1.2f)));
    }
}

/****************************************************************************************************/
void QuadraticCurveApproximation::updateCurveControlPoints() {
    if(m_BezierControlPoints.size() < 4) {
        return;
    }

    const auto nCurves = m_BezierControlPoints.size() / 4;

    for(size_t idx = 0; idx < nCurves; ++idx) {
        VPoints B = {
            m_BezierControlPoints[idx * 4],
            m_BezierControlPoints[idx * 4 + 1],
            m_BezierControlPoints[idx * 4 + 2],
            m_BezierControlPoints[idx * 4 + 3]
        };
        m_CubicBezierCurves[idx]->setControlPoints(B);

        /* Compute control points of the quadratic C1 curves */
        VPoints Q(5);
        Q[0] = B[0];
        Q[4] = B[3];
        Q[1] = B[0] + 1.5f * m_gamma * (B[1] - B[0]);
        Q[3] = B[3] - 1.5f * (1.0f - m_gamma) * (B[3] - B[2]);
        Q[2] = (1.0f - m_gamma) * Q[1] + m_gamma * Q[3];
        m_QuadraticC1Curves[idx]->setControlPoints(Q);
    }
}

/****************************************************************************************************/
void QuadraticCurveApproximation::computeCurves() {
    auto compute = [&](auto& curves, int subdiv) {
                       for(auto& curve: curves) {
                           curve->subdivision() = subdiv;
                           curve->recomputeCurve();
                       }
                   };
    m_Polylines->recomputeCurve();
    compute(m_CubicBezierCurves, m_Subdivision);
    compute(m_QuadraticC1Curves, m_Subdivision >> 1);
}

/****************************************************************************************************/
void QuadraticCurveApproximation::loadControlPoints() {
    std::ifstream file("points.txt");
    if(!file.is_open()) {
        Fatal() << "Cannot find point.txt";
    }

    m_DataPoints.resize(0);
    std::string line;
    std::string x, y, z;
    while(std::getline(file, line)) {
        line.erase(line.find_last_not_of(" \n\r\t") + 1);

        if(line == "") {
            continue;
        }

        if(line.find("//") != std::string::npos) {
            continue;
        }

        std::istringstream iss(line);
        iss >> x >> y >> z;
        m_DataPoints.push_back(Vector3{ stof(x), stof(y), stof(z) });
    }
    file.close();
    // Debug() << "Loaded" << m_DataPoints.size() << "points";

    /* Update drawable points and curves */
    m_bBezierFromCatmullRom = (m_DataPoints.size() % 4) != 0;
    computeBezierControlPoints();
    generateCurves();
}

/****************************************************************************************************/
void QuadraticCurveApproximation::saveControlPoints() {
    std::ofstream file("points.txt");
    int           i = 0;
    for(auto& p : m_DataPoints) {
        if(i % 4 == 0) {
            file << "\n// Control point of curve #" << i / 4 << "\n";
        }
        ++i;
        file << p.x() << " " << p.y() << " " << p.z() << "\n";
    }
    file.close();
}

/****************************************************************************************************/
void QuadraticCurveApproximation::computeBezierControlPointsFromCatmullRom() {
    auto convert =
        [](const VPoints& points, size_t idx, float alpha) {
            const Vector3* P  = &points[idx];
            const auto     d1 = (P[0] - P[1]).length();
            const auto     d2 = (P[1] - P[2]).length();
            const auto     d3 = (P[2] - P[3]).length();

            const auto d1_alpha  = std::pow(d1, alpha);
            const auto d2_alpha  = std::pow(d2, alpha);
            const auto d3_alpha  = std::pow(d3, alpha);
            const auto d1_2alpha = std::pow(d1, 2.0f * alpha);
            const auto d2_2alpha = std::pow(d2, 2.0f * alpha);
            const auto d3_2alpha = std::pow(d3, 2.0f * alpha);

            /* Compute control points of the cubic Bezier curve */
            VPoints B(4);
            B[0] = P[1];
            B[3] = P[2];
            B[1] = (d1_2alpha * P[2] - d2_2alpha * P[0] +
                    (2.0f * d1_2alpha + 3.0f * d1_alpha * d2_alpha + d2_2alpha) * P[1]) /
                   (3.0f * d1_alpha * (d1_alpha + d2_alpha));
            B[2] = (d3_2alpha * P[1] - d2_2alpha * P[3] +
                    (2.0f * d3_2alpha + 3.0f * d3_alpha * d2_alpha + d2_2alpha) * P[2]) /
                   (3.0f * d3_alpha * (d3_alpha + d2_alpha));
            return B;
        };

    m_BezierControlPoints.resize(0);
    for(size_t i = 0; i < m_DataPoints.size() - 3; ++i) {
        const auto bpoints = convert(m_DataPoints, i, m_CatmullRom_Alpha);
        m_BezierControlPoints.insert(m_BezierControlPoints.end(), bpoints.begin(), bpoints.end());
    }
}
