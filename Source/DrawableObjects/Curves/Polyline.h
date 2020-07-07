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
#include "DrawableObjects/Curves/Curve.h"

/****************************************************************************************************/
class Polyline : public Curve {
public:
    explicit Polyline(Scene3D* const scene,
                      const Color3&  color                 = Color3(1.0f),
                      float          thickness             = 1.0f,
                      bool           renderControlPoints   = false,
                      bool           editableControlPoints = false,
                      float          controlPointRadius    = 0.05f) :
        Curve(scene, 1, color, thickness, renderControlPoints, editableControlPoints, controlPointRadius) {}

protected:
    virtual void computeLines() override {
        if(m_ControlPoints.size() == 0) {
            return;
        }
        arrayResize(m_Points, 0);
        arrayAppend(m_Points, m_ControlPoints[0]);
        for(size_t i = 0; i < m_ControlPoints.size(); ++i) {
            arrayAppend(m_Points, m_ControlPoints[i]);
        }
        arrayAppend(m_Points, m_Points.back());
    }
};
