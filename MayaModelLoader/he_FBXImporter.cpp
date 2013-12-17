//
//  he_FBXImporter.cpp
//  MayaModelLoader
//
//  Created by Sid on 12/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#include "he_FBXImporter.h"

#include <cstdio>
#include <cassert>

#include <fbxsdk.h>

/** Get account of tabs */
int tabCount = 0;

/**
 * Print tabs for pretty print
 */
static void print_tabs()
{
 for (int i = 0; i < tabCount; ++i) {
  printf("\t");
 }
}

/**
 * Return a string-based representation based on the attribute type.
 */
static FbxString get_attrib_typename(FbxNodeAttribute::EType type)
{
 switch(type) {
  case FbxNodeAttribute::eUnknown: return "unidentified";
  case FbxNodeAttribute::eNull: return "null";
  case FbxNodeAttribute::eMarker: return "marker";
  case FbxNodeAttribute::eSkeleton: return "skeleton";
  case FbxNodeAttribute::eMesh: return "mesh";
  case FbxNodeAttribute::eNurbs: return "nurbs";
  case FbxNodeAttribute::ePatch: return "patch";
  case FbxNodeAttribute::eCamera: return "camera";
  case FbxNodeAttribute::eCameraStereo: return "stereo";
  case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
  case FbxNodeAttribute::eLight: return "light";
  case FbxNodeAttribute::eOpticalReference: return "optical reference";
  case FbxNodeAttribute::eOpticalMarker: return "marker";
  case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
  case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
  case FbxNodeAttribute::eBoundary: return "boundary";
  case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
  case FbxNodeAttribute::eShape: return "shape";
  case FbxNodeAttribute::eLODGroup: return "lodgroup";
  case FbxNodeAttribute::eSubDiv: return "subdiv";
  default: return "unknown";
 }
}

/**
 * Print attribute
 */
static void print_attribute(FbxNodeAttribute *attrib)
{
 if (!attrib) {
  return;
 }
 
 FbxString type = get_attrib_typename(attrib->GetAttributeType());
 FbxString name = attrib->GetName();
 print_tabs();
 
 printf("attribute: %s name: %s\n",type.Buffer(), name.Buffer());
}

/**
 * Prints a node atributes and it's children
 */
static void print_node(FbxNode *node)
{
 print_tabs();
 const char *name = node->GetName();
 FbxDouble3 translation = node->LclTranslation.Get();
 FbxDouble3 rotation = node->LclRotation.Get();
 FbxDouble3 scaling = node->LclScaling.Get();

 printf("name: %s{\nT: {%f, %f, %f}\nR: {%f, %f, %f}\nS: {%f, %f, %f}\n",
        name,
        translation[0],translation[1],translation[2],
        rotation[0],rotation[1],rotation[2],
        scaling[0],scaling[1],scaling[2]);

 tabCount++;

 for (int i = 0; i < node->GetNodeAttributeCount(); ++i) {
  print_attribute(node->GetNodeAttributeByIndex(i));
 }

 for (int i = 0; i < node->GetChildCount(); ++i) {
  print_node(node->GetChild(i));
 }
 
 tabCount--;
 print_tabs();
 printf("}\n");
}

/*
 * filepath is absolute path to the fbx file
 */
void LoadFBXFile(const char *filepath)
{
 /**
  * Program only requires one instance of FbxManager
  * Delete it with static function Destroy when done
  * Destroy cleans all the objects created by FbxManager
  */
 FbxManager *manager = FbxManager::Create();

 /**
  * Create IO settings.
  */
 FbxIOSettings *ios = FbxIOSettings::Create(manager, IOSROOT);
 manager->SetIOSettings(ios);
 
 /**
  * Create an importer
  */
 FbxImporter *importer = FbxImporter::Create(manager, "");
 
 if (!importer->Initialize(filepath, -1, manager->GetIOSettings())) {
  printf("Unable to initialize FbxImported\nFile: %s\nError: %s\n",filepath, importer->GetStatus().GetErrorString());
  assert(0);
 }
 
/**
 * Create an empty scene to be populated by the importer
 */
 FbxScene *scene = FbxScene::Create(manager, "myScene");
 importer->Import(scene);
 importer->Destroy();
 
 /**
  * Parse all the nodes in the scene.
  * The root nodes contains no attributes.
  */
 FbxNode *root = scene->GetRootNode();
 if (root) {
  for (int i = 0; i < root->GetChildCount(); ++i) {
   print_node(root->GetChild(i));
  }
 }
 
 /**
  * Release the manager
  */
 manager->Destroy();
}
