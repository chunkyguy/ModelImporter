//
//  he_Assimp.cpp
//  MayaModelLoader
//
//  Created by Sid on 13/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#include "he_Assimp.h"


#include <fstream>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)



static void get_bounding_box_for_node (he_Model *model,
                                       const aiNode* nd,
                                       aiVector3D* min,
                                       aiVector3D* max)

{
 aiMatrix4x4 prev;
 unsigned int n = 0, t;
 
 for (; n < nd->mNumMeshes; ++n) {
  const aiMesh* mesh = model->scene->mMeshes[nd->mMeshes[n]];
  for (t = 0; t < mesh->mNumVertices; ++t) {
   
   aiVector3D tmp = mesh->mVertices[t];
   
   min->x = aisgl_min(min->x,tmp.x);
   min->y = aisgl_min(min->y,tmp.y);
   min->z = aisgl_min(min->z,tmp.z);
   
   max->x = aisgl_max(max->x,tmp.x);
   max->y = aisgl_max(max->y,tmp.y);
   max->z = aisgl_max(max->z,tmp.z);
  }
 }
 
 for (n = 0; n < nd->mNumChildren; ++n) {
  get_bounding_box_for_node(model, nd->mChildren[n],min,max);
 }
}


static void get_bounding_box (he_Model *model, aiVector3D* min, aiVector3D* max)
{
 min->x = min->y = min->z =  1e10f;
 max->x = max->y = max->z = -1e10f;
 get_bounding_box_for_node(model, model->scene->mRootNode,min,max);
}


bool CreateWithFile(he_Model *model, const char *filepath)
{
 //check if file exists
 std::ifstream fin(filepath);
 if(!fin.fail()) {
  fin.close();
 }
 else{
  printf("Couldn't open file: %s\n", filepath);
  printf("%s\n", model->importer.GetErrorString());
  return false;
 }
 
 model->scene = model->importer.ReadFile( filepath, aiProcessPreset_TargetRealtime_Quality);
 
 // If the import failed, report it
 if(!model->scene) {
  printf("%s\n", model->importer.GetErrorString());
  return false;
 }
 
 // Now we can access the file's contents.
 printf("Import of scene %s succeeded\n",filepath);
 
 aiVector3D scene_min, scene_max, scene_center;
 get_bounding_box(model, &scene_min, &scene_max);
 float tmp;
 tmp = scene_max.x-scene_min.x;
 tmp = scene_max.y - scene_min.y > tmp?scene_max.y - scene_min.y:tmp;
 tmp = scene_max.z - scene_min.z > tmp?scene_max.z - scene_min.z:tmp;
 model->scaleFactor = 1.f / tmp;
 
 // We're done. Everything will be cleaned up by the importer destructor
 return true;
}
