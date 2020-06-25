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

#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/GL/Mesh.h>

using namespace Corrade;
using namespace Magnum;
using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;

/****************************************************************************************************/
class FlatShadeObject : public SceneGraph::Drawable3D {
public:
    explicit FlatShadeObject(Object3D& object, Shaders::Flat3D& shader,
                             const Color3& color, GL::Mesh& mesh,
                             SceneGraph::DrawableGroup3D* const drawables) :
        SceneGraph::Drawable3D{object, drawables}, m_Shader(shader), m_Color(color), m_Mesh(mesh) {}

    virtual void draw(const Matrix4& transformation, SceneGraph::Camera3D& camera) override {
        if(m_bEnable) {
            m_Shader.setColor(m_Color)
                .setTransformationProjectionMatrix(camera.projectionMatrix() * transformation)
                .draw(m_Mesh);
        }
    }

    FlatShadeObject& setColor(const Color3& color) { m_Color = color; return *this; }
    FlatShadeObject& setEnable(bool bStatus) { m_bEnable = bStatus; return *this; }

    Color3& color() { return m_Color; }
    const Color3& color() const { return m_Color; }
    bool& enabled() { return m_bEnable; }
    const bool& enabled() const { return m_bEnable; }

private:
    bool             m_bEnable { true };
    Shaders::Flat3D& m_Shader;
    Color3           m_Color;
    GL::Mesh&        m_Mesh;
};
