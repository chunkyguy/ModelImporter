//
//  he_Scene.cpp
//  MayaModelLoader
//
//  Created by Sid on 13/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#include "he_Scene.h"

#include <cassert>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <GLKit/GLKMath.h>

#include "he_Assimp.h"
#include "he_Image.h"
#include "he_Shader.h"
#include "he_ResourcePath.h"

namespace he_Scene {
 
 // This is for a shader uniform block
 struct he_Material {
  aiColor4D diffuse;
  aiColor4D ambient;
  aiColor4D specular;
  aiColor4D emissive;
  float shininess;
  int texCount;
  
  he_Material *next;
 };
 
 // Information to render each assimp node
 struct he_Mesh {
  GLuint vao;
  GLuint e_vbo; /*elements*/
  GLuint a_vbo; /*positions*/
  GLuint n_vbo; /*normals*/
  GLuint t_vbo; /*texture*/
  GLuint texID;
  int numFaces;
  he_Material material;
  
  he_Mesh *next;
 };
 
 struct he_Texture {
  GLuint texID;
  aiString name;
  
  he_Texture *next;
 };
 
 struct he_Uniform {
  GLuint mvpMat;
  GLuint nMat;
  GLuint tex;
 };
 
 he_Model		*g_Model;
 GLuint 		g_txShader;
 GLuint 		g_clrShader;
 he_Mesh 		*g_Meshes;
 he_Texture 	*g_Textures;
 he_Material	*g_Materials;
 GLKMatrix4 	g_ProjMat;
 GLKMatrix4 	g_WorldMat;
 he_Uniform 	g_Uniforms;
 float 			g_Rotation;
 
 static void debug_matrix(const GLKMatrix4 &mat)
 {
  for (int i = 0; i < 4; ++i) {
   printf("%f\t%f\t%f\t%f\n",mat.m[i*4+0],mat.m[i*4+1],mat.m[i*4+2],mat.m[i*4+3]);
  }
 }
 
 static void debug_vec3(const aiVector3D vec)
 {
  printf("%f\t%f\t%f\n",vec.x,vec.y,vec.z);
 }
 
 /* search for a texture */
 static he_Texture *search_texture_named(const aiString &name)
 {
  for (he_Texture *t = g_Textures; t; t = t->next) {
   if (t->name == name) {
    return t;
   }
  }
  return NULL;
 }
 
 /** Extract filename out of the path */
 static const char *extract_filename(const char *path)
 {
  char *last_slash = strrchr(path, '/');
  if (!last_slash) { /*test for windows*/
   last_slash = strrchr(path, '\\');
  }
  return (last_slash && strlen(last_slash)>1)?++last_slash:NULL;
 }
 
 
 static void load_textures()
 {
  g_Textures = NULL;
  he_Texture *curr_tex = NULL;
  
  /* scan materials for textures */
  for (unsigned int m = 0; m < g_Model->scene->mNumMaterials; ++m) {
   aiString path; /* filename */
   for (int texIndex = 0; g_Model->scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path) == AI_SUCCESS; ++texIndex) {
    he_Texture *tmp_tex = new he_Texture;
    tmp_tex->name = path;
    glGenTextures(1, &tmp_tex->texID);
    he_Image tmp_img;
    char img_path[1024];
    Image_Create(&tmp_img, BundlePath(img_path, extract_filename(path.C_Str())));
    glBindTexture(GL_TEXTURE_2D, tmp_tex->texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tmp_img.width, tmp_img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp_img.pixels);
    
    if (!curr_tex) {
     curr_tex = tmp_tex;
     g_Textures = tmp_tex;
    } else {
     curr_tex->next = tmp_tex;
     curr_tex = tmp_tex;
    }
    curr_tex->next = NULL;
   }
  }
 }
 
 static void unload_texture()
 {
  if (!g_Textures) {
   return;
  }
  
  he_Texture *q = g_Textures;
  for (he_Texture *p = q->next; p; q = p, p = p->next) {
   delete q;
  }
  delete q;
 }
 
 static he_Mesh *search_mesh_at_index(unsigned int index)
 {
  he_Mesh *m = NULL;
  for (m = g_Meshes; m && index--; m = m->next) {
  }
  return m;
 }
 
 static void load_buffer()
 {
  g_Meshes = NULL;
  he_Mesh *curr_mesh = NULL;
  
  /** for each mesh */
  for (unsigned int m = 0; m < g_Model->scene->mNumMeshes; ++m) {
   const aiMesh *mesh = g_Model->scene->mMeshes[m];
   
   /* create array with faces. Convert Assimpt format to array*/
   unsigned int *face_array = new unsigned int[mesh->mNumFaces * 3];
   unsigned int face_index = 0;
   for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
    const aiFace *face = &mesh->mFaces[f];
    memcpy(&face_array[face_index], face->mIndices, 3 * sizeof(unsigned int));
    face_index += 3;
   }
   
   he_Mesh *tmp_mesh = new he_Mesh;
   bzero(tmp_mesh, sizeof(he_Mesh));
   
   tmp_mesh->numFaces = mesh->mNumFaces;
   glGenVertexArraysOES(1, &tmp_mesh->vao);
   glBindVertexArrayOES(tmp_mesh->vao);
   
   /*face buffer*/
   glGenBuffers(1, &tmp_mesh->e_vbo);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmp_mesh->e_vbo);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, face_array, GL_STATIC_DRAW);
   printf("elements:\n");
   for (int i = 0; i < mesh->mNumFaces; ++i) {
    debug_vec3(aiVector3D(face_array[i*3+0],face_array[i*3+1],face_array[i*3+2]));
   }
   
   /*position buffer*/
   if (mesh->HasPositions()) {
    printf("position:\n");
    glGenBuffers(1, &tmp_mesh->a_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tmp_mesh->a_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(kAttrib_Position);
    glVertexAttribPointer(kAttrib_Position, 3, GL_FLOAT, GL_FALSE, 0, 0);
    for (int i = 0; i < mesh->mNumVertices; ++i) {
     debug_vec3(mesh->mVertices[i]);
    }
   }
   
   /*normal buffer */
   if (mesh->HasNormals()) {
    printf("normal:\n");
    glGenBuffers(1, &tmp_mesh->n_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tmp_mesh->n_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(kAttrib_Normal);
    glVertexAttribPointer(kAttrib_Normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    for (int i = 0; i < mesh->mNumVertices; ++i) {
     debug_vec3(mesh->mNormals[i]);
    }
   }
   
   /*texcoord buffer*/
   if (mesh->HasTextureCoords(0)) {
    printf("texcoord:\n");
    float *texcoords = new float[2 * mesh->mNumVertices];
    for (int t = 0; t < mesh->mNumVertices; ++t) {
     texcoords[t*2] = mesh->mTextureCoords[0][t].x;
     texcoords[t*2+1] = mesh->mTextureCoords[0][t].y;
     debug_vec3(aiVector3D(texcoords[t*2],texcoords[t*2+1],0.0f));
    }
    glGenBuffers(1, &tmp_mesh->t_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, tmp_mesh->t_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, texcoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(kAttrib_Texcoord);
    glVertexAttribPointer(kAttrib_Texcoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
    delete [] texcoords;
   }
   
   /* unbind buffers*/
   glBindVertexArrayOES(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   
   /*material data*/
   aiMaterial *mat = g_Model->scene->mMaterials[mesh->mMaterialIndex];
   aiString texPath; /*filename of material texture*/
   if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
    he_Texture *mat_tex = search_texture_named(texPath);
    tmp_mesh->texID = mat_tex->texID;
    tmp_mesh->material.texCount = 1;
   } else {
    tmp_mesh->material.texCount = 0;
   }
   
   tmp_mesh->material.diffuse = aiColor4D(0.8f, 0.8f, 0.8f, 1.0f);
   aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &tmp_mesh->material.diffuse);
   tmp_mesh->material.ambient = aiColor4D(0.2f, 0.2f, 0.2f, 1.0f);
   aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &tmp_mesh->material.ambient);
   tmp_mesh->material.specular = aiColor4D(0.0f, 0.0f, 0.0f, 1.0f);
   aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &tmp_mesh->material.specular);
   tmp_mesh->material.emissive = aiColor4D(0.0f, 0.0f, 0.0f, 1.0f);
   aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &tmp_mesh->material.emissive);
   tmp_mesh->material.shininess = 0.0f;
   unsigned int max;
   aiGetMaterialFloatArray(mat, AI_MATKEY_SHININESS, &tmp_mesh->material.shininess, &max);
   
   /* link mesh */
   if (!curr_mesh) {
    curr_mesh = tmp_mesh;
    g_Meshes = tmp_mesh;
   } else {
    curr_mesh->next = tmp_mesh;
    curr_mesh = tmp_mesh;
   }
   curr_mesh->next = NULL;
  }
 }
 
 static void delete_mesh(he_Mesh *m)
 {
  if (m->e_vbo) {
   glDeleteBuffers(1, &m->e_vbo);
  }
  if (m->a_vbo) {
   glDeleteBuffers(1, &m->a_vbo);
  }
  if (m->n_vbo) {
   glDeleteBuffers(1, &m->n_vbo);
  }
  if (m->t_vbo) {
   glDeleteBuffers(1, &m->t_vbo);
  }
  if (m->vao) {
   glDeleteVertexArraysOES(1, &m->vao);
  }
  delete m;
 }
 
 static void unload_buffer()
 {
  if (!g_Meshes) {
   return;
  }
  
  he_Mesh *q = g_Meshes;
  for (he_Mesh *p = q->next; p; q = p, p = p->next) {
   delete_mesh(q);
  }
  delete_mesh(q);
 }
 
 void Create(const char *model_path,
             const char *txvsh_src, const char *txfsh_src,
             const char *clrvsh_src, const char *clrfsh_src)
 {
  /* load model */
  g_Model = new he_Model;
  if (!CreateWithFile(g_Model, model_path)) {
   printf("unable to load model from file: %s\n",model_path);
   assert(0);
  }
  
  /* load textures from scene */
  load_textures();
  for (he_Texture *t = g_Textures; t; t = t->next) {
   printf("Loaded texture: %s\n",t->name.C_Str());
  }
  
  /* load shaders */
  g_txShader = CreateShader(txvsh_src, txfsh_src,
                            BF_Mask(kAttrib_Position)|BF_Mask(kAttrib_Normal)|BF_Mask(kAttrib_Texcoord));
  assert(g_txShader);
  g_Uniforms.mvpMat = glGetUniformLocation(g_txShader, "u_Mvp");
  g_Uniforms.nMat = glGetUniformLocation(g_txShader, "u_N");
  g_Uniforms.tex = glGetUniformLocation(g_txShader, "u_Tex");
  
  g_clrShader = CreateShader(clrvsh_src, clrfsh_src,
                             BF_Mask(kAttrib_Position)|BF_Mask(kAttrib_Normal));
  assert(g_clrShader);
  g_Uniforms.mvpMat = glGetUniformLocation(g_clrShader, "u_Mvp");
  g_Uniforms.nMat = glGetUniformLocation(g_clrShader, "u_N");
  
  /* load buffers */
  load_buffer();
  for (he_Mesh *m = g_Meshes; m; m = m->next) {
   assert(m->vao);
   printf("Loaded mesh: faces %d\n",m->numFaces);
  }
  
  /* set GL state */
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  
  g_Rotation = 0.0f;
 }
 
 void Destroy()
 {
  /* delete model */
  delete g_Model;
  
  /* delete textures */
  unload_texture();
  
  /* delete shaders */
  DeleteShader(g_txShader);
  DeleteShader(g_clrShader);
  
  /* delete buffers */
  unload_buffer();
 }
 
 void Reshape(int w, int h)
 {
  /* load proj matrix */
  float aspect = fabsf(float(w) / float(h));
  g_ProjMat = GLKMatrix4MakePerspective(GLKMathDegreesToRadians(65.0f), aspect, 0.1f, 100.0f);
  //  printf("Reshape: projection matrix\n");
  //  debug_matrix(g_ProjMat);
 }
 
 void Update(int dt)
 {
  /* update model */
  g_Rotation += dt * 0.01f;
 }
 
 static void render_node(const aiNode *node)
 {
  aiMatrix4x4 mvMat = node->mTransformation;
  mvMat.Transpose();
  
  GLKMatrix4 node_mvMat = GLKMatrix4Make(mvMat.a1, mvMat.a2, mvMat.a3, mvMat.a4,
                                         mvMat.b1, mvMat.b2, mvMat.b3, mvMat.b4,
                                         mvMat.c1, mvMat.c2, mvMat.c3, mvMat.c4,
                                         mvMat.d1, mvMat.d2, mvMat.d3, mvMat.d4);
//   GLKMatrix4 t_mat = GLKMatrix4MakeTranslation(0.0f, -4.0f, -20.0f);
//   node_mvMat = GLKMatrix4Multiply(node_mvMat, t_mat);
  GLKMatrix4 r_mat = GLKMatrix4MakeRotation(GLKMathDegreesToRadians(g_Rotation), 0.0f, 1.0f, 0.0f);
  node_mvMat = GLKMatrix4Multiply(node_mvMat, r_mat);
  GLKMatrix4 s_mat = GLKMatrix4MakeScale(4.0f, 4.0f, 4.0f);
  node_mvMat = GLKMatrix4Multiply(node_mvMat, s_mat);
  
  /*render all nodes attached to the node*/
  for (unsigned int n = 0; n < node->mNumMeshes; ++n) {
   he_Mesh *mesh = search_mesh_at_index(node->mMeshes[n]);
   
   
   /*bind texture and shader*/
   if (mesh->texID) {
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_Uniforms.tex, 0);
    glBindTexture(GL_TEXTURE_2D, mesh->texID);
    glUseProgram(g_txShader);
   } else {
    glUseProgram(g_clrShader);
   }
   
   /*bind vao*/
   glBindVertexArrayOES(mesh->vao);
   
   /*enable vertex attribs for this mesh*/
   if (mesh->a_vbo) {
    glEnableVertexAttribArray(kAttrib_Position);
   }
   if (mesh->n_vbo) {
    glEnableVertexAttribArray(kAttrib_Normal);
   }
   if (mesh->t_vbo) {
    glEnableVertexAttribArray(kAttrib_Texcoord);
   }
   
   /*bind uniforms*/
   GLKMatrix4 world_mvMatrix = GLKMatrix4Multiply(g_WorldMat, node_mvMat);
   GLKMatrix4 mvpMat = GLKMatrix4Multiply(g_ProjMat, world_mvMatrix);
   glUniformMatrix4fv(g_Uniforms.mvpMat, 1, GL_FALSE, mvpMat.m);
   
   GLKMatrix3 nMat = GLKMatrix3InvertAndTranspose(GLKMatrix4GetMatrix3(world_mvMatrix), NULL);
   glUniformMatrix3fv(g_Uniforms.nMat, 1, GL_FALSE, nMat.m);
   
   /*draw*/
   glDrawElements(GL_TRIANGLES, mesh->numFaces * 3, GL_UNSIGNED_INT, 0);
   
   /*unbind all*/
   glDisableVertexAttribArray(kAttrib_Position);
   glDisableVertexAttribArray(kAttrib_Normal);
   glDisableVertexAttribArray(kAttrib_Texcoord);
   glBindTexture(GL_TEXTURE_2D, 0);
   
  }
  
  /* render all children*/
  for (unsigned int n = 0; n < node->mNumChildren; ++n) {
   render_node(node->mChildren[n]);
  }
 }
 
 void Render()
 {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* update modelview mat */
  g_WorldMat = GLKMatrix4MakeTranslation(0.0f, 0.0f, -10.0f);
  
  /* render model */
  render_node(g_Model->scene->mRootNode);
 }
 
} /*namespace he_Scene*/