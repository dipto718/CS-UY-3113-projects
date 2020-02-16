// Author: Sm Shihubullah Dipto

#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

// represents the moving and
// rotating objects respectively
GLuint move;
GLuint rotate;

// Set the number of
// previous ticks to be
// 0 when the program
// starts
float prevTick = 0;

// holds the values for rotation
// and movement respectively
float movePos;
float rotatePos;

// determines whether
// the movement or
// rotation direction
// should change
bool moveBack = false;
bool rotateBack = false;

// Defines the shader used and the
// matrixes used
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix, moveMatrix, rotateMatrix;

void Shutdown() {
    SDL_Quit();
}

// Loads the texture from the location given
GLuint loadTexture(const char* name) {
    // downloads the images into the chars
    int w, h, n;
    unsigned char* pic = stbi_load(name, &w, &h, &n, STBI_rgb_alpha);

    // stops running if the image couldn't be found
    if (pic == NULL) {
        std::cout << "The picture is not in the same file\n";
        Shutdown();
    }

    
    // Gives an id to the image and binds it
    // to a texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic);

    // Magnifies or shrinks the texture as needed and
    // does so with a focus on high resolution
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // frees the image from the RAM
    stbi_image_free(pic);

    // Returns the id of the texture
    return id;
}

// Creates the textured object
void createTexture(GLuint pic) {
    // The positions of the two triangles the texture
    // is written on
    float trianglePos[] = { -.5, -.5, .5, -.5, .5, .5, -.5, -.5, .5, .5, -.5, .5 };

    // The positions of the box formed by
    // the two triangle
    float texturePos[] = { 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0 };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, trianglePos);
    glEnableVertexAttribArray(program.positionAttribute);

    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texturePos);
    glEnableVertexAttribArray(program.texCoordAttribute);

    // Binds the texture to the two triangles
    // and draws them
    glBindTexture(GL_TEXTURE_2D, pic);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}


// Initialize what is necessary for the 2D scene
void Initialize() {
    // creates the window
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    // loads the textures and the appropiate shaders
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    move = loadTexture("robot_yellowJump.png");
    rotate = loadTexture("platformChar_happy.png");

    // sets the various matrixes
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    // sets the positions and matrixes
    // for the moving and rotating
    // object
    movePos = 0;
    rotatePos = 0;
    moveMatrix = glm::mat4(1.0f);
    rotateMatrix = glm::mat4(1.0f);

    // Allows blending so that the objects
    // don't look out of place
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// Stops the scene and program completely
// when the window is closed
void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

// Makes changes to the scene each frame
void Update() {

    // Gets the fraction of a second it took
    // for the current frame to happen
    float currTick = (float)SDL_GetTicks() / 1000.0f;
    float numTicks = currTick - prevTick;
    prevTick = currTick;

    // The moving object moves back and forth diagonally
    // across the screen
    // while the rotating object stays in place and
    // changes the directions of the rotation
    // every 360 degrees
    if (!moveBack)
        movePos += (float).5 * numTicks;
    else
        movePos -= (float).5 * numTicks;
    if (!rotateBack)
        rotatePos += (float)40 * numTicks;
    else
        rotatePos -= (float)40 * numTicks;
    if (movePos >= 3)
        moveBack = true;
    else if (movePos <= -3)
        moveBack = false;
    if (rotatePos >= 360)
        rotateBack = true;
    else if (rotatePos <= -360)
        rotateBack = false;
    moveMatrix = glm::mat4(1.0f);
    moveMatrix = glm::translate(moveMatrix, glm::vec3(movePos, movePos, 0));
    rotateMatrix = glm::mat4(1.0f); 
    rotateMatrix = glm::rotate(rotateMatrix, glm::degrees(rotatePos), glm::vec3(0,0,1));
}

// Renders the two objects
// and the background
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    program.SetModelMatrix(moveMatrix);
    createTexture(move);

    program.SetModelMatrix(rotateMatrix);
    createTexture(rotate);

    SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}