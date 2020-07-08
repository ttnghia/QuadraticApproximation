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

#include "Application/GLApplication.h"

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Context.h>
#include <Magnum/GL/Version.h>

/****************************************************************************************************/
GLApplication::GLApplication(const std::string& title, const Arguments& arguments,
                             const Vector2i& defaultWindowSize) :
    Platform::Application{arguments, NoCreate} {
    /* Disable OpenGL debug info */
    Debug suppressOutput{ nullptr };
    (void)suppressOutput;

    /* Setup window */
    create(Configuration{}
               .setTitle(title)
               .setSize(defaultWindowSize)
               .setWindowFlags(Configuration::WindowFlag::Resizable),
           GLConfiguration{}.setSampleCount(0));
    #ifndef MAGNUM_TARGET_GLES
    MAGNUM_ASSERT_GL_VERSION_SUPPORTED(GL::Version::GL330);
    #endif
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::setClearColor(m_BkgColor);
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    setSwapInterval(1);
    #endif

    /* Setup scene objects and camera */
    m_Grid.emplace(&m_Scene, &m_Drawables);

    /* Configure camera */
    setupCamera();
}

/****************************************************************************************************/
void GLApplication::viewportEvent(ViewportEvent& event) {
    const auto newBufferSize = event.framebufferSize();

    /* Resize the main framebuffer */
    GL::defaultFramebuffer.setViewport({ {}, newBufferSize });

    /* Resize camera */
    m_Camera->reshape(event.windowSize(), event.framebufferSize());
}

/****************************************************************************************************/
void GLApplication::keyPressEvent(KeyEvent& event) {
    switch(event.key()) {
        #ifndef CORRADE_TARGET_EMSCRIPTEN
        case KeyEvent::Key::V:
            m_bVsync ^= true;
            setSwapInterval(m_bVsync);
            event.setAccepted(true);
            break;
        #endif
        case KeyEvent::Key::S:
            if(m_Camera->lagging() > 0.0f) {
                Debug{} << "Disable camera smooth navigation";
                m_Camera->setLagging(0.0f);
            } else {
                Debug{} << "Enable camera smooth navigation";
                m_Camera->setLagging(0.85f);
            }
            break;
        case KeyEvent::Key::R:
            m_Camera->reset();
            break;
        case KeyEvent::Key::Esc:
            exit(0);
    }
}

/****************************************************************************************************/
void GLApplication::mousePressEvent(MouseEvent& event) {
    m_Camera->initTransformation(event.position());
}

/****************************************************************************************************/
void GLApplication::mouseMoveEvent(MouseMoveEvent& event) {
    if(!event.buttons()) { return; }
    if(event.buttons() & MouseMoveEvent::Button::Left) {
        m_Camera->rotate(event.position());
    } else {
        m_Camera->translate(event.position());
    }
    event.setAccepted();
}

/****************************************************************************************************/
void GLApplication::mouseScrollEvent(MouseScrollEvent& event) {
    const float delta = event.offset().y();
    if(std::abs(delta) < 1.0e-2f) {
        return;
    }
    m_Camera->zoom(delta);
    event.setAccepted();
}

/****************************************************************************************************/
void GLApplication::setupCamera() {
    m_Camera.emplace(m_Scene, m_DefaultCamPosition, m_DefaultCamTarget, Vector3::yAxis(),
                     45.0_degf, windowSize(), framebufferSize());
    m_Camera->setLagging(0.85f);
}
