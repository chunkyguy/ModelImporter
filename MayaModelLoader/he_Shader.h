//
//  he_Shader.h
//  MayaModelLoader
//
//  Created by Sid on 14/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#ifndef __MayaModelLoader__he_Shader__
#define __MayaModelLoader__he_Shader__

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "he_BitFlag.h"

#define kAttrib_Position 	0
#define kAttrib_Normal 		1
#define kAttrib_Texcoord 	2

/** Create a new shader object 
 * @param vsh_src The vertex shader source
 * @param fsh_src The fragment shader source
 * @param attribs_flag The attributes flags to be enalbed.
 * @return A new shader object.
 */
GLuint CreateShader(const char *vsh_src, const char *fsh_src, const he_BitFlag attribs_flag);

/** Delete the shader object */
void DeleteShader(GLuint shader);
#endif /* defined(__MayaModelLoader__he_Shader__) */
