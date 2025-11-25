#include "mesh.h"
#include <stdio.h>
#include <stdlib.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

static void get_file_data(void* ctx, const char* filename, const int is_mtl,
                          const char* obj_filename, char** data, size_t* len) {
    (void)ctx; (void)is_mtl; (void)obj_filename;

    FILE* f = fopen(filename, "rb");
    if (!f) {
        *data = NULL;
        *len = 0;
        return;
    }

    fseek(f, 0, SEEK_END);
    *len = ftell(f);
    fseek(f, 0, SEEK_SET);

    *data = (char*)malloc(*len + 1);
    if (*data) {
        fread(*data, 1, *len, f);
        (*data)[*len] = '\0';
    }
    fclose(f);
}

Mesh Mesh_Load(const char* filename) {
    Mesh mesh = {0};

    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;

    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;

    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes,
                                &materials, &num_materials,
                                filename, get_file_data, NULL, flags);

    if (ret != TINYOBJ_SUCCESS) {
        printf("Error loading mesh: %s\n", filename);
        return mesh;
    }

    size_t total_vertices = attrib.num_face_num_verts * 3;

    Vertex* vertices = (Vertex*)malloc(total_vertices * sizeof(Vertex));
    size_t v_cursor = 0;

    for (size_t i = 0; i < num_shapes; i++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < attrib.num_face_num_verts; f++) {
            for (int v = 0; v < 3; v++) {
                tinyobj_vertex_index_t idx = attrib.faces[index_offset + v];
                vertices[v_cursor].position[0] = attrib.vertices[3 * idx.v_idx + 0];
                vertices[v_cursor].position[1] = attrib.vertices[3 * idx.v_idx + 1];
                vertices[v_cursor].position[2] = attrib.vertices[3 * idx.v_idx + 2];

                if (attrib.num_normals > 0) {
                    vertices[v_cursor].normal[0] = attrib.normals[3 * idx.vn_idx + 0];
                    vertices[v_cursor].normal[1] = attrib.normals[3 * idx.vn_idx + 1];
                    vertices[v_cursor].normal[2] = attrib.normals[3 * idx.vn_idx + 2];
                } else {
                    vertices[v_cursor].normal[0] = 0.0f;
                    vertices[v_cursor].normal[1] = 1.0f;
                    vertices[v_cursor].normal[2] = 0.0f;
                }

                if (attrib.num_texcoords > 0) {
                    vertices[v_cursor].texcoord[0] = attrib.texcoords[2 * idx.vt_idx + 0];
                    vertices[v_cursor].texcoord[1] = 1.0f - attrib.texcoords[2 * idx.vt_idx + 1];
                } else {
                    vertices[v_cursor].texcoord[0] = 0.0f;
                    vertices[v_cursor].texcoord[1] = 0.0f;
                }

                v_cursor++;
            }
            index_offset += 3;
        }
    }

    mesh.vertex_count = (GLsizei)v_cursor;

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertex_count * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, texcoord));

    glBindVertexArray(0);

    free(vertices);
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return mesh;
}

void Mesh_Draw(Mesh* mesh) {
    if (mesh->vertex_count > 0) {
        glBindVertexArray(mesh->vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertex_count);
        glBindVertexArray(0);
    }
}

void Mesh_Free(Mesh* mesh) {
    if (mesh->vao != 0) {
        glDeleteVertexArrays(1, &mesh->vao);
        glDeleteBuffers(1, &mesh->vbo);
        mesh->vertex_count = 0;
    }
}
