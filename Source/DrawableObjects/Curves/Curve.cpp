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

#include "DrawableObjects/Curves/QuadraticBezierCurve.h"
#include "DrawableObjects/PickableObject.h"

#include <Corrade/Utility/Assert.h>
#include <Corrade/Containers/ArrayView.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Trade/MeshData.h>

using namespace Corrade;
using namespace Magnum;

/****************************************************************************************************/
Curve::Curve(Scene3D* const scene,
             int            subdivision /*= 128*/,
             const Color3&  color /*= Color3(1.0f)*/,
             float          thickness /*= 1.0f*/,
             bool           renderControlPoints /*= true*/,
             bool           editableControlPoints /*= true*/,
             float          controlPointRadius /*= 0.05f*/) :
    m_Subdivision(subdivision),  m_Color(color), m_Thickness(thickness),
    m_MeshLines{GL::MeshPrimitive::LineStripAdjacency},
    m_Scene(scene),
    m_bRenderControlPoints(renderControlPoints),
    m_bEditableControlPoints(editableControlPoints),
    m_ControlPointRadius(controlPointRadius) {
    /* Make sure to have control points */
    CORRADE_INTERNAL_ASSERT(m_Scene);

    /* Setup mesh for line rendering */
    m_MeshLines.addVertexBuffer(m_BufferLines, 0, Shaders::Generic3D::Position{});

    /* Setup point rendering variables */
    m_MeshSphere = MeshTools::compile(Primitives::uvSphereSolid(8, 16));
    m_SphereShader.emplace(Shaders::Phong::Flag::ObjectId);
    m_DrawableGroup.emplace();
}

/****************************************************************************************************/
Curve::~Curve() {} /* Must define in .cpp file to have complete types */

/****************************************************************************************************/
Curve& Curve::recomputeCurve() {
    m_Points.resize(0);
    m_BufferLines.invalidateData();
    computeLines();
    m_bDirty = true;
    return *this;
}

/****************************************************************************************************/
Curve& Curve::setControlPoints(const Curve::VPoints& points) {
    m_ControlPoints = points;
    if(m_DrawableControlPoints.size() > points.size()) {
        m_DrawableControlPoints.resize(points.size());
    }
    for(size_t i = 0; i < m_DrawableControlPoints.size(); ++i) {
        m_DrawableControlPoints[i]->setTransformation(Matrix4::translation(m_ControlPoints[i]) *
                                                      Matrix4::scaling(Vector3(m_ControlPointRadius)));
    }

    for(size_t i = m_DrawableControlPoints.size(); i < points.size(); ++i) {
        m_DrawableControlPoints.resize(m_DrawableControlPoints.size() + 1);
        auto& newPoint = m_DrawableControlPoints.back();
        newPoint.emplace(*m_SphereShader,
                         m_Color,
                         m_MeshSphere,
                         m_Scene,
                         m_DrawableGroup.get());
        newPoint->setTransformation(Matrix4::translation(m_ControlPoints[i]) *
                                    Matrix4::scaling(Vector3(m_ControlPointRadius)));
        newPoint->setSelectable(m_bEditableControlPoints);
    }

    /* Recompute lines */
    recomputeCurve();
    return *this;
}

/****************************************************************************************************/
Curve& Curve::updateDrawableControlPoints() {
    CORRADE_INTERNAL_ASSERT(m_DrawableControlPoints.size() == m_ControlPoints.size());
    for(size_t i = 0; i < m_DrawableControlPoints.size(); ++i) {
        m_DrawableControlPoints[i]->setTransformation(Matrix4::translation(m_ControlPoints[i]) *
                                                      Matrix4::scaling(Vector3(m_ControlPointRadius)));
        m_DrawableControlPoints[i]->setColor(m_Color);
    }
    return *this;
}

/****************************************************************************************************/
Curve& Curve::drawCurve(SceneGraph::Camera3D& camera, const Vector2i& viewport) {
    if(!m_bEnable
       || m_Points.empty()
       ) {
        return *this;
    }

    if(m_bDirty) {
        Containers::ArrayView<const float> data(reinterpret_cast<const float*>(&m_Points[0]), m_Points.size() * 3);
        m_BufferLines.setData(data);
        m_MeshLines.setCount(static_cast<int>(m_Points.size()));
        m_bDirty = false;
    }

    const auto transformPrjMat = camera.projectionMatrix() * camera.cameraMatrix();
    m_LineShader.setTransformationProjectionMatrix(transformPrjMat)
        .setColor(m_Color)
        .setThickness(m_Thickness)
        .setMiterLimit(m_MiterLimit)
        .setViewport(viewport)
        .draw(m_MeshLines);

    return *this;
}

/****************************************************************************************************/
Curve& Curve::drawControlPoints(Magnum::SceneGraph::Camera3D& camera) {
    if(!m_bEnable
       || m_ControlPoints.empty()
       || !m_bRenderControlPoints
       ) {
        return *this;
    }
    camera.draw(*m_DrawableGroup);
    return *this;
}
