#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad.h"

typedef struct Texture2D {
    GLuint id;
    int width;
    int height;
} Texture2D;

Texture2D Texture2D_Load(const char *path,
                         GLint wrap_s, GLint wrap_t,
                         GLint min_filter, GLint mag_filter,
                         int flip_y);

void Texture2D_Destroy(Texture2D *tex);

#endif
