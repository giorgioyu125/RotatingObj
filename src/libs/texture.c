#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "texture.h"
#include <stdio.h>

Texture2D Texture2D_Load(const char *path,
                         GLint wrap_s, GLint wrap_t,
                         GLint min_filter, GLint mag_filter,
                         int flip_y)
{
    Texture2D tex = {0};

    int w, h, channels;
    stbi_set_flip_vertically_on_load(flip_y);
    unsigned char *data = stbi_load(path, &w, &h, &channels, 4);
    if (!data) {
        fprintf(stderr, "Failed to load texture '%s': %s\n",
                path, stbi_failure_reason());
        return tex;
    }

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    tex.width  = w;
    tex.height = h;
    return tex;
}

void Texture2D_Destroy(Texture2D *tex){
    if (tex && tex->id != 0) {
        glDeleteTextures(1, &tex->id);
        tex->id = 0;
        tex->width = tex->height = 0;
    }
}
