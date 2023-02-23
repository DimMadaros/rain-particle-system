#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

class Drawable;

struct Triangle {
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
    
    glm::vec3 n; // surface normal
    
    glm::vec3 c; // barycenter
};

/**
 * Represents the terrain
 */
class Terrain {
public:
    Drawable* terrain;
    float size;
    glm::mat4 modelMatrix;
    
    std::vector<glm::vec3> terrainVertices;
    std::vector<glm::vec3> terrainNormals;
    
    std::vector<Triangle> terrainTriangles;
    
    Terrain(float s);
    ~Terrain();

    void draw(unsigned int drawable = 0);
    void update(float t = 0, float dt = 0);
    void find_bounding_values(std::vector<glm::vec3> vertices);
    void find_terrain_triangles(std::vector<glm::vec3> vertices);
    
    float max_x;
    float max_y;
    float max_z;
    float min_x;
    float min_y;
    float min_z;
};
