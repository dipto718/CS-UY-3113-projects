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
#include "map.h"

// acts as the header file
// for the various parts of
// the Entity class
class Entity {
public:
    glm::vec3 position = glm::vec3(0);
    glm::vec3 velocity = glm::vec3(0);
    glm::vec3 accel = glm::vec3(0);
    float w = 1.0f;
    float h = 1.0f;
    bool isActive = true;
    bool blocked = false;

    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;


    GLuint textureID;

    glm::mat4 modelMatrix;
    
    Entity();

    void Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Map* map);
    void Render(ShaderProgram* program);
    bool getCollision(Entity* object);
    void CheckCollisionsX(Map* map);
    void CheckCollisionsY(Map* map);
    void CheckCollisionsY(Entity* objects, int objectCount);
    void CheckCollisionsX(Entity* objects, int objectCount);
};
