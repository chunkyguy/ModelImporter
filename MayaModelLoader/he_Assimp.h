//
//  he_Assimp.h
//  MayaModelLoader
//
//  Created by Sid on 13/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#ifndef __MayaModelLoader__he_Assimp__
#define __MayaModelLoader__he_Assimp__

// assimp include files. These three are usually needed.
#include <assimp/Importer.hpp>	//OO version Header!
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>

struct he_Model {
 // Create an instance of the Importer class
 Assimp::Importer importer;
 
 // the global Assimp scene object
 const aiScene* scene = NULL;
 
 // scale factor for the model to fit in the window
 float scaleFactor;
};

bool CreateWithFile(he_Model *model, const char *filepath);

#endif /* defined(__MayaModelLoader__he_Assimp__) */
