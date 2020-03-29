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

// represents the player
Entity* player;
// represents all the walls
Entity* walls;

// represents the three enemies

// bounces around
Entity* enemyBounce;

// blocks the player and
// dashes if they are on the same row
Entity* enemyBlockerDasher;

// tracks the player after
// the player first enters its
// row or clumn
Entity* enemyTracker;

// the number of enemies
int enemyCount = 3;

// the weapon that
// is needed to defeat
// the enemies
Entity* weapon;
bool hasWeapon = false;


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

// Initialize what is necessary for the Rise of the AI game
void Initialize() {
    // creates the window
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Rise of the AI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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

    // sets up the player
    player = new Entity();
    player->accel = glm::vec3(0.0f, 0.0f, 0.0f);
    player->textureID = loadTexture("platformChar_happy.png");
    player->position = glm::vec3(0.0f, 2.0f, 0.0f);

    // sets up the various enemies and the behavior for
    // the bouncing enemy

    enemyBounce = new Entity();
    enemyBounce->velocity = glm::vec3(-1.0f, 1.0f, 0.0f);
    enemyBounce->textureID = loadTexture("platformPack_tile008.png");
    enemyBounce->position = glm::vec3(0.0f, -1.0f, 0.0f);

    enemyTracker = new Entity();
    enemyTracker->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    enemyTracker->textureID = loadTexture("platformPack_tile008.png");
    enemyTracker->position = glm::vec3(1.0f, -2.5f, 0.0f);

    enemyBlockerDasher = new Entity();
    enemyBlockerDasher->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    enemyBlockerDasher->textureID = loadTexture("platformPack_tile008.png");
    enemyBlockerDasher->position = glm::vec3(-2.0f, 0.0f, 0.0f);

    // creates and places the weapon
    // enemies can only be beaten after
    // getting the weapon
    weapon = new Entity();
    weapon->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    weapon->textureID = loadTexture("platformPack_tile023.png");
    weapon->position = glm::vec3(0.0f, 0.0f, 0.0f);

    // sets up the textures for the walls
    walls = new Entity[34];
    GLuint texWall = loadTexture("platformPack_tile007.png");
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
        // acceleration to the left, right, up, and down happens
        // when the key is releaed and only if the
        // game has not ended
        // space acts as a break
        else if (event.type == SDL_KEYUP && !end) {
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                player->accel.x -= .1;
                break;
            case SDLK_RIGHT:
                player->accel.x += .1;
                break;
            case SDLK_UP:
                player->accel.y += .1;
                break;
            case SDLK_DOWN:
                player->accel.y -= .1;
                break;
            case SDLK_SPACE:
                player->accel.y = 0;
                player->accel.x = 0;
                player->velocity.y *= .5;
                player->velocity.x *= .5;
                break;
            }
        }
    }
}

// tracks the player once the player
// is in the enemie's row or column
void trackAI() {
    if (round(enemyTracker->position.x) == round(player->position.x)) {
        if (round(enemyTracker->position.y) > round(player->position.y)) {
            enemyTracker->velocity.x = 0;
            enemyTracker->velocity.y = -0.5f;
        }
        else {
            enemyTracker->velocity.x = 0;
            enemyTracker->velocity.y = 0.5f;
        }
    }
    if (round(enemyTracker->position.y) == round(player->position.y)) {
        if (round(enemyTracker->position.x) > round(player->position.x)) {
            enemyTracker->velocity.y = 0;
            enemyTracker->velocity.x = -0.5;
        }
        else {
            enemyTracker->velocity.y = 0;
            enemyTracker->velocity.x = 0.5f;
        }
    }
}

// blocks the player from getting to the weapon
// and dashes at them if the player ends up
// in the same row as the enemy
void dashAI() {
    if (round(enemyBlockerDasher->position.y) == round(player->position.y)) {
        if (round(enemyBlockerDasher->position.x) > round(player->position.x)) {
            enemyBlockerDasher->velocity.x = -1.0f;
        }
        else {
            enemyBlockerDasher->velocity.x = 1.0f;
        }
    }
    else {
        if (round(enemyBlockerDasher->position.x) > round(player->position.x)) {
            enemyBlockerDasher->velocity.x = -0.5f;
        }
        else {
            enemyBlockerDasher->velocity.x = 0.5f;
        }
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
        
        player->Update(FIXED_TIMESTEP);
        enemyBounce->Update(FIXED_TIMESTEP);
        trackAI();
        enemyTracker->Update(FIXED_TIMESTEP);
        dashAI();
        enemyBlockerDasher->Update(FIXED_TIMESTEP);
        weapon->Update(FIXED_TIMESTEP);

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;

}

// Renders the walls, the goal area,
// the player, and the enemies
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < 34; i++)
        walls[i].Render(&program);


    player->Render(&program);
    enemyBounce->Render(&program);
    enemyTracker->Render(&program);
    enemyBlockerDasher->Render(&program);
    weapon->Render(&program);

    // displays win or loss
    // when the game ends
    if (end) {
        font = loadTexture("font1.png");
        if (win)
            DrawText(&program, font, "You Win", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
        else
            DrawText(&program, font, "Game Over", 0.5f, -0.25f, glm::vec3(-4.75f, 3.3f, 0.0f));
    }

    SDL_GL_SwapWindow(displayWindow);
}

// Runs the game until it ends
int main(int argc, char* argv[]) {
    Initialize();
    while (gameIsRunning) {
        if (!end) {
            ProcessInput();
            // detects whether the player
            // or enemies should bounce off the
            // walls
            for (int i = 0; i < 34; i++) {
                if (player->getCollision(&walls[i])) {
                    player->blocked = true;
                }
                if (enemyBounce->getCollision(&walls[i])) {
                    enemyBounce->blocked = true;
                }
                if (enemyBlockerDasher->getCollision(&walls[i])) {
                    enemyBlockerDasher->blocked = true;
                }
                if (enemyTracker->getCollision(&walls[i])) {
                    enemyTracker->blocked = true;
                }
            }
            // updates the state
            // of the game
            Update();
            // the player now
            // has the weapon
            if (player->getCollision(weapon)) {
                hasWeapon = true;
                weapon->isActive = false;
            }
            // Lose the game if colliding with an enemy without the weapon
            // and destroy an enemy if you have the weapon
            // destroy all enemies to win
            if (player->getCollision(enemyBounce) || player->getCollision(enemyBlockerDasher) || player->getCollision(enemyTracker)) {
                //end = true;
                if (!hasWeapon) {
                    end = true;
                    win = false;
                }
                else {
                    if (player->getCollision(enemyBounce)) {
                        enemyCount -= 1;
                        enemyBounce->isActive = false;
                    }
                    if (player->getCollision(enemyBlockerDasher)) {
                        enemyCount -= 1;
                        enemyBlockerDasher->isActive = false;
                    }
                    if (player->getCollision(enemyTracker)) {
                        enemyCount -= 1;
                        enemyTracker->isActive = false;
                    }
                    if (enemyCount == 0) {
                        end = true;
                        win = true;
                    }
                }
            }
            // renders the game
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