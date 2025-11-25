#ifndef RENDER_H
#define RENDER_H
#include "mesh.h"
#include "texture.h"
#include "transform.h"
#include "camera.h"
#include "glad.h"

typedef struct {
    GLuint id;
    GLint model_loc;
    GLint view_loc;
    GLint proj_loc;
    GLint tex_loc;
} ShaderProgram;

ShaderProgram ShaderProgram_Create(const char *vs_src, const char *fs_src);
void ShaderProgram_Destroy(ShaderProgram *sp);

typedef struct Renderable {
    Mesh          *mesh;
    Texture2D     *texture;
    Transform      transform;
    ShaderProgram *shader;
} Renderable;

void Renderable_Draw(Renderable *r, Camera *cam);

#endif
