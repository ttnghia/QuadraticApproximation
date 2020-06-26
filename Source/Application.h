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

#include "Application/PickableApplication.h"
#include "QuadraticCurveApproximation.h"

/****************************************************************************************************/
class Application : public PickableApplication {
public:
    explicit Application(const Arguments& arguments);

protected:
    void drawEvent() override;
    void showMenu();

    /* Quadratic approximation object */
    Containers::Pointer<QuadraticCurveApproximation> m_Curves { nullptr };
};

/****************************************************************************************************/
MAGNUM_APPLICATION_MAIN(Application)
