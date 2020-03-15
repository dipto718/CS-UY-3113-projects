// Author: Sm Shihubullah Dipto

#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "entity.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

// represents the lander
Entity* lander;
// represents all the walls
Entity* walls;

// represents the chosen font
GLuint font;

// used to measure
// and keep track
// of time
#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

// determines whether
// the game ended and
// if there was a win
bool win = false;
bool end = false;

// Defines the shader used and the
// matrixes used
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

// closes the game
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

// writes text to the screen
void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text,
    float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            });
        texCoords.insert(texCoords.end(), {
        u, v,
        u, v + height,
        u + width, v,
        u + width, v + height,
        u + width, v,
        u, v + height,
            });
    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

// Initialize what is necessary for the lunar lander scene
void Initialize() {
    // creates the window
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Lunar Lander", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    // loads the appropiate shaders
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    // sets the various matrixes
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    // sets up the lander
    lander = new Entity();
    lander->accel = glm::vec3(0.0f, -0.1f, 0.0f);
    lander->textureID = loadTexture("platformChar_happy.png");
    lander->position = glm::vec3(0.0f, 4.0f, 0.0f);

    // sets up the textures for the walls
    walls = new Entity[26];
    GLuint texWall = loadTexture("platformPack_tile007.png");
    for (int i = 0; i < 26; i++) {
        walls[i].textureID = texWall;
    }

    // the goal gets a different texture
    walls[23].textureID = loadTexture("platformPack_tile008.png");

    // generate left walls
    float start = -2.75;
    for (int i = 0; i < 8; i++) {
        walls[i].position = glm::vec3(-5.0f, start, 0.0f);
        start += .9375;
    }

    //generate right walls
   start = -2.75;
    for (int i = 8; i < 16; i++) {
        walls[i].position = glm::vec3(5.0f, start, 0.0f);
        start += .9375;
    }

    // generate floor walls
    start = -5.0f;
    for (int i = 16; i < 24; i++) {
        walls[i].position = glm::vec3(1 + start, -3.75f, 0.0f);
        start += 1.0f;
    }

    // adds a wall obstacle to the middle of the screen
    walls[24].textureID = texWall;
    walls[24].position = glm::vec3(0.0f, 0.0f, 0.0f);

    // sets up the other half of the goal
    walls[25].textureID = loadTexture("platformPack_tile008.png");
    walls[25].position = glm::vec3(4.0f, -3.75f, 0.0f);

    // initialize all the walls
    for (int i = 0; i < 26; i++)
        walls[i].Update(0);
    // Allows blending so that the objects
    // don't look out of place
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// allows input to be understood
void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Stops the scene and program completely
        // when the window is closed
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
        // acceleration to the left and right happens
        // when the key is releaed and only if the
        // game has not ended
        else if (event.type == SDL_KEYUP && !end) {
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                lander->accel.x -= .1;
                break;
            case SDLK_RIGHT:
                lander->accel.x += .1;
                break;
            }
        }
    }
}

// Makes changes to the scene each frame
// at a consistent rate of time
// The only change that is made 
// is to the movement of the lander
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        
        lander->Update(FIXED_TIMESTEP);

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;

}

// Renders the walls, the goal area,
// and the lunar lander
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < 26; i++)
        walls[i].Render(&program);


    lander->Render(&program);

    SDL_GL_SwapWindow(displayWindow);
}

// Runs the game until it ends
int main(int argc, char* argv[]) {
    Initialize();
    // The game advances until the lunar
    // lander lands and the game ends
    while (gameIsRunning) {
        if (!end) {
            ProcessInput();
            Update();
            Render();
            for (int i = 0; i < 26; i++)
                // stops the game once a collision occurs
                if (lander->getCollision(&walls[i])) {
                    end = true;
                    // The game is won if the collision was
                    // with the goal area
                    if (i == 23 || i == 25)
                        win = true;
                    // loads the font and displays
                    // whether the game was won
                    // or lost
                    font = loadTexture("font1.png");
                    if (win)
                        DrawText(&program, font, "Mission Successful", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
                    else
                        DrawText(&program, font, "Mission Failed", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
                    SDL_GL_SwapWindow(displayWindow);
                    break;
                }
        }
        else {
            // Is here so that the window can still be closed
            ProcessInput();
        }
    }
    // exits the game
    Shutdown();
    return 0;
}