#include <cglm/cglm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "render.h"
#include "mesh.h"

void Renderable_Draw(Renderable *r, Camera *cam) {
    if (!r->mesh || r->mesh->vertex_count == 0 || !r->shader)
        return;

    ShaderProgram *sp = r->shader;
    glUseProgram(sp->id);

    if (sp->view_loc  >= 0){
        glUniformMatrix4fv(sp->view_loc,  1, GL_FALSE, (float*)cam->view);
    }

    if (sp->proj_loc  >= 0){
        glUniformMatrix4fv(sp->proj_loc,  1, GL_FALSE, (float*)cam->projection);
    }

    mat4 model;
    Transform_ToMat4(&r->transform, model);
    if (sp->model_loc >= 0) glUniformMatrix4fv(sp->model_loc, 1, GL_FALSE, (float*)model);

    if (sp->tex_loc >= 0 && r->texture && r->texture->id != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, r->texture->id);
        glUniform1i(sp->tex_loc, 0);
    }

    Mesh_Draw(r->mesh);
}

#define INFO_LOG_BUFFER_SIZE 1024

static bool check_shader_compile_errors(GLuint shader, const char* type) {
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        char infoLog[INFO_LOG_BUFFER_SIZE] = {0};
        glGetShaderInfoLog(shader, INFO_LOG_BUFFER_SIZE, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n", type, infoLog);
    }
    return success == GL_TRUE;
}

static bool check_program_link_errors(GLuint program) {
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        char infoLog[INFO_LOG_BUFFER_SIZE] = {0};
        glGetProgramInfoLog(program, INFO_LOG_BUFFER_SIZE, NULL, infoLog);
        fprintf(stderr, "ERROR::PROGRAM_LINKING_ERROR\n%s\n", infoLog);
    }
    return success == GL_TRUE;
}

ShaderProgram ShaderProgram_Create(const char *vs_path, const char *fs_path)
{
    ShaderProgram sp = (ShaderProgram){0};

    int vs_fd = -1, fs_fd = -1;
    char *vs_src = NULL, *fs_src = NULL;
    size_t vs_size = 0, fs_size = 0;

    GLuint vs = 0, fs = 0;
    bool ok = true;

    vs_fd = open(vs_path, O_RDONLY);
    if (vs_fd == -1) {
        perror("open vertex shader");
        ok = false;
        goto cleanup;
    }

    struct stat st;
    if (fstat(vs_fd, &st) == -1) {
        perror("fstat vertex shader");
        ok = false;
        goto cleanup;
    }
    vs_size = (size_t)st.st_size;

    vs_src = mmap(NULL, vs_size, PROT_READ, MAP_PRIVATE, vs_fd, 0);
    if (vs_src == MAP_FAILED) {
        perror("mmap vertex shader");
        vs_src = NULL;
        ok = false;
        goto cleanup;
    }

    fs_fd = open(fs_path, O_RDONLY);
    if (fs_fd == -1) {
        perror("open fragment shader");
        ok = false;
        goto cleanup;
    }

    if (fstat(fs_fd, &st) == -1) {
        perror("fstat fragment shader");
        ok = false;
        goto cleanup;
    }
    fs_size = (size_t)st.st_size;

    fs_src = mmap(NULL, fs_size, PROT_READ, MAP_PRIVATE, fs_fd, 0);
    if (fs_src == MAP_FAILED) {
        perror("mmap fragment shader");
        fs_src = NULL;
        ok = false;
        goto cleanup;
    }

    vs = glCreateShader(GL_VERTEX_SHADER);
    {
        const GLchar *src = (const GLchar*)vs_src;
        GLint len = (GLint)vs_size;
        glShaderSource(vs, 1, &src, &len);
    }
    glCompileShader(vs);
    if (!check_shader_compile_errors(vs, "vertex")) {
        ok = false;
        goto cleanup;
    }

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const GLchar *src = (const GLchar*)fs_src;
        GLint len = (GLint)fs_size;
        glShaderSource(fs, 1, &src, &len);
    }
    glCompileShader(fs);
    if (!check_shader_compile_errors(fs, "fragment")) {
        ok = false;
        goto cleanup;
    }

    sp.id = glCreateProgram();
    glAttachShader(sp.id, vs);
    glAttachShader(sp.id, fs);
    glLinkProgram(sp.id);
    if (!check_program_link_errors(sp.id)) {
        glDeleteProgram(sp.id);
        sp.id = 0;
        ok = false;
        goto cleanup;
    }

    sp.model_loc = glGetUniformLocation(sp.id, "model");
    sp.view_loc  = glGetUniformLocation(sp.id, "view");
    sp.proj_loc  = glGetUniformLocation(sp.id, "projection");
    sp.tex_loc   = glGetUniformLocation(sp.id, "uTexture"); // -1 se non esiste

cleanup:
    if (vs) glDeleteShader(vs);
    if (fs) glDeleteShader(fs);

    if (vs_src) munmap(vs_src, vs_size);
    if (fs_src) munmap(fs_src, fs_size);
    if (vs_fd != -1) close(vs_fd);
    if (fs_fd != -1) close(fs_fd);

    if (!ok) {
        sp.id = 0;
        sp.model_loc = sp.view_loc = sp.proj_loc = sp.tex_loc = -1;
    }

    return sp;
}

void ShaderProgram_Destroy(ShaderProgram *sp)
{
    if (!sp) return;

    if (sp->id != 0) {
        glDeleteProgram(sp->id);
        sp->id = 0;
    }

    sp->model_loc = -1;
    sp->view_loc  = -1;
    sp->proj_loc  = -1;
    sp->tex_loc   = -1;
}
