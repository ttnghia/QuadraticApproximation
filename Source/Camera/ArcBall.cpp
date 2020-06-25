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

#include <Magnum/Math/Matrix3.h>
#include "ArcBall.h"

/****************************************************************************************************/
namespace {
/* Project a point in NDC onto the arcball sphere */
Quaternion ndcToArcBall(const Vector2& p) {
    const Float dist = Math::dot(p, p);

    /* Point is on sphere */
    if(dist <= 1.0f) {
        return { { p.x(), p.y(), Math::sqrt(1.0f - dist) }, 0.0f };
    }
    /* Point is outside sphere */
    else {
        const Vector2 proj = p.normalized();
        return { { proj.x(), proj.y(), 0.0f }, 0.0f };
    }
}
}

/****************************************************************************************************/
ArcBall::ArcBall(const Vector3& eye, const Vector3& viewCenter,
                 const Vector3& upDir, Deg fov, const Vector2i& windowSize) :
    _fov{fov}, _windowSize{windowSize} {
    setViewParameters(eye, viewCenter, upDir);
}

/****************************************************************************************************/
void ArcBall::setViewParameters(const Vector3& eye, const Vector3& viewCenter,
                                const Vector3& upDir) {
    const Vector3 dir   = viewCenter - eye;
    Vector3       zAxis = dir.normalized();
    Vector3       xAxis = (Math::cross(zAxis, upDir.normalized())).normalized();
    Vector3       yAxis = (Math::cross(xAxis, zAxis)).normalized();
    xAxis = (Math::cross(zAxis, yAxis)).normalized();

    _targetPosition  = -viewCenter;
    _targetZooming   = -dir.length();
    _targetQRotation = Quaternion::fromMatrix(
        Matrix3x3{ xAxis, yAxis, -zAxis }.transposed()).normalized();

    _positionT0  = _currentPosition = _targetPosition;
    _zoomingT0   = _currentZooming = _targetZooming;
    _qRotationT0 = _currentQRotation = _targetQRotation;

    updateInternalTransformations();
}

/****************************************************************************************************/
void ArcBall::reset() {
    _targetPosition  = _positionT0;
    _targetZooming   = _zoomingT0;
    _targetQRotation = _qRotationT0;
}

/****************************************************************************************************/
void ArcBall::setLagging(const Float lagging) {
    CORRADE_INTERNAL_ASSERT(lagging >= 0.0f && lagging < 1.0f);
    _lagging = lagging;
}

/****************************************************************************************************/
void ArcBall::initTransformation(const Vector2i& mousePos) {
    _prevMousePosNDC = screenCoordToNDC(mousePos);
}

/****************************************************************************************************/
void ArcBall::rotate(const Vector2i& mousePos) {
    const Vector2    mousePosNDC      = screenCoordToNDC(mousePos);
    const Quaternion currentQRotation = ndcToArcBall(mousePosNDC);
    const Quaternion prevQRotation    = ndcToArcBall(_prevMousePosNDC);
    _prevMousePosNDC = mousePosNDC;
    _targetQRotation =
        (currentQRotation * prevQRotation * _targetQRotation).normalized();
}

/****************************************************************************************************/
void ArcBall::translate(const Vector2i& mousePos) {
    const Vector2 mousePosNDC    = screenCoordToNDC(mousePos);
    const Vector2 translationNDC = mousePosNDC - _prevMousePosNDC;
    _prevMousePosNDC = mousePosNDC;
    translateDelta(translationNDC);
}

/****************************************************************************************************/
void ArcBall::translateDelta(const Vector2& translationNDC) {
    /* Half size of the screen viewport at the view center and perpendicular
       with the viewDir */
    const Float hh = Math::abs(_targetZooming) * Math::tan(_fov * 0.5f);
    const Float hw = hh * Vector2{ _windowSize }.aspectRatio();

    _targetPosition += _inverseView.transformVector(
        { translationNDC.x()* hw, translationNDC.y()* hh, 0.0f });
}

/****************************************************************************************************/
void ArcBall::zoom(const Float delta) {
    _targetZooming += delta;
}

/****************************************************************************************************/
bool ArcBall::updateTransformation() {
    const Vector3    diffViewCenter = _targetPosition - _currentPosition;
    const Quaternion diffRotation   = _targetQRotation - _currentQRotation;
    const Float      diffZooming    = _targetZooming - _currentZooming;

    const Float dViewCenter = Math::dot(diffViewCenter, diffViewCenter);
    const Float dRotation   = Math::dot(diffRotation, diffRotation);
    const Float dZooming    = diffZooming * diffZooming;

    /* Nothing change */
    if(dViewCenter < 1.0e-10f &&
       dRotation < 1.0e-10f &&
       dZooming < 1.0e-10f) {
        return false;
    }

    /* Nearly done: just jump directly to the target */
    if(dViewCenter < 1.0e-6f &&
       dRotation < 1.0e-6f &&
       dZooming < 1.0e-6f) {
        _currentPosition  = _targetPosition;
        _currentQRotation = _targetQRotation;
        _currentZooming   = _targetZooming;

        /* Interpolate between the current transformation and the target
           transformation */
    } else {
        const Float t = 1 - _lagging;
        _currentPosition  = Math::lerp(_currentPosition, _targetPosition, t);
        _currentZooming   = Math::lerp(_currentZooming, _targetZooming, t);
        _currentQRotation = Math::slerpShortestPath(
            _currentQRotation, _targetQRotation, t);
    }

    updateInternalTransformations();
    return true;
}

/****************************************************************************************************/
void ArcBall::updateInternalTransformations() {
    _view = DualQuaternion::translation(Vector3::zAxis(_currentZooming)) *
            DualQuaternion{ _currentQRotation } *
    DualQuaternion::translation(_currentPosition);
    _inverseView = _view.inverted();
}

/****************************************************************************************************/
Vector2 ArcBall::screenCoordToNDC(const Vector2i& mousePos) const {
    return { mousePos.x()*2.0f / _windowSize.x() - 1.0f,
             1.0f - 2.0f* mousePos.y() / _windowSize.y() };
}
