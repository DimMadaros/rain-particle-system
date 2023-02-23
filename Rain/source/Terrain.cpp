#include "headers/Terrain.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/model.h>

using namespace glm;

Terrain::Terrain(float s) {
    size = s;
    
    terrain = new Drawable("models/terrain.obj");
    
    terrainVertices = terrain->indexedVertices;
    terrainNormals = terrain->indexedNormals;
    
    find_bounding_values(terrainVertices);
    find_terrain_triangles(terrainVertices);
}

Terrain::~Terrain() {
    delete terrain;
}

void Terrain::draw(unsigned int drawable) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    terrain->bind();
    terrain->draw();
    glEnable(GL_CULL_FACE);
}

void Terrain::update(float t, float dt) {
    mat4 translate = glm::translate(mat4(), vec3(0.0f, 0.0f, 0.0f));
    mat4 scale = glm::scale(mat4(), vec3(size, size, size));
    mat4 rotate = glm::rotate(mat4(1), 0.0f, vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translate * scale * rotate;
}

void Terrain::find_bounding_values(std::vector<glm::vec3> vertices)
{
    max_x = vertices[0].x;
    max_y = vertices[0].y;
    max_z = vertices[0].z;
    min_x = vertices[0].x;
    min_y = vertices[0].y;
    min_z = vertices[0].z;
    
    for (int i = 0; i < vertices.size(); i++)
    {
        if (vertices[i].x > max_x) max_x = vertices[i].x;
        if (vertices[i].y > max_y) max_y = vertices[i].y;
        if (vertices[i].z > max_z) max_z = vertices[i].z;

        if (vertices[i].x < min_x) min_x = vertices[i].x;
        if (vertices[i].y < min_y) min_y = vertices[i].y;
        if (vertices[i].z < min_z) min_z = vertices[i].z;
    }
}

void Terrain::find_terrain_triangles(std::vector<glm::vec3> vertices)
{
    Triangle tri;
    
    for (int i = 0; i < vertices.size() - 2; i+=3)
    {
        tri.p1 = vertices[i];
        tri.p2 = vertices[i + 1];
        tri.p3 = vertices[i + 2];
        
        // calculate normal
        vec3 e1 = tri.p1 - tri.p2;
        vec3 e2 = tri.p2 - tri.p1;
        
        tri.n = cross(e1, e2);
        
        // calculate barycenter
        tri.c.x = (tri.p1.x + tri.p2.x + tri.p3.x) / 3.0f;
        tri.c.y = (tri.p1.y + tri.p2.y + tri.p3.y) / 3.0f;
        tri.c.z = (tri.p1.z + tri.p2.z + tri.p3.z) / 3.0f;
        
        terrainTriangles.push_back(tri);
    }
    std::cout << terrainTriangles.size() << std::endl;
}
