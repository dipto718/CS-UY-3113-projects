#include "Entity.h"

// creates a new Entity
Entity::Entity()
{
}

// moves the entity according to
// how much acceleration it has
void Entity::Update(float time)
{
    if (!isActive)
        return;
    // moves normally if it hasn't 
    // collided with a wall
    if (!blocked) {
        // gets the velocity
        velocity += accel * time;
        // gets the position that results from the velocity
        position += velocity * time;
        // Sets the model matrix and moves the Entity
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
    }
    // bouces if it collided with a wall
    else {
        blocked = false;
        // gets the velocity
        velocity += accel * time;
        velocity *= -1;
        accel *= 0;
        // gets the position that results from the velocity
        position += velocity * time;
        // Sets the model matrix and moves the Entity
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
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