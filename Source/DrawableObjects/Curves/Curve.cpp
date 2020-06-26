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

#include "DrawableObjects/Curves/Curve.h"
#include "DrawableObjects/PickableObject.h"

#include <Corrade/Utility/Assert.h>
#include <Corrade/Containers/ArrayView.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Trade/MeshData.h>

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
    m_MeshSphere = MeshTools::compile(Primitives::icosphereSolid(3));
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
    size_t oldSize = m_DrawablePoints.size();
    m_DrawablePoints.resize(points.size());

    for(size_t i = oldSize; i < points.size(); ++i) {
        auto& newPoint = m_DrawablePoints[i];
        newPoint = new PickableObject(m_SphereShader,
                                      m_Color,
                                      m_MeshSphere,
                                      m_Scene,
                                      &m_Drawables);
        newPoint->setSelectable(m_bEditableControlPoints);
    }

    for(size_t i = 0; i < m_DrawablePoints.size(); ++i) {
        m_DrawablePoints[i]->setTransformation(Matrix4::translation(m_ControlPoints[i]) *
                                               Matrix4::scaling(Vector3(m_ControlPointRadius)));
    }

    /* Recompute lines */
    recomputeCurve();
    return *this;
}

/****************************************************************************************************/
Curve& Curve::draw(SceneGraph::Camera3D& camera, const Vector2i& viewport) {
    if(!m_bEnable || m_Points.empty()) {
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

    if(m_bRenderControlPoints) {
        camera.draw(m_Drawables);
    }

    return *this;
}
