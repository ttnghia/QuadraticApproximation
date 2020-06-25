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

#include <Corrade/Containers/Pointer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>

#include <unordered_map>

/****************************************************************************************************/
using namespace Corrade;
using namespace Magnum;
using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D  = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

class PickableObject;
class Polyline;
class CubicBezierCurve;
class QuadraticBezierCurve;
class CatmullRomCurve;

/****************************************************************************************************/
class QuadraticCurveApproximation {
public:
    /* The main control points */
    using VPoints        = std::vector<Vector3>;
    using DrawablePoints = std::vector<PickableObject*>;

public:
    explicit QuadraticCurveApproximation(Scene3D* const scene, SceneGraph::DrawableGroup3D* const drawableGroup);
    QuadraticCurveApproximation& draw(Magnum::SceneGraph::Camera3D& camera, const Vector2i& viewport);

    void setControlPoint(uint32_t selectedIdx, const Vector3& point);
    void resetMainControlPointPositions();
    void convertControlPoints();
    void generateCurves();
    void updateMainControlPoints();
    void updateCurveControlPoints();
    void computeCurves();
    void loadControlPoints();
    void saveControlPoints();
    void computeBezierControlPointsFromCatmullRom();

    int& subdivision() { return m_Subdivision; }
    float& gamma() { return m_gamma; }
    bool& BezierFromCatmullRom() { return m_bBezierFromCatmullRom; }

    float& controlPointRadius() { return m_ControlPointRadius; }
    Color3& controlPointColor() { return m_ControlPointColor; }
    VPoints& controlPoints() { return m_ControlPoints; }

    Polyline& polylines() { return *m_Polylines; }
    auto& getCubicBezierCurves() { return m_CubicBezierCurves; }
    auto& getQuadC1Curves() { return m_QuadraticC1Curves; }

    void updateCurveConfigs();
    struct CurveConfig {
        Color3 color { 0.0f, 0.0f, 1.0f };
        float  thickness{ 1.0f };
        float  controlPointRadius { 1.0f };
        bool   bRenderControlPoints{ true };
        bool   bEnabled { true };
    };
    CurveConfig cubicBezierConfig;
    CurveConfig quadC1BezierConfig;

private:
    Scene3D* const                      m_Scene;
    SceneGraph::DrawableGroup3D* const  m_DrawableGroup;
    Containers::Pointer<Shaders::Phong> m_SphereShader;
    GL::Mesh m_MeshSphere{ NoCreate };

    DrawablePoints m_DrawableControlPoints;
    VPoints        m_ControlPoints_t0;
    VPoints        m_ControlPoints;
    VPoints        m_CurveControlPoints;
    float          m_ControlPointRadius { 0.02f };
    Color3         m_ControlPointColor { 0.0f, 0.0f, 1.0f };
    std::unordered_map<uint32_t, size_t> m_mDrawableIdxToPointIdx;

    /* Line subdivision and curve approximation */
    bool  m_bBezierFromCatmullRom { true };
    float m_CatmullRom_Alpha { 0.5f };

    int   m_Subdivision { 128 };
    float m_gamma { 0.5f };

    /* Curves */
    Polyline* m_Polylines;
    std::vector<CubicBezierCurve*>     m_CubicBezierCurves;
    std::vector<QuadraticBezierCurve*> m_QuadraticC1Curves;
};
