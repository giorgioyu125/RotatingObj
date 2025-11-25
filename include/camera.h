#ifndef CAMERA_H
#define CAMERA_H
#include <cglm/cglm.h>
#include <stdbool.h>

typedef struct {
    mat4 view;
    mat4 projection;

    vec3 position;
    vec3 front;
    vec3 up;

    float yaw;
    float pitch;
} Camera;

void Camera_MovePosition(Camera *cam,
                         float deltaTime,
                         bool moveForward,
                         bool moveBackward,
                         bool moveLeft,
                         bool moveRight,
                         bool moveUp,
                         bool moveDown,
                         float speed);

#endif
