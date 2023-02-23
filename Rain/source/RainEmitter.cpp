#include "headers/RainEmitter.h"

RainEmitter::RainEmitter(Drawable* model, int number) : IntParticleEmitter(model, number) {}

void RainEmitter::updateParticles(float time, float dt, glm::vec3 camera_pos) {

    //This is for the fountain to slowly increase the number of its particles to the max amount
    //instead of shooting all the particles at once
    if (active_particles < number_of_particles) {
        int batch = 1;
        int limit = std::min(number_of_particles - active_particles, batch);
        for (int i = 0; i < limit; i++) {
            createNewParticle(active_particles);
            active_particles++;
        }
    }
    else {
        active_particles = number_of_particles; //In case we resized our ermitter to a smaller particle number
    }

    for(int i = 0; i < active_particles; i++){
        particleAttributes &particle = p_attributes[i];
        
        
        if(particle.x.y < height_threshold || particle.life == 0.0f){
            createNewParticle(i);
        }
        
        if (particle.rot_angle == 0)
        {
            particle.axis.p1 = particle.x + glm::vec3(0.0f, particle.r + 2.5f, 0.0f);
            particle.axis.p2 = particle.x - glm::vec3(0.0f, particle.r + 2.5f, 0.0f);
        }
        else
        {
            particle.axis.p1 = particle.x + particle.r + 2.5f;
            particle.axis.p2 = particle.x - particle.r + 2.5f;
        }
        
        if(handleRainTerrainCollision(particle, *terrain))
        {
            particle.count++;
            
            if (particle.count >= 2)
            {
                createNewParticle(i);
            }
        }
        
        if (particle.count == 1)
        {
            if (terrain->max_x < particle.x.x || terrain->min_x > particle.x.x) createNewParticle(i);
            if (terrain->max_z < particle.x.z || terrain->min_z > particle.x.z) createNewParticle(i);
        }
        
        particle.forcing = [&](float t, const std::vector<float>& y)->std::vector<float> {
            std::vector<float> f(6, 0.0f);
            f[1] = -particle.m * G;
            return f;
        };
        
        // integration
        particle.advanceState(time, dt);
    }
}

bool RainEmitter::checkForCollision(particleAttributes &particle)
{
    return particle.x.y < -80.0f;
}

bool RainEmitter::checkForRainTerrainCollision(particleAttributes &particle, Terrain &terrain, glm::vec3 &n)
{
    std::vector<Triangle> triangles = terrain.terrainTriangles;
    
    if (terrain.max_x < particle.x.x || terrain.min_x > particle.x.x) return false;
    if (terrain.max_z < particle.x.z || terrain.min_z > particle.x.z) return false;
    if (terrain.max_y < particle.x.y) return false;
    
    for (int i = 0; i < triangles.size(); i++)
    {
//        if (PointInTriangle(triangles[i], particle.x))
//        {
//            if (particle.x.y - particle.r <= triangles[i].c.y)
//            {
//                float dis = -(particle.x.y - particle.r - triangles[i].c.y);
//                particle.x = particle.x + glm::vec3(0, dis, 0);
//                //n = -triangles[i].n;
//                n = glm::vec3(0, -1, 0);
//
//                return true;
//            }
//        }
//
        if (LineSegInTriangle(triangles[i], particle.axis) == 1)
        {
            if (particle.x.y - particle.r < triangles[i].c.y)
            {
                float dis = -(particle.x.y - particle.r - triangles[i].c.y);
                particle.x = particle.x + glm::vec3(0, dis, 0);
                //n = triangles[i].n;
                n = glm::vec3(0, -1, 0);
                
                return true;
            }
        }
    }
    
    return false;
}

bool RainEmitter::handleRainTerrainCollision(particleAttributes &particle, Terrain &terrain)
{
    glm::vec3 n;
    if (checkForRainTerrainCollision(particle, terrain, n)) {
        // Velocity of the sphere after the collision
        particle.P = particle.P - 1.04f * dot(particle.P, n) * n;
        return true;
    }
    return false;
}

void RainEmitter::createNewParticle(int index){
    particleAttributes & particle = p_attributes[index];

    particle.r = 0.5;
    particle.m = 1;
    particle.x = emitter_pos + glm::vec3(30 - RAND * 60, 0, 30 - RAND * 60);
    particle.v = glm::vec3( 2 - 3 * RAND + wind_x * 0.1, -10, 2 - 3 * RAND + wind_z * 0.1) * 8.0f;
    particle.P = particle.m * particle.v;
    
    glm::mat3 I = glm::mat3(
            2.0f / 5 * particle.m * particle.r * particle.r, 0, 0,
            0, 2.0f / 5 * particle.m * particle.r * particle.r, 0,
            0, 0, 2.0f / 5 * particle.m * particle.r * particle.r);
    
    particle.I_inv = glm::inverse(I);
    
    particle.rot_axis = glm::normalize(glm::vec3(1, 0, 1));
    particle.rot_angle = wind_x + wind_z;
    
    particle.life = 1.0f; //mark it alive
    
    if (particle.rot_angle == 0)
    {
        particle.axis.p1 = particle.x + glm::vec3(0.0f, particle.r + 5.0f, 0.0f);
        particle.axis.p2 = particle.x - glm::vec3(0.0f, particle.r + 5.0f, 0.0f);
    }
    else
    {
        particle.axis.p1 = particle.x + particle.r + 2.5f;
        particle.axis.p2 = particle.x - particle.r + 2.5f;
    }
}

// Check if point belongs to triangle with barycentric coordinates
bool RainEmitter::PointInTriangle(Triangle tri, glm::vec3 p)
{
    glm::vec3 a = tri.p1;
    glm::vec3 b = tri.p2;
    glm::vec3 c = tri.p3;
    
    float u, v, w; // Barycentric coordinates
    
    glm::vec3 v0 = b - a;
    glm::vec3 v1 = c - a;
    glm::vec3 v2 = p - a;
    
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    
    // Cramer
    float denom = d00 * d11 - d01 * d01;

    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;

    if (v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f)
        return true;
    else
        return false;
}

bool RainEmitter::LineSegInTriangle(Triangle tri, LineSeg seg)
{
    float u, v, w; // Barycentric coordinates
    
    glm::vec3 p = seg.p1;
    glm::vec3 q = seg.p2;
    
    glm::vec3 a = tri.p1;
    glm::vec3 b = tri.p2;
    glm::vec3 c = tri.p3;
    
    glm::vec3 pq = q - p;
    glm::vec3 pa = a - p;
    glm::vec3 pb = b - p;
    glm::vec3 pc = c - p;
    
    glm::vec3 m = cross(pq, pc);
    
    u = dot(pb, m); // ScalarTriple(pq, pc, pb);
    if (u < 0.0f) return 0;
    
    v = -dot(pa, m); // ScalarTriple(pq, pa, pc);
    if (v < 0.0f) return 0;
    
    w = dot(cross(pq, pb), pa); // ScalarTriple(pq, pb, pa);
    if (w < 0.0f) return 0;
    
    if (u == v == w == 0) return 2; // The segment is parallel to the triangle's plane
    
    return 1;
}
