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

#include "Application/GLApplication.h"
#include <Magnum/ImGuiIntegration/Context.hpp>

/****************************************************************************************************/
class ImGuiApplication : public GLApplication {
public:
    explicit ImGuiApplication(const std::string& title, const Arguments& arguments,
                              const Vector2i& defaultWindowSize = Vector2i{ 1920, 1080 });

protected:
    void viewportEvent(ViewportEvent& event) override;
    void keyPressEvent(KeyEvent& event) override;
    void keyReleaseEvent(KeyEvent& event) override;
    void mousePressEvent(MouseEvent& event) override;
    void mouseReleaseEvent(MouseEvent& event) override;
    void mouseMoveEvent(MouseMoveEvent& event) override;
    void mouseScrollEvent(MouseScrollEvent& event) override;
    void textInputEvent(TextInputEvent& event) override;

    void beginFrame();
    void endFrame();

    void showMenuHeader();
    void showMenuFooter(bool bButtonResetCamera = true);

    /* Window control */
    bool m_bShowMenu { true };
    ImGuiIntegration::Context m_ImGuiContext{ NoCreate };
};
