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

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/AbstractTranslationRotation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include "ArcBall.h"

/****************************************************************************************************/
/* Arcball camera implementation integrated into the SceneGraph */
class ArcBallCamera : public ArcBall {
public:
    template<class Transformation> ArcBallCamera(
        SceneGraph::Scene<Transformation>& scene,
        const Vector3& cameraPosition, const Vector3& viewCenter,
        const Vector3& upDir, Deg fov, const Vector2i& windowSize,
        const Vector2i& viewportSize) :
        ArcBall{cameraPosition, viewCenter, upDir, fov, windowSize} {
        /* Create a camera object of a concrete type */
        auto* cameraObject = new SceneGraph::Object<Transformation>{ &scene };
        (*(_camera = new SceneGraph::Camera3D{ *cameraObject }))
            .setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
            .setProjectionMatrix(Matrix4::perspectiveProjection(
                                     fov, Vector2{ windowSize }.aspectRatio(), 0.01f, 100.0f))
            .setViewport(viewportSize);

        /* Save the abstract transformation interface and initialize the
           camera position through that */
        (*(_cameraObject = cameraObject))
            .rotate(transformation().rotation())
            .translate(transformation().translation());
    }

    /* Update screen and viewport size after the window has been resized */
    void reshape(const Vector2i& windowSize, const Vector2i& viewportSize) {
        _windowSize = windowSize;
        _camera->setViewport(viewportSize);
    }

    /* Update the SceneGraph camera if arcball has been changed */
    bool update() {
        /* call the internal update */
        if(!updateTransformation()) { return false; }

        (*_cameraObject)
            .resetTransformation()
            .rotate(transformation().rotation())
            .translate(transformation().translation());
        return true;
    }

    /* Draw objects using the internal scenegraph camera */
    void draw(SceneGraph::DrawableGroup3D& drawables) {
        _camera->draw(drawables);
    }

    /* Accessor to the raw camera object */
    SceneGraph::Camera3D& camera() const { return *_camera; }

private:
    SceneGraph::AbstractTranslationRotation3D* _cameraObject{};
    SceneGraph::Camera3D* _camera{};
};
