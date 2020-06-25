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

#include "QuadraticCurveApproximation.h"

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
#include "DrawableObjects/Curves/CubicBezierCurve.h"
#include "DrawableObjects/Curves/QuadraticBezierCurve.h"

#include <fstream>
#include <sstream>

using namespace Magnum::Math::Literals;

/****************************************************************************************************/
QuadraticCurveApproximation::QuadraticCurveApproximation(Scene3D* const                     scene,
                                                         SceneGraph::DrawableGroup3D* const drawableGroup) :
    m_Scene(scene), m_DrawableGroup(drawableGroup) {
    /* Curves config */
    cubicBezierConfig.color     = Color3{ 0.0f, 0.0f, 1.0f };
    cubicBezierConfig.thickness = 10.0f;
    cubicBezierConfig.bRenderControlPoints = false;
    cubicBezierConfig.controlPointRadius   = 0.02f;

    quadC1BezierConfig.color     = Color3{ 1.0f, 0.0f, 0.0f };
    quadC1BezierConfig.thickness = 5.0f;
    quadC1BezierConfig.bRenderControlPoints = true;
    quadC1BezierConfig.controlPointRadius   = 0.02f;
    quadC1BezierConfig.bEnabled = false;

    /* Drawable curves */
    m_Polylines = new Polyline(m_Scene, Color3{ 1.0f, 1.0f, 0.0f });

    /* Disable by default */
    for(auto& curve: m_QuadraticC1Curves) {
        curve->enabled() = false;
    }

    /* Variable for pickable control points */
    m_MeshSphere = MeshTools::compile(Primitives::uvSphereSolid(8, 16));
    m_SphereShader.emplace(Shaders::Phong::Flag::ObjectId);

    /* Update drawable control points and generate curve data */
    loadControlPoints();
}

/****************************************************************************************************/
QuadraticCurveApproximation& QuadraticCurveApproximation::draw(SceneGraph::Camera3D& camera,
                                                               const Vector2i&       viewport) {
    /* Firstly draw curves */
    m_Polylines->drawCurve(camera, viewport);
    m_Polylines->drawControlPoints(camera);

    auto drawCurves = [&](auto& curves) {
                          for(auto& curve: curves) {
                              curve->drawCurve(camera, viewport);
                              curve->drawControlPoints(camera);
                          }
                      };
    drawCurves(m_CubicBezierCurves);
    drawCurves(m_QuadraticC1Curves);

    return *this;
}

/****************************************************************************************************/
void QuadraticCurveApproximation::setControlPoint(uint32_t selectedIdx, const Vector3& point) {
    const auto it = m_mDrawableIdxToPointIdx.find(selectedIdx);
    CORRADE_INTERNAL_ASSERT(it != m_mDrawableIdxToPointIdx.end()
                            && it->second < m_ControlPoints.size());
    m_ControlPoints[it->second] = point;
}

/****************************************************************************************************/
void QuadraticCurveApproximation::resetMainControlPointPositions() {
    m_ControlPoints = m_ControlPoints_t0;
    convertControlPoints();
    generateCurves();
}

/****************************************************************************************************/
void QuadraticCurveApproximation::convertControlPoints() {
    if(m_bBezierFromCatmullRom) {
        computeBezierControlPointsFromCatmullRom();
    } else {
        m_CurveControlPoints = m_ControlPoints;
    }
}

/****************************************************************************************************/
void QuadraticCurveApproximation::generateCurves() {
    const auto nCurrentCurves = m_CubicBezierCurves.size();
    const auto nCurves        = m_CurveControlPoints.size() / 4;

    for(size_t i = nCurrentCurves; i < nCurves; ++i) {
        m_CubicBezierCurves.push_back(new CubicBezierCurve(m_Scene, m_Subdivision,
                                                           cubicBezierConfig.color,
                                                           cubicBezierConfig.thickness,
                                                           cubicBezierConfig.bRenderControlPoints,
                                                           false,
                                                           cubicBezierConfig.controlPointRadius));
        m_QuadraticC1Curves.push_back(new QuadraticBezierCurve(m_Scene, m_Subdivision >> 1,
                                                               quadC1BezierConfig.color,
                                                               quadC1BezierConfig.thickness,
                                                               quadC1BezierConfig.bRenderControlPoints,
                                                               false,
                                                               quadC1BezierConfig.controlPointRadius));
    }
    m_CubicBezierCurves.resize(nCurves);
    m_QuadraticC1Curves.resize(nCurves);

    /* Disable by default */
    for(auto& curve: m_QuadraticC1Curves) {
        curve->enabled() = quadC1BezierConfig.bEnabled;
    }

    /* Update drawable points and curves */
    updateMainControlPoints();
    updateCurveControlPoints();
}

/****************************************************************************************************/
void QuadraticCurveApproximation::updateMainControlPoints() {
    if(m_DrawableControlPoints.size() > m_ControlPoints.size()) {
        m_DrawableControlPoints.resize(m_ControlPoints.size());
    }

    for(size_t i = 0; i < m_DrawableControlPoints.size(); ++i) {
        auto& point = m_DrawableControlPoints[i];
        point->setTransformation(Matrix4::translation(m_ControlPoints[i]) *
                                 Matrix4::scaling(Vector3(m_ControlPointRadius)));
        point->setColor(m_ControlPointColor);
    }

    for(size_t i = m_DrawableControlPoints.size(); i < m_ControlPoints.size(); ++i) {
        m_DrawableControlPoints.resize(m_DrawableControlPoints.size() + 1);
        auto& newPoint = m_DrawableControlPoints.back();
        newPoint = new PickableObject(*m_SphereShader,
                                      m_ControlPointColor,
                                      m_MeshSphere,
                                      m_Scene,
                                      m_DrawableGroup);
        newPoint->setTransformation(Matrix4::translation(m_ControlPoints[i]) *
                                    Matrix4::scaling(Vector3(m_ControlPointRadius)));
        newPoint->setColor(m_ControlPointColor);
        m_mDrawableIdxToPointIdx[newPoint->idx()] = i;
    }
}

/****************************************************************************************************/
void QuadraticCurveApproximation::updateCurveControlPoints() {
    if(m_CurveControlPoints.size() < 4) {
        return;
    }

    m_Polylines->setControlPoints(m_CurveControlPoints);

    const auto nCurves = m_CurveControlPoints.size() / 4;

    for(size_t idx = 0; idx < nCurves; ++idx) {
        VPoints currentControlPoints = {
            m_CurveControlPoints[idx * 4],
            m_CurveControlPoints[idx * 4 + 1],
            m_CurveControlPoints[idx * 4 + 2],
            m_CurveControlPoints[idx * 4 + 3]
        };

        m_CubicBezierCurves[idx]->setControlPoints(currentControlPoints);

        std::vector<Vector3> controlPointsC1;
        std::vector<Vector3> controlPointsG1;
        const auto           alpha           = m_gamma;
        const auto           one_m_alpha     = 1.0f - alpha;
        const auto           one_m_alpha_sqr = one_m_alpha * one_m_alpha;
        const auto           alpha_sqr       = alpha * alpha;

        const auto B0 = currentControlPoints[0];
        const auto B1 = currentControlPoints[1];
        const auto B2 = currentControlPoints[2];
        const auto B3 = currentControlPoints[3];

        /* Compute control points of the quadratic C1 curves */
        {
            const auto Q0 = B0;
            const auto Q4 = B3;
            const auto Q1 = B0 + 1.5f * alpha * (B1 - B0);
            const auto Q3 = B3 - 1.5f * (1.0f - alpha) * (B3 - B2);
            const auto Q2 = (1.0f - alpha) * Q1 + alpha * Q3;
            controlPointsC1.push_back(Q0);
            controlPointsC1.push_back(Q1);
            controlPointsC1.push_back(Q2);
            controlPointsC1.push_back(Q3);
            controlPointsC1.push_back(Q4);
        }

        m_QuadraticC1Curves[idx]->setControlPoints(controlPointsC1);
    }

    saveControlPoints();
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
    std::ifstream file("Data/points.txt");
    if(!file.is_open()) {
        Fatal() << "Cannot find point.txt";
    }

    m_ControlPoints.resize(0);
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
        m_ControlPoints.push_back(Vector3{ stof(x), stof(y), stof(z) });
    }
    file.close();
    Debug() << "Loaded" << m_ControlPoints.size() << "points";

    /* Update drawable points and curves */
    convertControlPoints();
    generateCurves();
}

/****************************************************************************************************/
void QuadraticCurveApproximation::saveControlPoints() {
    std::ofstream file("Data/points.txt");
    int           i = 0;
    for(auto& p : m_ControlPoints) {
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
        [](VPoints controlPoints, auto alpha) {
            const auto P0 = controlPoints[0];
            const auto P1 = controlPoints[0 + 1];
            const auto P2 = controlPoints[0 + 2];
            const auto P3 = controlPoints[0 + 3];

            const auto d1 = (P0 - P1).length();
            const auto d2 = (P1 - P2).length();
            const auto d3 = (P2 - P3).length();

            const auto d1_alpha  = std::pow(d1, alpha);
            const auto d2_alpha  = std::pow(d2, alpha);
            const auto d3_alpha  = std::pow(d3, alpha);
            const auto d1_2alpha = std::pow(d1, 2.0f * alpha);
            const auto d2_2alpha = std::pow(d2, 2.0f * alpha);
            const auto d3_2alpha = std::pow(d3, 2.0f * alpha);

            /* Compute control points of the cubic Bezier curve */
            VPoints Bi;
            Bi.resize(4);
            Bi[0] = P1;
            Bi[3] = P2;
            Bi[1] = (d1_2alpha * P2 - d2_2alpha * P0 + (2.0f * d1_2alpha + 3.0f * d1_alpha * d2_alpha + d2_2alpha) * P1) /
                    (3.0f * d1_alpha * (d1_alpha + d2_alpha));
            Bi[2] = (d3_2alpha * P1 - d2_2alpha * P3 + (2.0f * d3_2alpha + 3.0f * d3_alpha * d2_alpha + d2_2alpha) * P2) /
                    (3.0f * d3_alpha * (d3_alpha + d2_alpha));
            return Bi;
        };

    VPoints allPoints;
    for(const auto& p : m_ControlPoints) {
        allPoints.push_back(p);
    }

    VPoints newPoints;
    for(size_t i = 0; i < allPoints.size() - 3; ++i) {
        VPoints    points  = { allPoints[i], allPoints[i + 1], allPoints[i + 2], allPoints[i + 3] };
        const auto bpoints = convert(points, m_CatmullRom_Alpha);
        newPoints.insert(newPoints.end(), bpoints.begin(), bpoints.end());
    }

    m_CurveControlPoints = newPoints;
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
