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

#include "DrawableObjects/FlatShadeObject.h"

#include <Corrade/Containers/Pointer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>

/****************************************************************************************************/
using namespace Corrade;
using namespace Magnum;
using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D  = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

/****************************************************************************************************/
class Grid {
public:
    explicit Grid(Scene3D* const scene, SceneGraph::DrawableGroup3D* const drawableGroup);

    Grid& setColor(const Color3& color) { m_DrawableObj->color() = color; return *this; }
    Grid& setEnable(bool bStatus) { m_DrawableObj->setEnable(bStatus); return *this; }
    Grid& transform(const Matrix4& transMatrix) { m_Obj3D->transform(transMatrix); return *this; }

    Color3& color() { return m_DrawableObj->color(); }
    bool& enabled() { return m_DrawableObj->enabled(); }

private:
    GL::Mesh                             m_Mesh { NoCreate };
    Shaders::Flat3D                      m_FlatShader { NoCreate };
    Containers::Pointer<Object3D>        m_Obj3D;
    Containers::Pointer<FlatShadeObject> m_DrawableObj;
};
