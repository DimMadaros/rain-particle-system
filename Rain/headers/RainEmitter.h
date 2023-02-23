#pragma once
#include "IntParticleEmitter.h"
#include "headers/Terrain.h"

class RainEmitter : public IntParticleEmitter {
public:

    RainEmitter(Drawable* model, int number);
    
    // Terrain for collision
    Terrain* terrain;
    
    // After this height particles die
    float height_threshold;
    
    // wind
    int wind_x = 0.0f;
    int wind_z = 0.0f;
    
    bool checkForCollision(particleAttributes& particle);
    bool handleRainTerrainCollision(particleAttributes& particle, Terrain& terrain);
    bool checkForRainTerrainCollision(particleAttributes& particle, Terrain& terrain, glm::vec3 &n);
    bool PointInTriangle(Triangle tri, glm::vec3 p);
    bool LineSegInTriangle(Triangle tri, LineSeg seg);

    int active_particles = 0;
    void createNewParticle(int index) override;
    void updateParticles(float time, float dt, glm::vec3 camera_pos = glm::vec3(0, 0, 0)) override;
};
