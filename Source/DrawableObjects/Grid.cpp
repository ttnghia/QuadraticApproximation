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

#include "DrawableObjects/Grid.h"

#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Primitives/Grid.h>

/****************************************************************************************************/
Grid::Grid(Scene3D* const                     scene,
           SceneGraph::DrawableGroup3D* const drawableGroup) {
    using namespace Magnum::Math::Literals;
    m_Mesh = MeshTools::compile(Primitives::grid3DWireframe({ 20, 20 }));

    m_Obj3D.emplace(scene);
    m_Obj3D->scale(Vector3(10.0f));
    m_Obj3D->rotateX(90.0_degf);

    m_FlatShader = Shaders::Flat3D{};
    m_DrawableObj.emplace(*m_Obj3D.get(), m_FlatShader, Color3(0.75f), m_Mesh, drawableGroup);
}
