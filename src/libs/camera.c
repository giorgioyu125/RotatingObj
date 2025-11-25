#include "camera.h"
#include <cglm/cglm.h>

void Camera_MovePosition(Camera *cam,
                         float deltaTime,
                         bool moveForward,
                         bool moveBackward,
                         bool moveLeft,
                         bool moveRight,
                         bool moveUp,
                         bool moveDown,
                         float speed)
{
    if (!moveForward && !moveBackward &&
        !moveLeft    && !moveRight   &&
        !moveUp      && !moveDown)
    {
        return; // nessun input, niente da fare
    }

    float moveStep = speed * deltaTime;

    vec3 forward;
    glm_vec3_copy(cam->front, forward);

    vec3 right;
    glm_vec3_crossn(cam->front, cam->up, right); // right = front x up

    vec3 upDir;
    glm_vec3_copy(cam->up, upDir);
    glm_vec3_normalize(upDir);

    if (moveForward) {
        vec3 delta;
        glm_vec3_scale(forward, moveStep, delta);
        glm_vec3_add(cam->position, delta, cam->position);
    }
    if (moveBackward) {
        vec3 delta;
        glm_vec3_scale(forward, moveStep, delta);
        glm_vec3_sub(cam->position, delta, cam->position);
    }
    if (moveRight) {
        vec3 delta;
        glm_vec3_scale(right, moveStep, delta);
        glm_vec3_add(cam->position, delta, cam->position);
    }
    if (moveLeft) {
        vec3 delta;
        glm_vec3_scale(right, moveStep, delta);
        glm_vec3_sub(cam->position, delta, cam->position);
    }
    if (moveUp) {
        vec3 delta;
        glm_vec3_scale(upDir, moveStep, delta);
        glm_vec3_add(cam->position, delta, cam->position);
    }
    if (moveDown) {
        vec3 delta;
        glm_vec3_scale(upDir, moveStep, delta);
        glm_vec3_sub(cam->position, delta, cam->position);
    }
}
