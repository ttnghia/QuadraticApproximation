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

#include "LineShader.h"

#include <Corrade/Containers/Reference.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

/****************************************************************************************************/
LineShader::LineShader() {
    GL::Shader vertShader{ GL::Version::GL330, GL::Shader::Type::Vertex };
    GL::Shader fragShader{ GL::Version::GL330, GL::Shader::Type::Fragment };

    const std::string srcVert = R"(
        uniform highp mat4 transformationProjectionMatrix;

        /* Matches LineShader::Position and LineShader::Normal definitions */
        layout(location = 0) in highp vec4 position;

        void main() {
            gl_Position = position;
        }
    )";

    const std::string srcFrag = R"(
        uniform lowp vec3 color;
        layout(location = 0) out lowp vec4 fragmentColor;

        void main() {
            fragmentColor = vec4(color, 1.0);
        }
    )";

    vertShader.addSource(srcVert);
    fragShader.addSource(srcFrag);
    CORRADE_INTERNAL_ASSERT(GL::Shader::compile({ vertShader, fragShader }));
    attachShaders({ vertShader, fragShader });
    CORRADE_INTERNAL_ASSERT(link());
    m_uColor = uniformLocation("color");
}

/****************************************************************************************************/
LineShader& LineShader::setColor(const Color3& color) {
    setUniform(m_uColor, color);
    return *this;
}
