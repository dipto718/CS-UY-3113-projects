#include "Entity.h"


// creates a new Entity
Entity::Entity()
{
}

// moves the entity according to
// how much velocity it has
void Entity::Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Map* map)
{
    if (isActive == false)
        return;
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    velocity += accel * deltaTime;

    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(map);
    CheckCollisionsY(objects, objectCount); // Fix if needed

    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(map);
    CheckCollisionsX(objects, objectCount); // Fix if needed


    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

// checks whether it has collided with another
// object along the y-axis
void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (getCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (h / 2.0f) - (object->h / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
            }
        }
    }
}

// checks whether it has collided with another
// object along the x-axis
void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (getCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (w / 2.0f) - (object->w / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
            }
        }
    }
}

// checks whether it has collided with the
// terrain along the y-axis
void Entity::CheckCollisionsY(Map* map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (h / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (w / 2), position.y + (h / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (w / 2), position.y + (h / 2), position.z);

    glm::vec3 bottom = glm::vec3(position.x, position.y - (h / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (w / 2), position.y - (h / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (w / 2), position.y - (h / 2), position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(top, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    else if (map->IsSolid(top_left, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    else if (map->IsSolid(top_right, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
    }
    if (map->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
    else if (map->IsSolid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
    else if (map->IsSolid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
    }
}

// checks whether it has collided with the
// terrain along the x-axis
void Entity::CheckCollisionsX(Map* map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (w / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (w / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
        position.x += penetration_x;
        velocity.x = 0;
        collidedLeft = true;
    }

    if (map->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
        position.x -= penetration_x;
        velocity.x = 0;
        collidedRight = true;
    }
}

// Creates the textured object
void Entity::Render(ShaderProgram* program) {
    if (!isActive)
        return;
    // sets the model matrix
    program->SetModelMatrix(modelMatrix);

    // The positions of the two triangles the texture
    // is written on
    float trianglePos[] = { -.5, -.5, .5, -.5, .5, .5, -.5, -.5, .5, .5, -.5, .5 };

    // The positions of the box formed by
    // the two triangle
    float texturePos[] = { 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0 };

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, trianglePos);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texturePos);
    glEnableVertexAttribArray(program->texCoordAttribute);

    // Binds the texture to the two triangles
    // and draws them
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

// returns whether a collision happened between the two entities
bool Entity::getCollision(Entity* object) {
    if (!isActive || !(object->isActive))
        return false;
    float xDist = fabs(position.x - object->position.x) - ((w + object->w) / 2.0f);
    float yDist = fabs(position.y - object->position.y) - ((h + object->h) / 2.0f);
    return xDist < 0 && yDist < 0;
}