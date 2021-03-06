//
//  Shader.vsh
//  MayaModelLoader
//
//  Created by Sid on 09/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

attribute vec4 a_Position;
attribute vec3 a_Normal;
attribute vec2 a_Texcoord;

varying lowp vec4 v_Color;
varying lowp vec2 v_Texcoord;

uniform mat4 u_Mvp;
uniform mat3 u_N;

void main()
{
 vec3 E = normalize(u_N * a_Normal);
 vec3 L = vec3(0.0, 0.0, 1.0);
 float nDotVP = max(0.0, dot(E, normalize(L)));
 vec4 diffuse = vec4(1.0, 1.0, 1.0, 1.0);
 
 v_Color = diffuse * nDotVP;
 v_Texcoord = a_Texcoord;
 gl_Position = u_Mvp * a_Position;
}
