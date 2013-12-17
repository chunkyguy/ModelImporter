//
//  he_Shader.cpp
//  MayaModelLoader
//
//  Created by Sid on 14/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#include "he_Shader.h"

#include <cassert>
#include <cstdio>


/** Print shader debug message
 * glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
 * glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei *length, GLchar *infolog)
 *
 * glGetProgramiv(GLuint program, GLenum pname, GLint *params)
 * glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei *length, GLchar *infolog)
 
 */
static void debug_shader(GLuint shader,
                         void(*getLogLen)(GLuint object, GLenum pname, GLint *params),
                         void(*getLog)(GLuint object, GLsizei bufsize, GLsizei *length, GLchar *infolog))
{
 GLint logLength;
 getLogLen(shader, GL_INFO_LOG_LENGTH, &logLength);
 if (logLength > 0) {
  GLchar *log = new GLchar[logLength];
  getLog(shader, logLength, &logLength, log);
  printf("Shader compile log:\n%s\n", log);
  delete [] log;
 }
}

/** Compile shader from source */
GLuint CreateShader(const char *vsh_src, const char *fsh_src, const he_BitFlag attribs_flag)
{
 GLint status;
 GLuint shader = glCreateProgram();
 
 GLuint vsh = glCreateShader(GL_VERTEX_SHADER);
 glShaderSource(vsh, 1, &vsh_src, NULL);
 glCompileShader(vsh);
 debug_shader(vsh, glGetShaderiv, glGetShaderInfoLog);
 glGetShaderiv(vsh, GL_COMPILE_STATUS, &status);
 assert(status);
 
 GLuint fsh = glCreateShader(GL_FRAGMENT_SHADER);
 glShaderSource(fsh, 1, &fsh_src, NULL);
 glCompileShader(fsh);
 debug_shader(fsh, glGetShaderiv, glGetShaderInfoLog);
 glGetShaderiv(vsh, GL_COMPILE_STATUS, &status);
 assert(status);
 
 glAttachShader(shader, vsh);
 glAttachShader(shader, fsh);
 
 
 if (BF_IsSet(attribs_flag, kAttrib_Position)) {
  glBindAttribLocation(shader, kAttrib_Position, "a_Position");
 }
 if (BF_IsSet(attribs_flag, kAttrib_Normal)) {
  glBindAttribLocation(shader, kAttrib_Normal, "a_Normal");
 }
 if (BF_IsSet(attribs_flag, kAttrib_Texcoord)) {
  glBindAttribLocation(shader, kAttrib_Texcoord, "a_Texcoord");
 }

 glLinkProgram(shader);
 debug_shader(shader, glGetProgramiv, glGetProgramInfoLog);
 glGetProgramiv(shader, GL_LINK_STATUS, &status);
 assert(status);
 
 glDetachShader(shader, vsh);
 glDeleteShader(vsh);
 glDetachShader(shader, fsh);
 glDeleteShader(fsh);
 
 return shader;
}

void DeleteShader(GLuint shader)
{
 glDeleteProgram(shader);
}

