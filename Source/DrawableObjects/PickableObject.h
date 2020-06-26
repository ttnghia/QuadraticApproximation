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
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Shaders/Phong.h>

using namespace Corrade;
using namespace Magnum;
using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D  = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

/****************************************************************************************************/
class PickableObject : public Object3D, SceneGraph::Drawable3D {
public:
    explicit PickableObject(Shaders::Phong& shader, const Color3& color,
                            GL::Mesh& mesh,
                            Scene3D* parent,
                            SceneGraph::DrawableGroup3D* drawables);
    virtual ~PickableObject() override;

    virtual void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

    PickableObject& setSelected(bool bState) { m_bSelected = bState; return *this; }
    PickableObject& setMovable(bool bState) { m_bMovable = bState; return *this; }
    PickableObject& setColor(const Color3& color) { m_Color = color; return *this; }
    PickableObject& setSelectable(bool bStatus) { m_bSelectable = bStatus; return *this; }

    uint32_t idx() const { return m_idx; }
    bool isSelectable() const { return m_bSelectable; }
    bool isSelected() const { return m_bSelected; }
    bool isMovable() const { return m_bMovable; }

    static void updateSelectedObject(uint32_t selectedIdx);
    static PickableObject* selectedObj() { return s_SelectedObj; }

private:
    static uint32_t getUniqueID();
    static inline std::vector<PickableObject*> s_GeneratedObjs;
    static inline PickableObject*              s_SelectedObj { nullptr };

    bool     m_bSelectable { true };
    bool     m_bSelected { false };
    bool     m_bMovable { true };
    uint32_t m_idx;

    Shaders::Phong& m_Shader;
    Color3          m_Color;
    GL::Mesh&       m_Mesh;
};
