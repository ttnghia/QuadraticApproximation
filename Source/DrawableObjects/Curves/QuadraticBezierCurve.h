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
class QuadraticBezierCurve : public Curve {
public:
    explicit QuadraticBezierCurve(Scene3D* const scene,
                                  int            subdivision           = 128,
                                  const Color3&  color                 = Color3(1.0f),
                                  float          thickness             = 1.0f,
                                  bool           renderControlPoints   = true,
                                  bool           editableControlPoints = true,
                                  float          controlPointRadius    = 0.05f);
protected:
    virtual void computeLines() override;
};
