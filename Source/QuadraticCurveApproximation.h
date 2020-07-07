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

#pragma once

#include <Corrade/Containers/Array.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>

#include <unordered_map>

/****************************************************************************************************/
using namespace Corrade;
using namespace Magnum;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

class PickableObject;
class Polyline;
class CubicBezier;
class QuadraticApproximatingCubic;
class CatmullRomCurve;

/****************************************************************************************************/
class QuadraticCurveApproximation {
public:
    /* The main control points */
    using VPoints        = Containers::Array<Vector3>;
    using DrawablePoints = Containers::Array<PickableObject*>;

public:
    explicit QuadraticCurveApproximation(Scene3D* const                     scene,
                                         SceneGraph::DrawableGroup3D* const drawables);
    QuadraticCurveApproximation& draw(Magnum::SceneGraph::Camera3D& camera, const Vector2i& viewport,
                                      bool bCamChanged);

    void updateCurveConfigs();
    struct CurveConfig {
        Color3 color { 0.0f, 0.0f, 1.0f };
        float  thickness{ 1.0f };
        float  controlPointRadius { 0.02f };
        bool   bRenderControlPoints{ true };
        bool   bEnabled { true };
    };
    CurveConfig cubicBezierConfig;
    CurveConfig quadC1BezierConfig;

    int& subdivision() { return m_Subdivision; }
    float& gamma() { return m_gamma; }
    bool& BezierFromCatmullRom() { return m_bBezierFromCatmullRom; }

    void setDataPoint(uint32_t selectedIdx, const Vector3& point);
    void computeBezierControlPoints();
    void generateCurves();
    void updatePolylines();
    void updateCurveControlPoints();
    void computeCurves();
    void loadControlPoints(int curveID = 0);

private:
    void updateDrawablePoints();
    void computeBezierControlPointsFromCatmullRom();

    Scene3D* const                     m_Scene;
    SceneGraph::DrawableGroup3D* const m_Drawables;
    Shaders::Phong                     m_SphereShader{ Shaders::Phong::Flag::ObjectId };
    GL::Mesh m_MeshSphere{ NoCreate };

    DrawablePoints m_DrawablePoints;
    VPoints        m_DataPoints_t0;
    VPoints        m_DataPoints;
    VPoints        m_BezierControlPoints;
    std::unordered_map<uint32_t, size_t> m_mDrawableIdxToPointIdx;

    /* Line subdivision and curve approximation */
    bool  m_bBezierFromCatmullRom { false };
    float m_CatmullRom_Alpha { 0.5f };
    int   m_Subdivision { 64 };
    float m_gamma { 0.5f };

    /* Curves */
    Polyline* m_Polylines;
    Containers::Array<CubicBezier*>                 m_CubicBezierCurves;
    Containers::Array<QuadraticApproximatingCubic*> m_QuadraticC1Curves;
};
