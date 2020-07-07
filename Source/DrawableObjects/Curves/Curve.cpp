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
             int            subdivision /*= 64 */,
             const Color3&  color /*= Color3(1.0f)*/,
             float          thickness /*= 1.0f*/,
             bool           renderControlPoints /*= true*/,
             bool           editableControlPoints /*= true*/,
             float          controlPointRadius /*= 0.05f*/) :
    m_Subdivision(subdivision),  m_Color(color), m_Thickness(thickness),
    m_MesTriangles{GL::MeshPrimitive::Triangles},
    m_Scene(scene),
    m_bRenderControlPoints(renderControlPoints),
    m_bEditableControlPoints(editableControlPoints),
    m_ControlPointRadius(controlPointRadius) {
    /* Make sure to have control points */
    CORRADE_INTERNAL_ASSERT(m_Scene);

    /* Setup mesh for line rendering */
    m_MesTriangles.addVertexBuffer(m_BufferTriangles, 0, Shaders::Generic3D::Position{});

    /* Setup point rendering variables */
    m_MeshSphere = MeshTools::compile(Primitives::icosphereSolid(3));
}

/****************************************************************************************************/
Curve::~Curve() {} /* Must define in .cpp file to have complete types */

/****************************************************************************************************/
Curve& Curve::recomputeCurve() {
    m_Points.resize(0);
    m_BufferTriangles.invalidateData();
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
void Curve::convertToTriangleStrip(const Matrix4& transformPrjMat, const Vector2& viewport) {
    auto toScreenSpace = [&](const Vector4& vertex) {
                             return Vector2(vertex.xy() / vertex.w()) * viewport;
                         };
    auto toZValue = [](const Vector4& vertex) { return (vertex.z() / vertex.w()); };

    m_TriangleVerts.resize(0);
    for(size_t i = 1; i < m_Points.size() - 2; ++i) {
        Vector4 points[4] { transformPrjMat* Vector4{ m_Points[i - 1], 1.0f },
                            transformPrjMat* Vector4{ m_Points[i + 0], 1.0f },
                            transformPrjMat* Vector4{ m_Points[i + 1], 1.0f },
                            transformPrjMat* Vector4{ m_Points[i + 2], 1.0f } };
        Vector2 screenPoints[4] = { toScreenSpace(points[0]),
                                    toScreenSpace(points[1]),
                                    toScreenSpace(points[2]),
                                    toScreenSpace(points[3]) };
        float   zVals[4] = { toZValue(points[0]),
                             toZValue(points[1]),
                             toZValue(points[2]),
                             toZValue(points[3]) };

        const Vector2& p0 = screenPoints[0];
        const Vector2& p1 = screenPoints[1];
        const Vector2& p2 = screenPoints[2];
        const Vector2& p3 = screenPoints[3];

        /* Perform naive culling */
        const Vector2 area = viewport * 4.0f;
        if(p1.x() < -area.x() || p1.x() > area.x()) { continue; }
        if(p1.y() < -area.y() || p1.y() > area.y()) { continue; }
        if(p2.x() < -area.x() || p2.x() > area.x()) { continue; }
        if(p2.y() < -area.y() || p2.y() > area.y()) { continue; }

        /* Determine the direction of each of the 3 segments (previous, current, next) */
        Vector2 v0  = (p1 - p0);
        Vector2 v1  = (p2 - p1);
        Vector2 v2  = (p3 - p2);
        float   lv0 = v0.length();
        float   lv1 = v1.length();
        float   lv2 = v2.length();
        if(lv0 > 0) { v0 /= lv0; } else { v0 = v1; }
        if(lv1 > 0) { v1 /= lv1; }
        if(lv2 > 0) { v2 /= lv2; } else { v2 = v1; }

        /* Determine the normal of each of the 3 segments (previous, current, next) */
        const Vector2 n0 = Vector2(-v0.y(), v0.x());
        const Vector2 n1 = Vector2(-v1.y(), v1.x());
        const Vector2 n2 = Vector2(-v2.y(), v2.x());

        /* Determine miter lines by averaging the normals of the 2 segments */
        Vector2 miter_a = (n0 + n1).normalized();  // miter at start of current segment
        Vector2 miter_b = (n1 + n2).normalized();  // miter at end of current segment

        /* Determine the length of the miter by projecting it onto normal and then inverse it */
        float an1 = Math::dot(miter_a, n1);
        float bn1 = Math::dot(miter_b, n2);
        if(an1 == 0) { an1 = 1; }
        if(bn1 == 0) { bn1 = 1; }
        float length_a = m_Thickness / an1;
        float length_b = m_Thickness / bn1;

        /* Prevent excessively long miters at sharp corners */
        if(Math::dot(v0, v1) < -m_MiterLimit) {
            miter_a  = n1;
            length_a = m_Thickness;

            /* Close the gap */
            if(Math::dot(v0, n1) > 0) {
                m_TriangleVerts.push_back(Vector3{ (p1 + m_Thickness * n0) / viewport, zVals[1] });
                m_TriangleVerts.push_back(Vector3{ (p1 + m_Thickness * n1) / viewport, zVals[1] });
                m_TriangleVerts.push_back(Vector3{ p1 / viewport, zVals[1] });
            } else {
                m_TriangleVerts.push_back(Vector3{ (p1 - m_Thickness * n1) / viewport, zVals[1] });
                m_TriangleVerts.push_back(Vector3{ (p1 - m_Thickness * n0) / viewport, zVals[1] });
                m_TriangleVerts.push_back(Vector3{ p1 / viewport, zVals[1] });
            }
        }

        if(Math::dot(v1, v2) < -m_MiterLimit) {
            miter_b  = n1;
            length_b = m_Thickness;
        }

        /* Generate 2 triangles */
        m_TriangleVerts.push_back(Vector3{ (p1 + length_a * miter_a) / viewport, zVals[1] });
        m_TriangleVerts.push_back(Vector3{ (p1 - length_a * miter_a) / viewport, zVals[1] });
        m_TriangleVerts.push_back(Vector3{ (p2 + length_b * miter_b) / viewport, zVals[2] });

        m_TriangleVerts.push_back(Vector3{ (p2 + length_b * miter_b) / viewport, zVals[2] });
        m_TriangleVerts.push_back(Vector3{ (p1 - length_a * miter_a) / viewport, zVals[1] });
        m_TriangleVerts.push_back(Vector3{ (p2 - length_b * miter_b) / viewport, zVals[2] });
    }
}

/****************************************************************************************************/
Curve& Curve::draw(SceneGraph::Camera3D& camera, const Vector2i& viewport, bool bCamChanged) {
    if(!m_bEnable || m_Points.empty()) {
        return *this;
    }

    if(m_bDirty || bCamChanged) {
        const Matrix4 transformPrjMat = camera.projectionMatrix() * camera.cameraMatrix();
        convertToTriangleStrip(transformPrjMat, Vector2{ viewport });
        Containers::ArrayView<const float> data(reinterpret_cast<const float*>(&m_TriangleVerts[0]), m_TriangleVerts.size() * 3);
        m_BufferTriangles.setData(data);
        m_MesTriangles.setCount(static_cast<int>(m_TriangleVerts.size()));
        m_bDirty = false;
    }
    m_LineShader.setColor(m_Color).draw(m_MesTriangles);
    if(m_bRenderControlPoints) { camera.draw(m_Drawables); }
    return *this;
}
