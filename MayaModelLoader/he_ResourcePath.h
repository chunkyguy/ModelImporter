//
//  he_ResourcePath.h
//  OGL_Basic
//
//  Created by Sid on 22/08/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//
#ifndef he_ResourcePath_h
#define he_ResourcePath_h

/** OS Resource paths */
const char *BundlePath(char *buffer, const char *filename);

void ReadFile(char *buffer, const char *path);
#endif
