//
//  Shader.fsh
//  MayaModelLoader
//
//  Created by Sid on 09/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

varying lowp vec4 v_Color;
varying lowp vec2 v_Texcoord;

uniform sampler2D u_Tex;

void main()
{
    gl_FragColor = texture2D(u_Tex, v_Texcoord) * v_Color;
}
