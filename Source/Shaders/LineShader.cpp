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

#include <Corrade/Containers/Reference.h>
#include <Magnum/GL/Shader.h>
#include <Magnum/GL/Version.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

#include "LineShader.h"

/****************************************************************************************************/
LineShader::LineShader() {
    GL::Shader vertShader{ GL::Version::GL330, GL::Shader::Type::Vertex };
    GL::Shader fragShader{ GL::Version::GL330, GL::Shader::Type::Fragment };
    GL::Shader geomShader{ GL::Version::GL330, GL::Shader::Type::Geometry };

    const std::string srcVert = R"(
        uniform highp mat4 transformationProjectionMatrix;

        /* Matches LineShader::Position and LineShader::Normal definitions */
        layout(location = 0) in highp vec4 position;

        void main() {
            gl_Position = transformationProjectionMatrix * position;
        }
    )";

    const std::string srcFrag = R"(
        uniform lowp vec3 color;
        layout(location = 0) out lowp vec4 fragmentColor;

        void main() {
            fragmentColor = vec4(color, 1.0);
        }
    )";

    const std::string srcGeom = R"(
        uniform lowp float thickness = 5.0;
        uniform lowp float miterLimit = 0.1;
        uniform lowp vec2 viewport;

        layout(lines_adjacency) in;
        layout(triangle_strip, max_vertices = 7) out;

        vec2 toScreenSpace(vec4 vertex) {
            return vec2( vertex.xy / vertex.w ) * viewport;
        }

        float toZValue(vec4 vertex) {
            return (vertex.z/vertex.w);
        }

        void drawSegment(vec2 points[4], float zValues[4]) {
            vec2 p0 = points[0];
            vec2 p1 = points[1];
            vec2 p2 = points[2];
            vec2 p3 = points[3];

            /* perform naive culling */
            vec2 area = viewport * 4.0;
            if( p1.x < -area.x || p1.x > area.x ) return;
            if( p1.y < -area.y || p1.y > area.y ) return;
            if( p2.x < -area.x || p2.x > area.x ) return;
            if( p2.y < -area.y || p2.y > area.y ) return;

            /* determine the direction of each of the 3 segments (previous, current, next) */
            vec2 v0 = normalize( p1 - p0 );
            vec2 v1 = normalize( p2 - p1 );
            vec2 v2 = normalize( p3 - p2 );

            /* determine the normal of each of the 3 segments (previous, current, next) */
            vec2 n0 = vec2( -v0.y, v0.x );
            vec2 n1 = vec2( -v1.y, v1.x );
            vec2 n2 = vec2( -v2.y, v2.x );

            /* determine miter lines by averaging the normals of the 2 segments */
            vec2 miter_a = normalize( n0 + n1 );	// miter at start of current segment
            vec2 miter_b = normalize( n1 + n2 ); // miter at end of current segment

            /* determine the length of the miter by projecting it onto normal and then inverse it */
            float an1 = dot(miter_a, n1);
            float bn1 = dot(miter_b, n2);
            if (an1==0) an1 = 1;
            if (bn1==0) bn1 = 1;
            float length_a = thickness / an1;
            float length_b = thickness / bn1;

            /* prevent excessively long miters at sharp corners */
            if( dot( v0, v1 ) < -miterLimit ) {
                miter_a = n1;
                length_a = thickness;

                /* close the gap */
                if( dot( v0, n1 ) > 0 ) {
                    gl_Position = vec4( ( p1 + thickness * n0 ) / viewport, zValues[1], 1.0 );
                    EmitVertex();

                    gl_Position = vec4( ( p1 + thickness * n1 ) / viewport, zValues[1], 1.0 );
                    EmitVertex();

                    gl_Position = vec4( p1 / viewport, zValues[1], 1.0 );
                    EmitVertex();

                    EndPrimitive();
                }
                else {
                    gl_Position = vec4( ( p1 - thickness * n1 ) / viewport, zValues[1], 1.0 );
                    EmitVertex();

                    gl_Position = vec4( ( p1 - thickness * n0 ) / viewport, zValues[1], 1.0 );
                    EmitVertex();

                    gl_Position = vec4( p1 / viewport, zValues[1], 1.0 );
                    EmitVertex();

                    EndPrimitive();
                }
            }

            if( dot( v1, v2 ) < -miterLimit ) {
                miter_b = n1;
                length_b = thickness;
            }

            // generate the triangle strip
            gl_Position = vec4( ( p1 + length_a * miter_a ) / viewport, zValues[1], 1.0 );
            EmitVertex();

            gl_Position = vec4( ( p1 - length_a * miter_a ) / viewport, zValues[1], 1.0 );
            EmitVertex();

            gl_Position = vec4( ( p2 + length_b * miter_b ) / viewport, zValues[2], 1.0 );
            EmitVertex();

            gl_Position = vec4( ( p2 - length_b * miter_b ) / viewport, zValues[2], 1.0 );
            EmitVertex();

            EndPrimitive();
        }

        void main(void) {
            // 4 points
            vec4 Points[4];
            Points[0] = gl_in[0].gl_Position;
            Points[1] = gl_in[1].gl_Position;
            Points[2] = gl_in[2].gl_Position;
            Points[3] = gl_in[3].gl_Position;

            // screen coords
            vec2 points[4];
            points[0] = toScreenSpace(Points[0]);
            points[1] = toScreenSpace(Points[1]);
            points[2] = toScreenSpace(Points[2]);
            points[3] = toScreenSpace(Points[3]);

            // deepness values
            float zValues[4];
            zValues[0] = toZValue(Points[0]);
            zValues[1] = toZValue(Points[1]);
            zValues[2] = toZValue(Points[2]);
            zValues[3] = toZValue(Points[3]);

            drawSegment(points, zValues);
        }
    )";

    vertShader.addSource(srcVert);
    fragShader.addSource(srcFrag);
    geomShader.addSource(srcGeom);
    CORRADE_INTERNAL_ASSERT(GL::Shader::compile({ vertShader, fragShader, geomShader }));
    attachShaders({ vertShader, fragShader, geomShader });
    CORRADE_INTERNAL_ASSERT(link());

    m_uColor      = uniformLocation("color");
    m_uThickness  = uniformLocation("thickness");
    m_uMiterLimit = uniformLocation("miterLimit");
    m_uViewport   = uniformLocation("viewport");
    m_uTransformationProjectionMatrix = uniformLocation("transformationProjectionMatrix");
}

/****************************************************************************************************/
LineShader& LineShader::setThickness(float thickness) {
    setUniform(m_uThickness, thickness);
    return *this;
}

/****************************************************************************************************/
LineShader& LineShader::setMiterLimit(float limit) {
    setUniform(m_uMiterLimit, limit);
    return *this;
}

/****************************************************************************************************/
LineShader& LineShader::setColor(const Color3& color) {
    setUniform(m_uColor, color);
    return *this;
}

/****************************************************************************************************/
LineShader& LineShader::setViewport(const Vector2i& viewport) {
    setUniform(m_uViewport, Vector2{ viewport });
    return *this;
}

/****************************************************************************************************/
LineShader& LineShader::setTransformationProjectionMatrix(const Matrix4& matrix) {
    setUniform(m_uTransformationProjectionMatrix, matrix);
    return *this;
}
