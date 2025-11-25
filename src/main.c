#include "SDL3/SDL_init.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_video.h"
#include <SDL3/SDL_keycode.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "glad.h"
#include <cglm/cglm.h>
#include <math.h>
#include <stdbool.h>

#include "mesh.h"
#include "texture.h"
#include "transform.h"
#include "camera.h"
#include "render.h"

#define UP_DIRECTION ((vec3){0.0f, 1.0f, 0.0f})
#define MAX_RENDERABLES 16

typedef struct AppState {
    SDL_Window   *window;
    SDL_GLContext gl_context;

    ShaderProgram shaderTextured;

    Mesh      cubeMesh;
    Texture2D cubeTexture;

    Renderable renderables[MAX_RENDERABLES];
    int        renderableCount;

    Camera camera;

    int screenWidth;
    int screenHeight;

    Uint64 start_time;
    float  lastTimeSec;

    bool moveForward, moveBackward, moveLeft, moveRight, moveUp, moveDown;
} AppState;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    AppState *state = SDL_calloc(1, sizeof(AppState));
    if (!state) return SDL_APP_FAILURE;
    *appstate = state;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    state->window = SDL_CreateWindow("MyGame", 1200, 800, SDL_WINDOW_OPENGL);
    if (!state->window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->gl_context = SDL_GL_CreateContext(state->window);
    if (!state->gl_context) {
        SDL_Log("Couldn't create context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!gladLoadGL()) {
        SDL_Log("Couldn't load OpenGL: fatal error");
        return SDL_APP_FAILURE;
    }

    SDL_Log("OpenGL Version: %s\n", glGetString(GL_VERSION));

    SDL_GetWindowSizeInPixels(state->window, &state->screenWidth, &state->screenHeight);
    glViewport(0, 0, state->screenWidth, state->screenHeight);

    state->shaderTextured = ShaderProgram_Create(
        "shaders/shader.vert",
        "shaders/shader.frag"
    );
    if (state->shaderTextured.id == 0) {
        SDL_Log("Failed to create textured shader program");
        return SDL_APP_FAILURE;
    }

    glUseProgram(state->shaderTextured.id);
    if (state->shaderTextured.tex_loc >= 0) {
        glUniform1i(state->shaderTextured.tex_loc, 0); 
    }

    state->start_time  = SDL_GetTicks();
    state->lastTimeSec = SDL_GetTicks() / 1000.0f;

    glm_vec3_copy((vec3){0.0f, 2.0f, 5.0f}, state->camera.position);
    glm_vec3_copy((vec3){0.0f, 0.0f,-1.0f}, state->camera.front);
    glm_vec3_copy(UP_DIRECTION,              state->camera.up);

    state->cubeMesh = Mesh_Load("assets/cube.obj");
    if (state->cubeMesh.vertex_count == 0) {
        SDL_Log("Warning: cube mesh failed to load or is empty.");
    }

    state->cubeTexture = Texture2D_Load(
        "assets/cube.jpg",
        GL_REPEAT, GL_REPEAT,
        GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,
        1
    );
    if (state->cubeTexture.id == 0) {
        SDL_Log("Failed to load cube texture.");
        return SDL_APP_FAILURE;
    }

    // Instance 0
    state->renderables[0].mesh    = &state->cubeMesh;
    state->renderables[0].texture = &state->cubeTexture;
    state->renderables[0].shader  = &state->shaderTextured;
    Transform_Init(&state->renderables[0].transform);
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, state->renderables[0].transform.position);

    // Instance 1
    state->renderables[1].mesh    = &state->cubeMesh;
    state->renderables[1].texture = &state->cubeTexture;
    state->renderables[1].shader  = &state->shaderTextured;
    Transform_Init(&state->renderables[1].transform);
    glm_vec3_copy((vec3){3.0f, 0.0f, 0.0f}, state->renderables[1].transform.position);

    // Instance 2
    state->renderables[2].mesh    = &state->cubeMesh;
    state->renderables[2].texture = &state->cubeTexture;
    state->renderables[2].shader  = &state->shaderTextured;
    Transform_Init(&state->renderables[2].transform);
    glm_vec3_copy((vec3){-3.0f, 0.0f, 0.0f}, state->renderables[2].transform.position);

    state->renderableCount = 3;

    glm_vec3_copy((vec3){0.0f, 2.0f, 5.0f}, state->camera.position);
    glm_vec3_copy((vec3){0.0f, 0.0f,-1.0f}, state->camera.front);
    glm_vec3_copy(UP_DIRECTION,              state->camera.up);

    state->camera.yaw   = glm_rad(-90.0f);
    state->camera.pitch = 0.0f;

    SDL_SetWindowRelativeMouseMode(state->window, true);

    glEnable(GL_DEPTH_TEST);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    AppState *state = (AppState*)appstate;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        SDL_Keycode key = event->key.key;
        if (key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        if (key == SDLK_W)      state->moveForward  = true;
        if (key == SDLK_S)      state->moveBackward = true;
        if (key == SDLK_A)      state->moveLeft     = true;
        if (key == SDLK_D)      state->moveRight    = true;
        if (key == SDLK_SPACE)  state->moveUp       = true;
        if (key == SDLK_LCTRL)  state->moveDown     = true;
    }

    if (event->type == SDL_EVENT_KEY_UP) {
        SDL_Keycode key = event->key.key;
        if (key == SDLK_W)      state->moveForward  = false;
        if (key == SDLK_S)      state->moveBackward = false;
        if (key == SDLK_A)      state->moveLeft     = false;
        if (key == SDLK_D)      state->moveRight    = false;
        if (key == SDLK_SPACE)  state->moveUp       = false;
        if (key == SDLK_LCTRL)  state->moveDown     = false;
    }

    if (event->type == SDL_EVENT_MOUSE_MOTION) {
        float sensitivity = 0.0025f;

        float dx = (float)event->motion.xrel;
        float dy = (float)event->motion.yrel;

        state->camera.yaw   += dx * sensitivity;
        state->camera.pitch -= dy * sensitivity; 

        float maxPitch = glm_rad(89.0f);
        if (state->camera.pitch >  maxPitch) state->camera.pitch =  maxPitch;
        if (state->camera.pitch < -maxPitch) state->camera.pitch = -maxPitch;

        vec3 dir;
        float cp = cosf(state->camera.pitch);
        dir[0] = cosf(state->camera.yaw) * cp;
        dir[1] = sinf(state->camera.pitch);
        dir[2] = sinf(state->camera.yaw) * cp;
        glm_vec3_normalize_to(dir, state->camera.front);

        vec3 right;
        glm_vec3_crossn(state->camera.front, UP_DIRECTION, right);
        glm_vec3_crossn(right, state->camera.front, state->camera.up);
    }

    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        SDL_GetWindowSizeInPixels(state->window,
                                  &state->screenWidth,
                                  &state->screenHeight);
        glViewport(0, 0, state->screenWidth, state->screenHeight);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    AppState *state = (AppState*)appstate;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float currentSec = SDL_GetTicks() / 1000.0f;
    float deltaTime  = currentSec - state->lastTimeSec;
    state->lastTimeSec = currentSec;

    Camera_MovePosition(&state->camera,
                        deltaTime,
                        state->moveForward,
                        state->moveBackward,
                        state->moveLeft,
                        state->moveRight,
                        state->moveUp,
                        state->moveDown,
                        3.0f);

    float aspect = 1.0f;
    if (state->screenHeight > 0) {
        aspect = (float)state->screenWidth / (float)state->screenHeight;
    }
    glm_perspective(glm_rad(50.0f), aspect, 0.1f, 100.0f, state->camera.projection);

    vec3 center;
    glm_vec3_add(state->camera.position, state->camera.front, center);
    glm_lookat(state->camera.position, center, state->camera.up, state->camera.view);

    float secondsSinceStart = (SDL_GetTicks() - state->start_time) / 1000.0f;
    state->renderables[0].transform.rotation[1] = secondsSinceStart;
    state->renderables[1].transform.rotation[1] = secondsSinceStart;
    state->renderables[2].transform.rotation[1] = secondsSinceStart;

    for (int i = 0; i < state->renderableCount; ++i) {
        Renderable_Draw(&state->renderables[i], &state->camera);
    }

    if (!SDL_GL_SwapWindow(state->window)) {
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;
    AppState *state = (AppState*)appstate;
    if (state) {
        Texture2D_Destroy(&state->cubeTexture);
        Mesh_Free(&state->cubeMesh);
        ShaderProgram_Destroy(&state->shaderTextured);

        SDL_GL_DestroyContext(state->gl_context);
        SDL_DestroyWindow(state->window);
        SDL_free(state);
    }
}
