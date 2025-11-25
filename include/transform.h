#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <cglm/cglm.h>

typedef struct {
    vec3 position;
    vec3 rotation; // in radiants, es. {pitch, yaw, roll}
    vec3 scale;
} Transform;

void Transform_Init(Transform *t);
void Transform_ToMat4(Transform *t, mat4 out);

#endif
