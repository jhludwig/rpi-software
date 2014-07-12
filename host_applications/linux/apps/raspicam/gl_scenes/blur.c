/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, Tim Gover
All rights reserved.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Copyright (c) 2014, Surround.IO
Copyright (c), John Ludwig
All rights reserved.

*/

#include "blur.h"
#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>


/* Simple vertical blur shader  */

static RASPITEXUTIL_SHADER_PROGRAM_T blur_shader = {
    .vertex_source =
    "attribute vec2 vertex;\n"
    "varying vec2 texcoord;"
    "void main(void) {\n"
    "   texcoord = 0.5 * (vertex + 1.0);\n"
    "   gl_Position = vec4(vertex, 0.0, 1.0);\n"
    "}\n",

    .fragment_source =
    "#extension GL_OES_EGL_image_external : require\n"
    "uniform samplerExternalOES tex;\n"
    "const float blurSize = 1.0/512.0;\n" \
    "varying vec2 texcoord;\n"
    "void main(void) {\n" \
    "    vec4 sum = vec4(0.0);\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y - 4.0*blurSize)) * 0.05;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y - 3.0*blurSize)) * 0.09;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y - 2.0*blurSize)) * 0.12;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y - blurSize)) * 0.15;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y)) * 0.16;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y + blurSize)) * 0.15;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y + 2.0*blurSize)) * 0.12;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y + 3.0*blurSize)) * 0.09;\n" \
    "    sum += texture2D(tex, vec2(texCoord.x, texCoord.y + 4.0*blurSize)) * 0.05;\n" \
    "    gl_FragColor = sum;\n" \
    "    gl_FragColor.a = 1.0;\n" \
    "}\n"
    .uniform_names = {"tex"},
    .attribute_names = {"vertex"},
};

/**
 * Creates the OpenGL ES 2.X context and builds the shaders.
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
static int blur_init(RASPITEX_STATE *state)
{
    int rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
       goto end;

    rc = raspitexutil_build_shader_program(&blur_shader);

end:
    return rc;
}

static int blur_redraw(RASPITEX_STATE *state) {

    // Start with a clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the OES texture which is used to render the camera preview
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, state->texture);

    GLCHK(glUseProgram(blur_shader.program));
    GLCHK(glEnableVertexAttribArray(blur_shader.attribute_locations[0]));
    GLfloat varray[] = {
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f,

        -1.0f,  1.0f,
        1.0f,  1.0f,
        -1.0f, -1.0f,
    };
    GLCHK(glVertexAttribPointer(blur_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));

    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    GLCHK(glDisableVertexAttribArray(blur_shader.attribute_locations[0]));
    GLCHK(glUseProgram(0));
    return 0;
}



int blur_open(RASPITEX_STATE *state)
{
   state->ops.gl_init = blur_init;
   state->ops.redraw = blur_redraw;
   state->ops.update_texture = raspitexutil_update_texture;
   return 0;
}
