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

#include "Shaders/LineShader.h"

#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/SceneGraph/Drawable.h>

#include <vector>

using namespace Corrade;
using namespace Magnum;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;
class PickableObject;

/****************************************************************************************************/
class Curve {
public:
    using Point          = Vector3;
    using VPoints        = std::vector<Vector3>;
    using DrawablePoints = std::vector<PickableObject*>;

    explicit Curve(Scene3D* const scene,
                   int            subdivision           = 64,
                   const Color3&  color                 = Color3(1.0f),
                   float          thickness             = 1.0f,
                   bool           renderControlPoints   = true,
                   bool           editableControlPoints = true,
                   float          controlPointRadius    = 0.05f);
    virtual ~Curve();

    /* Operations */
    Curve& draw(SceneGraph::Camera3D& camera, const Vector2i& viewport, bool bCamChanged);
    Curve& recomputeCurve();
    Curve& setControlPoints(const VPoints& points);

    /* General curve data */
    int& subdivision() { return m_Subdivision; }
    bool& enabled() { return m_bEnable; }
    Color3& color() { return m_Color; }
    float& thickness() { return m_Thickness; }
    float& miterLimit() { return m_MiterLimit; }

    /* Control point data */
    bool& renderControlPoints() { return m_bRenderControlPoints; }
    float& controlPointRadius() { return m_ControlPointRadius; }

protected:
    virtual void computeLines() = 0;
    void         convertToTriangleStrip(const Matrix4& transformPrjMat, const Vector2& viewport);

    /* Main variables */
    bool m_bEnable { true };
    bool m_bDirty { false };

    /* Main points of line segments */
    int     m_Subdivision { 128 };
    VPoints m_Points;
    VPoints m_TriangleVerts;
    Color3  m_Color { 1.0f };
    float   m_Thickness { 1.0f };
    float   m_MiterLimit { 0.1f };

    /* Render variables for line segments */
    GL::Buffer m_BufferLines;
    GL::Mesh   m_MeshLines;
    LineShader m_LineShader;

    /* Scene variable for rendering control points */
    Scene3D* const              m_Scene;
    SceneGraph::DrawableGroup3D m_Drawables;
    Shaders::Phong              m_SphereShader{ Shaders::Phong::Flag::ObjectId };
    GL::Mesh                    m_MeshSphere{ NoCreate };
    DrawablePoints              m_DrawablePoints;

    VPoints m_ControlPoints;
    bool    m_bRenderControlPoints { true };
    bool    m_bEditableControlPoints { true };
    float   m_ControlPointRadius { 0.05f };
};
