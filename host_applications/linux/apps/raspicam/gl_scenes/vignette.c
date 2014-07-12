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

#include "vignette.h"
#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>


/* Simple vertical blur shader  */

static RASPITEXUTIL_SHADER_PROGRAM_T vignette_shader = {
    .vertex_source =
    "attribute vec2 vertex;\n"
    "varying vec2 texcoord;"
    "void main(void) {\n"
    "   texcoord = 0.5 * (vertex + 1.0);\n"
    "   gl_Position = vec4(vertex, 0.0, 1.0);\n"
    "}\n",

    .fragment_source =
    "#extension GL_OES_EGL_image_external : require\n" \
    "uniform samplerExternalOES tex;\n" \
    "varying vec2 texcoord;\n" \
    "const float darkness = 1.0;\n" \
    "const float offset = 1.0;\n" \
    "void main(void) {\n" \
      "vec4 texel = texture2D( tex, texcoord );\n" \
      "vec2 uv = ( texcoord - vec2( 0.5 ) ) * vec2( offset );\n" \
      "gl_FragColor = vec4( mix( texel.rgb, vec3( 1.0 - darkness ), dot( uv, uv ) ), texel.a );\n" \
    "}\n"
    .uniform_names = {"tex"},
    .attribute_names = {"vertex"},
};


static int vignette_init(RASPITEX_STATE *state)
{
    int rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
       goto end;

    rc = raspitexutil_build_shader_program(&vignette_shader);

end:
    return rc;
}


/**
 * Creates the OpenGL ES 2.X context and builds the shaders.
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
static int vignette_init(RASPITEX_STATE *state)
{
    int rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
       goto end;

    rc = raspitexutil_build_shader_program(&vignette_shader);

end:
    return rc;
}



/* Redraws the scene with the latest luma buffer.
 *
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
static int vignette_redraw(RASPITEX_STATE *state)
{

    // Start with a clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the OES texture which is used to render the camera preview
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, state->texture);

    GLCHK(glUseProgram(vignette_shader.program));
    GLCHK(glEnableVertexAttribArray(vignette_shader.attribute_locations[0]));
    GLfloat varray[] = {
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f,

        -1.0f,  1.0f,
        1.0f,  1.0f,
        -1.0f, -1.0f,
    };
    GLCHK(glVertexAttribPointer(vignette_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));

    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    GLCHK(glDisableVertexAttribArray(vignette_shader.attribute_locations[0]));
    GLCHK(glUseProgram(0));

   return 0;
}

int vignette_open(RASPITEX_STATE *state)
{
   state->ops.gl_init = vignette_init;
   state->ops.redraw = vignette_redraw;
   state->ops.update_texture = raspitexutil_update_texture;
   return 0;
}
