//
//  he_OBJImporter.m
//  MayaModelLoader
//
//  Created by Sid on 09/12/13.
//  Copyright (c) 2013 whackylabs. All rights reserved.
//

#include "he_OBJImporter.h"

#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#define FLOATS_PER_POSITION 	3 /*3 floats per v*/
#define FLOATS_PER_TEXCOORD 	2 /*2 floats per vt*/
#define FLOATS_PER_NORMAL		3 /*3 floats per vn*/
#define PRIMITIVES_PER_FACE		4 /*4 face indices per f*/

/** Read a word from the file stream. The word is delimeted by a whitespace.
 * @param buffer copy the read word in to the buffer
 * @param file The stream from where the data is to be read
 @return the length of the read word
 */
static int read_word(FILE *file, char *buffer)
{
 int len = 0;
 
 for (int ch = fgetc(file); !isspace(ch); ch = fgetc(file)) {
  buffer[len++] = ch;
 }
 buffer[len] = '\0';
 
 return len;
}

/** move the file pointer to the next line 
 * @param file The file in context.
 * @return false if EOF is encountered
 */
static bool next_line(FILE *file)
{
 for (int ch = fgetc(file); ch != '\n'; ch = fgetc(file)) {
  if (ch == EOF) {
   fputc(ch, file); /* put back for the next read */
   return false;
  }
 }
 return true;
}

/** read floats into the buffer from the file 
 * @param buffer The float data buffer
 * @param file The file in context
 * count The number of requested floats to be copied
 * @return The actual number of floats copied
 */
static int copy_floats(float *buffer, FILE *file, const int count)
{
 char word[1024];
 int i = 0;
 while (i < count && read_word(file, word)) {
  buffer[i++] = atof(word);
 }
 return i;
}

/** read primitives into the buffer from the file
 * @param buffer The primitive data buffer
 * @param file The file in context
 * count The number of requested primitives to be copied
 * @return The actual number of primitives copied
 */
static int copy_faces(int *buffer, FILE *file, int count)
{
 char word[1024];
 int i = 0;
 while (i < count && read_word(file, word)) {
  i = sscanf(word, "%d/%d/%d",&buffer[0],&buffer[1],&buffer[2]);
 }
 return i;
}

/** Create Mesh data from a Maya OBJ file
 * @param vertex_data_bf The vertex data buffer. Callee owns the data.
 * @return The vertex primitives count.
 */
int CreateMeshDataFromMayaOBJ(float *vertex_data_bf, const char *filepath)
{
 FILE *file = fopen(filepath, "r");
 char word[1024];

 int v_datai = 0;
 float *v_data = NULL;
 int v_count = 0;

 int vt_datai = 0;
 float *vt_data = NULL;
 int vt_count = 0;
 
 int vn_datai = 0;
 float *vn_data = NULL;
 int vn_count = 0;
 
 int f_count = 0;
 int *f_data = NULL;
 int f_datai = 0;
 
 /* count number of data types */
 while (read_word(file, word)) {
  if (strcmp(word, "v") == 0) { /*vertex data*/
   v_count++;
  } else if (strcmp(word, "vt") == 0) { /*vertex texture*/
   vt_count++;
  } else if (strcmp(word, "vn") == 0) { /*vertex normal*/
   vn_count++;
  } else if (strcmp(word, "f") == 0) { /*face data*/
   f_count++;
  }
  next_line(file);
 }
 
 /* fill the data */
 v_data = (float *)malloc(sizeof(float) * v_count * FLOATS_PER_POSITION);
 vt_data = (float *)malloc(sizeof(float) * vt_count * FLOATS_PER_TEXCOORD);
 vn_data = (float *)malloc(sizeof(float) * vn_count * FLOATS_PER_NORMAL);
 f_data = (int *)malloc(sizeof(int) * f_count * 3 * PRIMITIVES_PER_FACE);

 fseek(file, 0, SEEK_SET);
 while (read_word(file, word)) {
  if (strcmp(word, "v") == 0) { /*vertex data*/
   v_datai += copy_floats(&v_data[v_datai], file, FLOATS_PER_POSITION);
  } else if (strcmp(word, "vt") == 0) { /*vertex texture*/
   vt_datai += copy_floats(&vt_data[vt_datai], file, FLOATS_PER_TEXCOORD);
  } else if (strcmp(word, "vn") == 0) { /*vertex normal*/
   vn_datai += copy_floats(&vn_data[vn_datai], file, FLOATS_PER_NORMAL);
  } else if (strcmp(word, "f") == 0) { /*face data*/
   f_datai += copy_faces(&f_data[f_datai], file, PRIMITIVES_PER_FACE);
  }
 }
 
 fclose(file);
 
 /* calculate interleaved vertex data 
  	the faces are provided in quad format, convert them to triangles.
  	A sample quad is provided as ccw ABDC, converting it to ABCD to be renderable as triangle-strip
  */
 int vertex_primitives = f_count * PRIMITIVES_PER_FACE;
 vertex_data_bf = (float *)malloc(sizeof(float) * vertex_primitives * (FLOATS_PER_POSITION+FLOATS_PER_TEXCOORD+FLOATS_PER_NORMAL));
 int vertex_datai = 0;

 for (int f = 0; f < f_count; ++f) {
  for (int p = 0; p < PRIMITIVES_PER_FACE; ++p) {
   /* swap primitive 2 and 3 (see comment above) */
   if (p == 2) {
    p = 3;
   } else if (p == 3) {
    p = 2;
   }
   
   int v_index = *(f_data + f + p + 0);
   memcpy(vertex_data_bf + vertex_datai, v_data + v_index, sizeof(float)*FLOATS_PER_POSITION);
   vertex_datai += FLOATS_PER_POSITION;
   
   int vt_index = *(f_data + f + p + 1);
   memcpy(vertex_data_bf + vertex_datai, vt_data + vt_index, sizeof(float)*FLOATS_PER_TEXCOORD);
   vertex_datai += FLOATS_PER_TEXCOORD;
   
   int vn_index = *(f_data + f + p + 2);
   memcpy(vertex_data_bf + vertex_datai, vn_data + vn_index, sizeof(float)*FLOATS_PER_NORMAL);
   vertex_datai += FLOATS_PER_NORMAL;
  }
 }
 
 free(v_data);
 free(vt_data);
 free(vn_data);
 free(f_data);
 
 return vertex_primitives;
}
