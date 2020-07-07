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

#include <Magnum/GL/DefaultFramebuffer.h>

#include "DrawableObjects/PickableObject.h"
#include "Application.h"

/****************************************************************************************************/
Application::Application(const Arguments& arguments) :
    PickableApplication{"Quadratic Approximation of Cubic Curves", arguments} {
    m_DefaultCamPosition = Vector3(0, 1.0, 5);
    m_DefaultCamTarget   = Vector3(0, 0.75, 0);
    setupCamera();

    /* Setup curves */
    m_Curves.emplace(&m_Scene, &m_Drawables);
}

/****************************************************************************************************/
void Application::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);
    ImGuiApplication::beginFrame();

    /* Update camera */
    bool bCamChanged = m_Camera->update();

    /* Draw to custom framebuffer */
    m_FrameBuffer
        .clearColor(0, m_BkgColor)
        .clearColor(1, Vector4ui{})
        .clearDepth(1.0f)
        .bind();

    /* Draw curves (with control points) */
    m_Curves->draw(m_Camera->camera(), m_FrameBuffer.viewport().size(), bCamChanged);

    /* Draw other objects (grid) */
    m_Camera->draw(m_Drawables);

    /* Bind the default framebuffer back, as only the clickable objects need to be render to custom framebuffer */
    GL::defaultFramebuffer.bind();

    /* Blit color to window framebuffer */
    m_FrameBuffer.mapForRead(GL::Framebuffer::ColorAttachment{ 0 });
    GL::AbstractFramebuffer::blit(m_FrameBuffer, GL::defaultFramebuffer,
                                  { {}, m_FrameBuffer.viewport().size() }, GL::FramebufferBlit::Color);

    if(m_bShowMenu) {
        showMenuHeader();
        showMenu();
        showMenuFooter();
    }

    /* Manipulate nodes' transformation */
    PickableObject* selectedPoint = PickableObject::selectedObj();
    if(selectedPoint) {
        if(selectedPoint->isSelectable()
           && selectedPoint->isMovable()) {
            ImGui::Begin("Editor");
            std::string str = "Point: #" + std::to_string(selectedPoint->idx());
            ImGui::Text("%s", str.c_str());
            ImGui::Separator();
            ImGui::Spacing();

            Matrix4 objMat = selectedPoint->transformation();
            if(editPointTransformation(objMat)) {
                selectedPoint->setTransformation(objMat); /* Update drawable transformation */

                /* Update the corresponding node's point */
                Vector3 translation = objMat[3].xyz();
                m_Curves->setDataPoint(selectedPoint->idx(), translation);

                /* Update all control points and curves */
                m_Curves->saveControlPoints();
                m_Curves->computeBezierControlPoints();
                m_Curves->updatePolylines();
                m_Curves->updateCurveControlPoints();
                m_Curves->computeCurves();
            }
            ImGui::End();
        }
    }

    ImGuiApplication::endFrame();
    swapBuffers();
    redraw();
}

/****************************************************************************************************/
void Application::showMenu() {
    if(ImGui::CollapsingHeader("Tessellation and quadratic approximation", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID("Subdivision+Approximation");
        if(ImGui::SliderInt("Segments", &m_Curves->subdivision(), 1, 128)) {
            m_Curves->computeCurves();
        }
        if(ImGui::Checkbox("Bezier from Catmull-Rom", &m_Curves->BezierFromCatmullRom())) {
            m_Curves->computeBezierControlPoints();
            m_Curves->generateCurves();
            m_Curves->computeCurves();
        }
        if(ImGui::Checkbox("Render quadratic Bezier", &m_Curves->quadC1BezierConfig.bEnabled)) {
            m_Curves->updateCurveConfigs();
        }

        if(m_Curves->quadC1BezierConfig.bEnabled &&
           ImGui::SliderFloat("\\gamma", &m_Curves->gamma(), 0.0f, 1.0f)) {
            m_Curves->updateCurveControlPoints();
            m_Curves->computeCurves();
        }
        ImGui::SameLine();
        if(ImGui::Button("Set 0.5")) {
            m_Curves->gamma() = 0.5f;
            m_Curves->updateCurveControlPoints();
            m_Curves->computeCurves();
        }

        ImGui::PopID();
    }
}
