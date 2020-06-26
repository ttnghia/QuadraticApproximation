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

#include "Application/PickableApplication.h"
#include "DrawableObjects/PickableObject.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/Image.h>

#include <ImGuizmo.h>

/****************************************************************************************************/
PickableApplication::PickableApplication(const std::string& title, const Arguments& arguments,
                                         size_t indexDataSize, const Vector2i& defaultWindowSize) :
    ImGuiApplication{title, arguments, defaultWindowSize} {
    /* Relayout ImGuizmo */
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    /* Setup buffers */
    m_RBColor = GL::Renderbuffer{};
    m_RBColor.setStorage(GL::RenderbufferFormat::RGBA8, GL::defaultFramebuffer.viewport().size());

    CORRADE_INTERNAL_ASSERT(indexDataSize == 8
                            || indexDataSize == 16
                            || indexDataSize == 32);
    if(indexDataSize == 8) {
        m_IndexFormat = GL::RenderbufferFormat::R8UI;
        m_PixelFormat = PixelFormat::R8UI;
    } else if(indexDataSize == 16) {
        m_IndexFormat = GL::RenderbufferFormat::R16UI;
        m_PixelFormat = PixelFormat::R16UI;
    } else if(indexDataSize == 32) {
        m_IndexFormat = GL::RenderbufferFormat::R32UI;
        m_PixelFormat = PixelFormat::R32UI;
    } else {
        Fatal{} << "Invalid index data size type";
    }

    m_RBObjectIdx = GL::Renderbuffer{};
    m_RBObjectIdx.setStorage(m_IndexFormat, GL::defaultFramebuffer.viewport().size());

    m_RBDepth = GL::Renderbuffer{};
    m_RBDepth.setStorage(GL::RenderbufferFormat::DepthComponent24, GL::defaultFramebuffer.viewport().size());

    m_FrameBuffer = GL::Framebuffer{ GL::defaultFramebuffer.viewport() };
    m_FrameBuffer.attachRenderbuffer(GL::Framebuffer::ColorAttachment{ 0 }, m_RBColor)
        .attachRenderbuffer(GL::Framebuffer::ColorAttachment{ 1 }, m_RBObjectIdx)
        .attachRenderbuffer(GL::Framebuffer::BufferAttachment::Depth,  m_RBDepth)
        .mapForDraw({ { Shaders::Phong::ColorOutput, GL::Framebuffer::ColorAttachment{ 0 } },
                        { Shaders::Phong::ObjectIdOutput, GL::Framebuffer::ColorAttachment{ 1 } } });
    CORRADE_INTERNAL_ASSERT(m_FrameBuffer.checkStatus(GL::FramebufferTarget::Draw) == GL::Framebuffer::Status::Complete);
}

/****************************************************************************************************/
void PickableApplication::viewportEvent(ViewportEvent& event) {
    ImGuiApplication::viewportEvent(event);

    /* Relayout ImGuizmo */
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    /* Resize buffers */
    const auto viewport = GL::defaultFramebuffer.viewport();
    m_RBColor.setStorage(GL::RenderbufferFormat::RGBA8, viewport.size());
    m_RBObjectIdx.setStorage(GL::RenderbufferFormat::R16UI, viewport.size());
    m_RBDepth.setStorage(GL::RenderbufferFormat::DepthComponent24, viewport.size());
    m_FrameBuffer.setViewport(viewport);
}

/****************************************************************************************************/
void PickableApplication::mousePressEvent(MouseEvent& event) {
    ImGuiApplication::mousePressEvent(event);
    if(event.isAccepted()) {
        return;
    }

    /* Update picking object
     * Read object ID at given click position (framebuffer has Y up while windowing system Y down)
     * If is there any active text edit, then just deselect object
     */
    m_FrameBuffer.mapForRead(GL::Framebuffer::ColorAttachment{ 1 });
    Image2D data = m_FrameBuffer.read(
        Range2Di::fromSize({ event.position().x(),
                             m_FrameBuffer.viewport().sizeY() - event.position().y() - 1 },
                           { 1, 1 }),
        { m_PixelFormat });

    uint32_t idx = 0;
    if(m_PixelFormat == PixelFormat::R8UI) {
        idx = static_cast<uint32_t>(data.data()[0]);
    } else if(m_PixelFormat == PixelFormat::R16UI) {
        idx = static_cast<uint32_t>(Containers::arrayCast<uint16_t>(data.data())[0]);
    } else if(m_PixelFormat == PixelFormat::R32UI) {
        idx = Containers::arrayCast<uint32_t>(data.data())[0];
    } else {
        Fatal{} << "Invalid pixel format";
    }

    PickableObject::updateSelectedObject(idx);
    event.setAccepted();
}

/****************************************************************************************************/
bool PickableApplication::editPointTransformation(Matrix4& objMat) {
    const auto camMat = m_Camera->viewMatrix();
    const auto prjMat = m_Camera->camera().projectionMatrix();
    ImGuizmo::BeginFrame();
    bool bEdited = ImGui::InputFloat3("Position", &objMat.data()[12], 3);
    ImGuizmo::Manipulate(camMat.data(), prjMat.data(),
                         ImGuizmo::TRANSLATE, ImGuizmo::WORLD,
                         objMat.data(),
                         nullptr, nullptr, nullptr, nullptr);
    return bEdited || ImGuizmo::IsUsing();
}

/****************************************************************************************************/
void PickableApplication::setPointTransformation(size_t selectedObjID, const Matrix4& objMat,
                                                 Containers::Array<Vector3>& points) {
    const auto it = m_mDrawableIdxToPointIdx.find(selectedObjID);
    CORRADE_INTERNAL_ASSERT(it != m_mDrawableIdxToPointIdx.end());

    const Vector3& translation = objMat[3].xyz();
    points[it->second] = translation;
}
