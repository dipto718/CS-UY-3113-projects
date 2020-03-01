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

SDL_Window* displayWindow;
bool gameIsRunning = true;

// Set the number of
// previous ticks to be
// 0 when the program
// starts
float prevTick = 0;
float currTick;
float numTicks;

// Defines the shader used and the
// matrixes used
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

// renders the game
SDL_Renderer* rend;

// sets the sizes and starting positions
// for the paddles and the ball
SDL_Rect pad1 = {0, 0, 20, 100};
SDL_Rect pad2 = {620, 0, 20, 100};
SDL_Rect ball = {320, 240, 20, 20};

// Determines the directions
// that the ball goes
int xDirection = 1;
int yDirection = 1;

// Determines whether or
// not the game has started
bool start = false;

// exits the game
void Shutdown() {
    SDL_Quit();
}

// returns whether there was a collision
bool getCollision(SDL_Rect rect1, SDL_Rect rect2) {
    float xDist = fabs(rect2.x - rect1.x) - ((rect2.w + rect1.w) / 2.0f);
    float yDist = fabs(rect2.y - rect1.y) - ((rect2.h + rect1.h) / 2.0f);
    return xDist < 0 && yDist < 0;
}

// Draws the respective rectangle
// in red
void draw(SDL_Rect rect) {
    SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
    SDL_RenderFillRect(rend, &rect);
    SDL_RenderPresent(rend);
}

// Initialized the components
// of the game
void Initialize() {
    // creates the window
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    // creates the rendere
    rend = SDL_CreateRenderer(displayWindow, -1, SDL_RENDERER_ACCELERATED);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    // loads the shaders
    program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");

    // sets the various matrixes
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    // draws the initial state
    // of the games
    draw(pad1);
    draw(pad2);
    draw(ball);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}


void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Stops the game and program completely
        // when the window is closed
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
        // moves one paddle up and down
        // with the up and down key and
        // moves the other paddle up and down
        // with the w and s key
        // starts the game by pressing
        // the space key
        else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_UP:
                if (pad2.y > 0 && start)
                    pad2.y -= 40;
                break;
            case SDLK_DOWN:
                if (pad2.y < 380 && start)
                    pad2.y += 40;
                break;
            case SDLK_w:
                if (pad1.y > 0 && start)
                    pad1.y -= 40;
                break;
            case SDLK_s:
                if (pad1.y < 380 && start)
                    pad1.y += 40;
                break;
            case SDLK_SPACE:
                start = true;
                break;
            }
        }
    }
}



// Makes changes to the game each frame
void Update() {

    // moves the ball
    ball.x += 200 * numTicks * xDirection;
    ball.y += 200 * numTicks * yDirection;
    // ends the game if the ball makes it past either paddle
    if (ball.x >= 620 || ball.x <= 0)
        start = false;
    // changes the ball's direction
    // if the top or bottom
    // wall is hit
    else if (ball.y >= 460 || ball.y <= 0)
        yDirection *= -1;
    // changes the ball's direction if
    // it collides with a paddle
    if (getCollision(pad1, ball) || getCollision(pad2, ball))
        xDirection *= -1;
}

// renders the game
void Render() {
    // inserts a small delay before each drawing
    // so that so many drawings cosecutively 
    // does not cause an issue where the screen freezes
    SDL_Delay(5);
    // Deletes the previous screen
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
    SDL_RenderClear(rend);

    program.SetModelMatrix(modelMatrix);

    // Draws the new updated screen
    draw(pad1);
    draw(pad2);
    draw(ball);

    SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        // Gets the fraction of a second it took
        // for the current frame to happen
        currTick = (float)SDL_GetTicks() / 1000.0f;
        numTicks = currTick - prevTick;
        prevTick = currTick;
        ProcessInput();
        // waits to starts the game until
        // space is pressed
        if (start) {
            Update();
            Render();
        }
    }

    // turns off the game once its done
    Shutdown();
    return 0;
}