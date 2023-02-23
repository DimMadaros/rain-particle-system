#pragma once
#include "headers/RigidBody.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/model.h>
#include <glm/gtx/string_cast.hpp>
#include "iostream"
#include <algorithm>

//Gives a random number between 0 and 1
#define RAND ((float) rand()) / (float) RAND_MAX

// Standard acceleration due to gravity
#define G 9.80665f

struct LineSeg {
    glm::vec3 p1;
    glm::vec3 p2;
    
    
};

class particleAttributes : public RigidBody {
public:
    glm::vec3 rot_axis= glm::vec3(0,1,0);
    float rot_angle = 0.0f; //degrees
    glm::vec3 accel = glm::vec3(0,0,0);
    float life = 0.0f;
    float r = 0.0f; // radius
    float count = 0.0f; // number of collisions
    
    LineSeg axis;

    float dist_from_camera = 0.0f; //In case you want to do depth sorting
    bool operator < (const particleAttributes & p) const
    {
        return dist_from_camera < p.dist_from_camera;
    }
};


//ParticleEmitterInt is an interface class. Emitter classes must derive from this one and implement the updateParticles method
class IntParticleEmitter
{
public:
    GLuint emitterVAO;
    int number_of_particles;

    std::vector<particleAttributes> p_attributes;
    std::vector<int> counter;

    bool use_rotations = true;
    bool use_sorting = false;


    glm::vec3 emitter_pos; //the origin of the emitter

    IntParticleEmitter(Drawable* _model, int number);
	void changeParticleNumber(int new_number);

	void renderParticles(int time = 0);
	virtual void updateParticles(float time, float dt, glm::vec3 camera_pos) = 0;
	virtual void createNewParticle(int index) = 0;
    
    glm::vec4 calculateBillboardRotationMatrix(glm::vec3 particle_pos, glm::vec3 camera_pos);


private:

    std::vector<glm::mat4> translations;
    std::vector<glm::mat4> rotations;
    std::vector<float> scales;
    std::vector<float> lifes;

    Drawable* model;
    void configureVAO();
    void bindAndUpdateBuffers();
    GLuint translations_buffer;
    GLuint rotations_buffer;
    GLuint scales_buffer;
};
