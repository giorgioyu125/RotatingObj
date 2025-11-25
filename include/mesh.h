#ifndef MESH_H
#define MESH_H

#include <glad.h>
#include <cglm/cglm.h>

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
} Vertex;

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLsizei vertex_count;
} Mesh;

Mesh Mesh_Load(const char* filepath);
void Mesh_Draw(Mesh* mesh);
void Mesh_Free(Mesh* mesh);

#endif
