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

#include "DrawableObjects/Curves/CubicBezierCurve.h"

/****************************************************************************************************/
CubicBezierCurve::CubicBezierCurve(Scene3D* const scene,
                                   int            subdivision /*= 128*/,
                                   const Color3&  color /*= Color3(1.0f)*/,
                                   float          thickness /*= 1.0f*/,
                                   bool           renderControlPoints /*= true*/,
                                   bool           editableControlPoints /*= true*/,
                                   float          controlPointRadius /*= 0.05f*/) :
    Curve(scene, subdivision, color, thickness, renderControlPoints, editableControlPoints, controlPointRadius) {}

/****************************************************************************************************/
void CubicBezierCurve::computeLines() {
    if(m_ControlPoints.size() != 4) {
        Fatal() << "Cubic Bezier requires 4 control points, currently has " << m_ControlPoints.size();
    }
    const auto B0 = m_ControlPoints[0];
    const auto B1 = m_ControlPoints[0 + 1];
    const auto B2 = m_ControlPoints[0 + 2];
    const auto B3 = m_ControlPoints[0 + 3];

    const auto step = 1.0f / static_cast<float>(m_Subdivision);
    for(int i = 0; i <= m_Subdivision; ++i) {
        const auto t           = static_cast<float>(i) * step;
        const auto t_sqr       = t * t;
        const auto one_m_t     = 1.0f - t;
        const auto one_m_t_sqr = one_m_t * one_m_t;

        const auto position = one_m_t * one_m_t_sqr * B0 +
                              3.0f * one_m_t_sqr * t * B1 +
                              3.0f * one_m_t * t_sqr * B2 +
                              t * t_sqr * B3;
        m_Points.push_back(position);

        if(i == 0 || i == m_Subdivision) {
            m_Points.push_back(position);
        }
    }
}
