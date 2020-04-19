// Author: Sm Shihubullah Dipto

#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include <vector>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "entity.h"
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

// represents the player
Entity* player;

// the opponents slam down
// when the player gets near
// Each level has the opponents
// be in a different location
Entity* opponents = new Entity[3];
int numOpponents = 3;

// the number of lives
int lives = 3;

// the texture map used
GLuint mapTexture;

// keeps track of the
// game's states
int state = 0;
// 0 = menu
// 1 = level 1
// 2 = level 2
// 3 = level 3

// represents whether the
// game started or not
bool started = false;

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

// defines the map and
// its dimensions
Map* map;
int width = 14;
int height = 5;

// The looping music
// and the jumping sound
Mix_Music* loop;
Mix_Chunk* jump;

// The data used to construct each of
// the three levels
unsigned int level1_data[] =
{
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
1, 1, 1, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0,
2, 2, 2, 2, 0, 0, 2, 2, 0, 0, 0, 3, 3, 3
};
unsigned int level2_data[] =
{
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
1, 0, 1, 0, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0,
2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 3, 3, 3
};
unsigned int level3_data[] =
{
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 0,
1, 1, 0, 0, 1, 0, 2, 2, 0, 1, 0, 3, 3, 3
};

// closes the game and frees
// the music used
void Shutdown() {
    Mix_FreeMusic(loop);
    Mix_FreeChunk(jump);
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

// Initialize what is necessary for the Platformer game
void Initialize() {
    // creates the window and allows sound
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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

    mapTexture = loadTexture("tileset.png");
    map = new Map(width, height, level1_data, mapTexture, 1.0f, 4, 1);

    // sets up the player
    player = new Entity();
    player->accel = glm::vec3(0.0f, -9.81f, 0.0f);
    player->textureID = loadTexture("platformChar_happy.png");
    player->position = glm::vec3(0.0f, 0.0f, 0.0f);

    // sets up the sound
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    loop = Mix_LoadMUS("crypto.mp3");
    Mix_PlayMusic(loop, -1);
    jump = Mix_LoadWAV("bounce.wav");

    // sets up the opponents
    for (int i = 0; i < numOpponents; i++) {
        opponents[i] = *(new Entity());
        opponents[i].accel = glm::vec3(0.0f, 0.0f, 0.0f);
        opponents[i].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        opponents[i].textureID = loadTexture("platformPack_tile007.png");
        opponents[i].position = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    opponents[0].position = glm::vec3(1.0f, 4.0f, 0.0f);
    opponents[1].position = glm::vec3(4.0f, 3.0f, 0.0f);
    opponents[2].position = glm::vec3(8.0f, 4.0f, 0.0f);

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
            end = true;
        }
        // velocityt to the left and right happens
        // when the key is releaed and only if the
        // game has not ended
        // space acts as the jump key
        // the enter on the keypad starts
        // the game if on the menu
        else if (event.type == SDL_KEYUP && !end) {
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                if(started)
                    player->velocity.x -= 1.0f;
                break;
            case SDLK_RIGHT:
                if(started)
                    player->velocity.x += 1.0f;
                break;
            case SDLK_SPACE:
                if (player->collidedBottom == true && started) {
                    player->velocity.y += 5.0f;
                    // plays the jump sound
                    Mix_PlayChannel(-1, jump, 0);
                }
                break;
            case SDLK_KP_ENTER:
                state = 1;
                started = true;
            }
        }
    }
}

// resets a level when
// a life is lost
void reset() {
    lives -= 1;
    if (state == 1) {
        opponents[0].position = glm::vec3(1.0f, 4.0f, 0.0f);
        opponents[1].position = glm::vec3(4.0f, 3.0f, 0.0f);
        opponents[2].position = glm::vec3(8.0f, 4.0f, 0.0f);
        for (int i = 0; i < numOpponents; i++) {
            opponents[i].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    else if (state == 2) {
        opponents[0].position = glm::vec3(2.0f, 4.0f, 0.0f);
        opponents[1].position = glm::vec3(4.0f, 3.0f, 0.0f);
        opponents[2].position = glm::vec3(9.0f, 3.0f, 0.0f);
        for (int i = 0; i < numOpponents; i++) {
            opponents[i].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    else if (state == 3) {
        opponents[0].position = glm::vec3(0.0f, 4.0f, 0.0f);
        opponents[1].position = glm::vec3(1.0f, 4.0f, 0.0f);
        opponents[2].position = glm::vec3(7.0f, 4.0f, 0.0f);
        for (int i = 0; i < numOpponents; i++) {
            opponents[i].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    if (lives != 0) {
        player->accel = glm::vec3(0.0f, -9.81f, 0.0f);
        player->position = glm::vec3(0.0f, 0.0f, 0.0f);
        player->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    else {
        end = true;
        win = false;
        player->accel = glm::vec3(0.0f, 0.0f, 0.0f);
        player->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

// Renders the map, player, the enemies, 
// and the goal area that needs to be reached
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    // changes the view matrix to center
    // on the player
    if (player->position.x > 0 && player->position.x < 14)
        program.SetViewMatrix(viewMatrix);
    map->Render(&program);
    player->Render(&program);
    for (int i = 0; i < numOpponents; i++)
        opponents[i].Render(&program);
    if (started == false) {
        font = loadTexture("font1.png");
        DrawText(&program, font, "Runner: Press Enter to Start", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
    }
    else {
        font = loadTexture("font1.png");
        DrawText(&program, font, "Lives remaining: " + std::to_string(lives), 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
    }

    // displays win or loss
    // when the game ends
    if (end) {
        Mix_HaltMusic();
        font = loadTexture("font1.png");
        if (win)
            DrawText(&program, font, "You Win", 0.5f, -0.25f, player->position);
        else
            DrawText(&program, font, "You Lose", 0.5f, -0.25f, player->position);
    }

    SDL_GL_SwapWindow(displayWindow);
}

// Moves onto the next level
void advance() {
    player->accel = glm::vec3(0.0f, -9.81f, 0.0f);
    player->position = glm::vec3(0.0f, 0.0f, 0.0f);
    player->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    if (state == 1) {
        state += 1;
        opponents[0].position = glm::vec3(2.0f, 4.0f, 0.0f);
        opponents[1].position = glm::vec3(4.0f, 3.0f, 0.0f);
        opponents[2].position = glm::vec3(9.0f, 3.0f, 0.0f);
        for (int i = 0; i < numOpponents; i++) {
            opponents[i].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        map = new Map(width, height, level2_data, mapTexture, 1.0f, 4, 1);
    }
    else if (state == 2) {
        state += 1;
        opponents[0].position = glm::vec3(0.0f, 4.0f, 0.0f);
        opponents[1].position = glm::vec3(1.0f, 4.0f, 0.0f);
        opponents[2].position = glm::vec3(7.0f, 4.0f, 0.0f);
        for (int i = 0; i < numOpponents; i++) {
            opponents[i].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        map = new Map(width, height, level3_data, mapTexture, 1.0f, 4, 1);
    }
}

// Has the enemies slam down when the player is under them
// and slams up if the player attempts to jump over them so that
// tricking them into reaching the ground and then jumping doesn't work
// stops moving when the player is no longer above or below
void trackAI(Entity* opponent){
    if (round(opponent->position.x) == round(player->position.x)) {
        if (round(opponent->position.y) > round(player->position.y)) {
            opponent->velocity.y = -4.5f;
        }
        else {
            opponent->velocity.y = 4.5f;
        }
    }
    else {
        opponent->velocity.y = 0.0f;
    }
}


// Makes changes to the scene each frame
// at a consistent rate of time
// The changes made are to the player
// and the various enemies
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
        player->Update(FIXED_TIMESTEP, player, opponents, numOpponents, map);
        for (int i = 0; i < numOpponents; i++) {
            trackAI(&opponents[i]);
            opponents[i].Update(FIXED_TIMESTEP, player, player, 0, map);
        }
        deltaTime -= FIXED_TIMESTEP;
    }
    // detects if the player fell or got hit by an enemy
    if (player->position.y < -3 || player->getCollision(&opponents[0]) || player->getCollision(&opponents[1]) || player->getCollision(&opponents[2]))
        reset();
    // detects if the player reached the goal
    else if (player->position.x >= 12 && player->collidedBottom == true)
        if (state == 3) {
            win = true;
            end = true;
            player->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        else
            advance();

    accumulator = deltaTime;
    // Allows scrolling
    viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::translate(viewMatrix,glm::vec3(-(player->position.x), 0, 0));
}



// Runs the game until it ends
int main(int argc, char* argv[]) {
    Initialize();
    while (gameIsRunning) {
        ProcessInput();
        while (!end) {
            ProcessInput();
            Update();
            Render();
        }
    }
    // turns off the game
    Shutdown();
    return 0;
}