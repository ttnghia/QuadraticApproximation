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

#include "Application/ImGuiApplication.h"
#include <Magnum/GL/Renderer.h>

/****************************************************************************************************/
ImGuiApplication::ImGuiApplication(const std::string& title, const Arguments& arguments,
                                   const Vector2i& defaultWindowSize) :
    GLApplication{title, arguments, defaultWindowSize} {
    /* Setup ImGui and ImGuizmo */
    m_ImGuiContext = ImGuiIntegration::Context(Vector2{ windowSize() } / dpiScaling(),
                                               windowSize(), framebufferSize());
    ImGui::StyleColorsDark();

    /* Setup proper blending to be used by ImGui. There's a great chance
       you'll need this exact behavior for the rest of your scene. If not, set
       this only for the drawFrame() call. */
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
                                   GL::Renderer::BlendFunction::OneMinusSourceAlpha);
}

/****************************************************************************************************/
void ImGuiApplication::viewportEvent(ViewportEvent& event) {
    GLApplication::viewportEvent(event);

    /* Relayout ImGui */
    m_ImGuiContext.relayout(Vector2{ event.windowSize() } / event.dpiScaling(),
                            event.windowSize(), event.framebufferSize());
}

/****************************************************************************************************/
void ImGuiApplication::keyPressEvent(KeyEvent& event) {
    if(m_ImGuiContext.handleKeyPressEvent(event)) {
        event.setAccepted(true);
    } else {
        GLApplication::keyPressEvent(event);
        if(!event.isAccepted()) {
            if(event.key() == KeyEvent::Key::H) {
                m_bShowMenu ^= true;
                event.setAccepted(true);
            }
        }
    }
}

void ImGuiApplication::keyReleaseEvent(KeyEvent& event) {
    if(m_ImGuiContext.handleKeyReleaseEvent(event)) {
        event.setAccepted(true);
    }
}

/****************************************************************************************************/
void ImGuiApplication::mousePressEvent(MouseEvent& event) {
    if(m_ImGuiContext.handleMousePressEvent(event)) {
        event.setAccepted(true);
    } else {
        GLApplication::mousePressEvent(event);
    }
}

/****************************************************************************************************/
void ImGuiApplication::mouseReleaseEvent(MouseEvent& event) {
    if(m_ImGuiContext.handleMouseReleaseEvent(event)) {
        event.setAccepted(true);
    }
}

/****************************************************************************************************/
void ImGuiApplication::mouseMoveEvent(MouseMoveEvent& event) {
    if(m_ImGuiContext.handleMouseMoveEvent(event)) {
        event.setAccepted(true);
    } else {
        GLApplication::mouseMoveEvent(event);
    }
}

/****************************************************************************************************/
void ImGuiApplication::mouseScrollEvent(MouseScrollEvent& event) {
    if(m_ImGuiContext.handleMouseScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted(true);
    } else {
        GLApplication::mouseScrollEvent(event);
    }
}

/****************************************************************************************************/
void ImGuiApplication::textInputEvent(TextInputEvent& event) {
    if(m_ImGuiContext.handleTextInputEvent(event)) {
        event.setAccepted(true);
    }
}

/****************************************************************************************************/
void ImGuiApplication::beginFrame() {
    m_ImGuiContext.newFrame();
    /* Enable text input, if needed */
    if(ImGui::GetIO().WantTextInput && !isTextInputActive()) {
        startTextInput();
    } else if(!ImGui::GetIO().WantTextInput && isTextInputActive()) {
        stopTextInput();
    }
}

/****************************************************************************************************/
void ImGuiApplication::endFrame() {
    /* Update application cursor */
    m_ImGuiContext.updateApplicationCursor(*this);

    /* Set appropriate states. If you only draw imgui UI, it is sufficient to do this once in the constructor. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);

    m_ImGuiContext.drawFrame();

    /* Reset state. Only needed if you want to draw something else with different state next frame. */
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);
}

/****************************************************************************************************/
void ImGuiApplication::showMenuHeader() {
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Hide/show menu: H | Exit: ESC");
    ImGui::Text("%3.2f FPS", static_cast<double>(ImGui::GetIO().Framerate));
    ImGui::SameLine(100);
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    if(ImGui::Checkbox("VSync", &m_bVsync)) {
        setSwapInterval(m_bVsync);
    }
    #endif
    ImGui::Spacing();
    ImGui::Checkbox("Render grid", &m_Grid->enabled());
    ImGui::ColorEdit3("Background color", m_BkgColor.data());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void ImGuiApplication::showMenuFooter(bool bButtonResetCamera) {
    if(bButtonResetCamera) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        if(ImGui::Button("Reset camera")) {
            m_Camera->reset();
        }
    }
    ImGui::End();
}
