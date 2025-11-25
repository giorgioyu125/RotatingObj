#include <cglm/cglm.h>
#include "transform.h"

void Transform_Init(Transform *t) {
    glm_vec3_zero(t->position);
    glm_vec3_zero(t->rotation);
    glm_vec3_one(t->scale);
}

void Transform_ToMat4(Transform *t, mat4 out) {
    glm_mat4_identity(out);

    glm_translate(out, t->position);

    glm_rotate(out, t->rotation[1], (vec3){0.0f, 1.0f, 0.0f}); // yaw   (Y)
    glm_rotate(out, t->rotation[0], (vec3){1.0f, 0.0f, 0.0f}); // pitch (X)
    glm_rotate(out, t->rotation[2], (vec3){0.0f, 0.0f, 1.0f}); // roll  (Z)

    glm_scale(out, t->scale);
}
