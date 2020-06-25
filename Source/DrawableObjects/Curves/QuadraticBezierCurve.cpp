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

#include "DrawableObjects/Curves/QuadraticBezierCurve.h"

/****************************************************************************************************/
QuadraticBezierCurve::QuadraticBezierCurve(Scene3D* const scene,
                                           int            subdivision /*= 128*/,
                                           const Color3&  color /*= Color3(1.0f)*/,
                                           float          thickness /*= 1.0f*/,
                                           bool           renderControlPoints /*= true*/,
                                           bool           editableControlPoints /*= true*/,
                                           float          controlPointRadius /*= 0.05f*/) :
    Curve(scene, subdivision, color, thickness, renderControlPoints, editableControlPoints, controlPointRadius) {}

/****************************************************************************************************/
void QuadraticBezierCurve::computeLines() {
    const auto Q0 = m_ControlPoints[0];
    const auto Q1 = m_ControlPoints[0 + 1];
    const auto Q2 = m_ControlPoints[0 + 2];
    const auto Q3 = m_ControlPoints[0 + 3];
    const auto Q4 = m_ControlPoints[0 + 4];

    for(int i = 0; i <= m_Subdivision; ++i) {
        const auto t           = static_cast<float>(i) / static_cast<float>(m_Subdivision);
        const auto s           = (t < 0.5f) ? 2.0f * t : 2.0f * (t - 0.5f);
        const auto s_sqr       = s * s;
        const auto one_m_s     = 1.0f - s;
        const auto one_m_s_sqr = one_m_s * one_m_s;

        const auto position = (t < 0.5f) ? one_m_s_sqr * Q0 + 2.0f * one_m_s * s * Q1 + s_sqr * Q2 :
                              one_m_s_sqr * Q2 + 2.0f * one_m_s * s * Q3 + s_sqr * Q4;
        m_Points.push_back(position);

        if(i == 0 || i == m_Subdivision) {
            m_Points.push_back(position);
        }
    }
}
