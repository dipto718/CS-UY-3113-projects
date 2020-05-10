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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

// represents the player
Entity* player;
// represents all the walls
Entity* walls;

// represents the moving walls
// colliding with them results
// in a loss
// each level adds one more wall
// to the game
// each wall has a different speed
// 1st level has one
// 2nd level has two
// 3rd level has three
Entity* movWalls = new Entity[3];

// hitting it slows down the player
// is destroyed after the collision
// so that the game isn't too difficult
// 2nd level has one
// 3rd level hase two
// halves current velocity
// and stops acceleration
Entity* slowTile = new Entity[2];

// hitting it speeds up the player
// is destroyed after the collision
// so that the game isn't too difficult
// 3rd level has one
// increases current velocity by 50%
Entity* speedTile = new Entity[1];

// Doesn't move but touching it kills
// the player
// 3rd level has one
// beware since speed tiles can
// cause you to to get too fast and
// collide into them
Entity* killerWall = new Entity[1];

// The goal
Entity* portal;

// The current level
int level = 0;
// Whether it is now
// on the menu screen
bool menu = true;

// The music that loops
// during the game
Mix_Music* loop;
// The sound that plays
// when a level is won
Mix_Chunk* soundEffect;


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

// Sets up the first level
void initializeOne() {
    // sets up the player
    player = new Entity();
    player->accel = glm::vec3(0.0f, 0.0f, 0.0f);
    player->textureID = loadTexture("platformChar_happy.png");
    player->position = glm::vec3(0.0f, 2.5f, 0.0f);

    // sets up the first moving wall
    movWalls[0].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    movWalls[0].textureID = loadTexture("platformPack_tile007.png");
    movWalls[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    movWalls[0].velocity.x = 2.0f;

    // sets up the textures for the walls
    walls = new Entity[34];
    GLuint texWall = loadTexture("platformPack_tile019.png");
    for (int i = 0; i < 34; i++) {
        walls[i].textureID = texWall;
    }

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
    for (int i = 16; i < 25; i++) {
        walls[i].position = glm::vec3(1 + start, -3.75f, 0.0f);
        start += 1.0f;
    }

    // generate cieling walls
    start = -5.0f;
    for (int i = 25; i < 34; i++) {
        walls[i].position = glm::vec3(1 + start, 3.75f, 0.0f);
        start += 1.0f;
    }

    // initialize all the walls
    for (int i = 0; i < 34; i++)
        walls[i].Update(0);

    // sets up the portal
    portal = new Entity();
    portal->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    portal->textureID = loadTexture("platformPack_tile011.png");
    portal->position = glm::vec3(0.0f, -2.5f, 0.0f);
}

// sets up the second level
void initializeTwo() {
    // sets up the player
    player->accel = glm::vec3(0.0f, 0.0f, 0.0f);
    player->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    player->position = glm::vec3(0.0f, 2.5f, 0.0f);

    // sets up the two moving walls
    movWalls[0].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    movWalls[0].position = glm::vec3(0.0f, 1.0f, 0.0f);
    movWalls[0].velocity.x = 2.0f;

    movWalls[1].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    movWalls[1].textureID = loadTexture("platformPack_tile007.png");
    movWalls[1].position = glm::vec3(0.0f, -1.0f, 0.0f);
    movWalls[1].velocity.x = -1.0f;

    // sets up the portal
    portal->velocity = glm::vec3(-0.25f, 0.0f, 0.0f);
    portal->position = glm::vec3(2.0f, -2.5f, 0.0f);

    // sets up the slow down tile
    slowTile[0].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    slowTile[0].textureID = loadTexture("platformPack_tile018.png");
    slowTile[0].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    slowTile[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
}

// sets up the third level
void initializeThree() {
    // sets up the player
    player->accel = glm::vec3(0.0f, 0.0f, 0.0f);
    player->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    player->position = glm::vec3(0.0f, 2.5f, 0.0f);

    // sets up the portal
    portal->velocity = glm::vec3(-1.0f, 0.0f, 0.0f);
    portal->position = glm::vec3(2.0f, -2.5f, 0.0f);

    // sets up the three moving walls
    movWalls[0].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    movWalls[0].position = glm::vec3(-4.0f, 2.0f, 0.0f);
    movWalls[0].velocity.x = 0.25f;

    movWalls[1].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    movWalls[1].position = glm::vec3(1.0f, 0.0f, 0.0f);
    movWalls[1].velocity.x = -1.5f;

    movWalls[2].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    movWalls[2].textureID = loadTexture("platformPack_tile007.png");
    movWalls[2].position = glm::vec3(0.0f, -2.0f, 0.0f);
    movWalls[2].velocity.x = 2.0f;

    // sets up the two slow down tiles
    slowTile[0].isActive = true;
    slowTile[0].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    slowTile[0].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    slowTile[0].position = glm::vec3(1.0f, 2.0f, 0.0f);

    slowTile[1].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    slowTile[1].textureID = loadTexture("platformPack_tile018.png");
    slowTile[1].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    slowTile[1].position = glm::vec3(-1.0f, -1.0f, 0.0f);

    // sets up the speed up tile
    speedTile[0].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    speedTile[0].textureID = loadTexture("platformPack_tile017.png");
    speedTile[0].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    speedTile[0].position = glm::vec3(0.0f, 1.0f, 0.0f);

    // sets up the wall that kills on contact
    killerWall[0].accel = glm::vec3(0.0f, 0.0f, 0.0f);
    killerWall[0].textureID = loadTexture("platformPack_tile030.png");
    killerWall[0].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    killerWall[0].position = glm::vec3(0.0f, -1.0f, 0.0f);
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

// Initialize what is necessary for the game
void Initialize() {
    // creates the window
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Get to the Portal", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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

    // sets up the sound
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    loop = Mix_LoadMUS("music.mp3");
    Mix_PlayMusic(loop, -1);
    soundEffect = Mix_LoadWAV("success.wav");

    glUseProgram(program.programID);

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
        // acceleration to the left, right, up, and down happens
        // when the key is released and only if the
        // game has not ended
        // space starts the game if it is still
        // on the menu screen
        else if (event.type == SDL_KEYUP && !end) {
            if (menu) {
                switch (event.key.keysym.sym) {
                case SDLK_SPACE:
                    level += 1;
                    menu = false;
                    initializeOne();
                    break;
                }
            }
            else{
                switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    player->accel.x -= .2;
                    break;
                case SDLK_RIGHT:
                    player->accel.x += .2;
                    break;
                case SDLK_UP:
                    player->accel.y += .2;
                    break;
                case SDLK_DOWN:
                    player->accel.y -= .2;
                    break;
                case SDLK_SPACE:
                    initializeThree();
                    break;
                }
            }
        }
    }
}

// Makes changes to the scene each frame
// at a consistent rate of time
// The changes made are to the player
// and the various obstacle
// presented in the game along with
// the goal portal
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
        if (!menu) {
            player->Update(FIXED_TIMESTEP);
            movWalls[0].Update(FIXED_TIMESTEP);
            if (level > 1) {
                movWalls[1].Update(FIXED_TIMESTEP);
                slowTile[0].Update(FIXED_TIMESTEP);
            }
            if (level > 2) {
                movWalls[2].Update(FIXED_TIMESTEP);
                slowTile[1].Update(FIXED_TIMESTEP);

                killerWall[0].Update(FIXED_TIMESTEP);
                speedTile[0].Update(FIXED_TIMESTEP);
            }
            portal->Update(FIXED_TIMESTEP);
        }
        deltaTime -= FIXED_TIMESTEP; 
    }
    accumulator = deltaTime;
}

// Renders the walls, the goal portal,
// the player, and the various obstacles
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    // displays the menu screen
    // and how to start the game
    if (menu){
        font = loadTexture("font1.png");
        DrawText(&program, font, "Press space to start", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
        DrawText(&program, font, "Get to the Portal", 0.5f, -0.25f, glm::vec3(-2.0f, 0.0f, 0.0f));
    }
    // renders parts of the game
    // as needed for the levels
    else {
        for (int i = 0; i < 34; i++)
            walls[i].Render(&program);

        movWalls[0].Render(&program);
        if (level > 1) {
            movWalls[1].Render(&program);
            slowTile[0].Render(&program);
        }
        if (level > 2) {
            movWalls[2].Render(&program);
            slowTile[1].Render(&program);

            killerWall[0].Render(&program);
            speedTile[0].Render(&program);
        }
        player->Render(&program);
        portal->Render(&program);

        // displays win or loss
        // when the game ends
        if (end) {
            font = loadTexture("font1.png");
            if (win)
                DrawText(&program, font, "You Win", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
            else
                DrawText(&program, font, "Game Over", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
        }
    }
    SDL_GL_SwapWindow(displayWindow);
}

// Runs the game until it ends
int main(int argc, char* argv[]) {
    // Initializes everything
    Initialize();
    while (gameIsRunning) {
        // starts only after the menu
        // has been exited and the
        // game started
        if (!end && !menu) {
            ProcessInput();
            // ends the game with a loss
            // if the player collides with
            // one of the moving walls
            for (int j = 0; j < level; j++) {
                if (movWalls[j].getCollision(player)) {
                    end = true;
                    win = false;
                    break;
                }
            }

            if (level > 1) {
                // slows down the player
                // and stops acceleration
                // upon collision
                if (slowTile[0].getCollision(player)) {
                    player->velocity *= .5;
                    player->accel = glm::vec3(0);
                    slowTile[0].isActive = false;
                }
            }

            if (level > 2) {
                // slows down the player
                // and stops acceleration
                // upon collision
                if (slowTile[1].getCollision(player)) {
                    player->velocity *= .5;
                    player->accel = glm::vec3(0);
                    slowTile[1].isActive = false;
                }
                // slows down the player
                // upon collision
                if (speedTile[0].getCollision(player)) {
                    player->velocity *= 1.5;
                    //player->accel = glm::vec3(0);
                    speedTile[0].isActive = false;
                }
                // ends the game with a loss
                // upon collision with the player
                if (killerWall[0].getCollision(player)) {
                    end = true;
                    win = false;
                }
            }

            // detects whether the player.
            // portal when it moves,
            // or moving walls should 
            // bounce off the border walls
            if (!end) {
                for (int i = 0; i < 34; i++) {
                    if (player->getCollision(&walls[i])) {
                        player->blocked = true;
                    }
                    for (int j = 0; j < level; j++) {
                        if (movWalls[j].getCollision(&walls[i])) {
                            movWalls[j].blocked = true;
                        }
                    }
                    if (portal->getCollision(&walls[i])) {
                        portal->blocked = true;
                    }
                }
            }
            // updates the state
            // of the game
            Update();
            // detects if the player has
            // reached the portal
            // and initializes the next level
            // as needed
            if (player->getCollision(portal)) {
                // plays a sound to signal
                // that the portal has been
                // reached
                Mix_PlayChannel(-1, soundEffect, 0);
                level += 1;
                if (level == 2) {
                    initializeTwo();
                }
                else if (level == 3) {
                    initializeThree();
                }
                // once all the levels
                // have been completed
                // the game ends in a win
                else if (level == 4) {
                    end = true;
                    win = true;
                }
            }
            // renders the game
            Render();
        }
        // Still processes input
        // and renders the screen
        // while the game is on
        // the menu screen
        else if (menu) {
            ProcessInput();
            Update();
            Render();
        }
        else {
            // Is here so that the window can still be closed
            ProcessInput();
        }
    }
    // turns off the game
    Shutdown();
    return 0;
}