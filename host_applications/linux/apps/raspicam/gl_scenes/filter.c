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

#include "filter.h"
#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

// allocate shader.  we should create a fully populated standard set of uniforms 
//     that all shaders have access to. 
static RASPITEXUTIL_SHADER_PROGRAM_T filter_shader = {
    .vertex_source = NULL,
    .fragment_source = NULL,
    .uniform_names = {"tex"},
    .attribute_names = {"vertex"},
};


static int filter_init(RASPITEX_STATE *state)
{
    int rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
       goto end;
    vcos_log_info("Getting shader %s", state->filter_name);
    // construct file names for frag and vert shaders
    int filter_name_length = strlen(state->filter_name);
    char frag_name[filter_name_length + 10];
    char vert_name[filter_name_length + 10];
    strcpy(frag_name, state->filter_name);
    strcat(frag_name, ".frag");
    vcos_log_info("Frag name %s", frag_name);
    strcpy(vert_name, state->filter_name);
    strcat(vert_name, ".vert");  
    vcos_log_info("Vert name %s", vert_name);  

    // read in contents of frag and vert shaders
    FILE* fp = fopen(vert_name, "r");
    char* vertex_source = NULL;
    ssize_t bytes_read = getdelim( &vertex_source, 0, '\0', fp);
    if ( bytes_read == -1 ) {
      // handle error
      vcos_log_error("Vertex source unreadable %s", vert_name);
    }
    fclose(fp);
    fp = fopen(frag_name, "r");
    char* fragment_source = NULL;
    bytes_read = getdelim( &fragment_source, 0, '\0', fp);
    if ( bytes_read == -1 ) {
      // handle error
      vcos_log_error("Fragment source unreadable %s", frag_name);
    }
    fclose(fp);    

    // construct a complete shader structure.   
    filter_shader.vertex_source = vertex_source;
    filter_shader.fragment_source = fragment_source;

    rc = raspitexutil_build_shader_program(&filter_shader);

end:
    return rc;
}



/* Redraws the scene with the latest luma buffer.
 *
 * @param raspitex_state A pointer to the GL preview state.
 * @return Zero if successful.
 */
static int filter_redraw(RASPITEX_STATE *state)
{

    // Start with a clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the OES texture which is used to render the camera preview
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, state->texture);

    GLCHK(glUseProgram(filter_shader.program));
    GLCHK(glEnableVertexAttribArray(filter_shader.attribute_locations[0]));
    GLfloat varray[] = {
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f,

        -1.0f,  1.0f,
        1.0f,  1.0f,
        -1.0f, -1.0f,
    };
    GLCHK(glVertexAttribPointer(filter_shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));

    GLCHK(glDrawArrays(GL_TRIANGLES, 0, 6));

    GLCHK(glDisableVertexAttribArray(filter_shader.attribute_locations[0]));
    GLCHK(glUseProgram(0));

   return 0;
}

int filter_open(RASPITEX_STATE *state)
{
   state->ops.gl_init = filter_init;
   state->ops.redraw = filter_redraw;
   state->ops.update_texture = raspitexutil_update_texture;
   return 0;
}
