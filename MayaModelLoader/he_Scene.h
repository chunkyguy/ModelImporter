//
//  he_Scene.h
//  MayaModelLoader
//
//  Created by Sid on 13/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#ifndef __MayaModelLoader__he_Scene__
#define __MayaModelLoader__he_Scene__

namespace he_Scene {
 
 /** Load data */
 void Create(const char *model_path,
             const char *txvsh_src, const char *txfsh_src,
             const char *clrvsh_src, const char *clrfsh_src);
 
 /** Delete data */
 void Destroy();
 
 /** Set up */
 void Reshape(int w, int h);
 
 /** update in ms */
 void Update(int dt);
 
 /** render */
 void Render();
 
}
#endif /* defined(__MayaModelLoader__he_Scene__) */
