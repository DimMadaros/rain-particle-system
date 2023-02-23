// Include C++ headers
#include <iostream>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/model.h>
#include <common/texture.h>

#include "headers/RainEmitter.h"
#include "headers/Terrain.h"

//TODO delete the includes afterwards
#include <chrono>
using namespace std::chrono;
//

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Rain"

// Global variables
GLFWwindow* window;
Camera* camera;
GLuint particleShaderProgram, terrainShaderProgram;
GLuint p_projectionMatrixLocation, p_viewMatrixLocation, p_modelMatrixLocation;
GLuint t_projectionMatrixLocation, t_viewMatrixLocation, t_modelMatrixLocation;
GLuint rainTexture, diffuseColorSampler, specularColorSampler, terrainTexture;

// light properties
GLuint t_LaLocation, t_LdLocation, t_LsLocation, t_lightPositionLocation, t_lightPowerLocation;
GLuint p_LaLocation, p_LdLocation, p_LsLocation, p_lightPositionLocation, p_lightPowerLocation;
// material properties
GLuint KdLocation, KsLocation, KaLocation, NsLocation;

Terrain* terrain;

struct Light {
    glm::vec4 La;
    glm::vec4 Ld;
    glm::vec4 Ls;
    glm::vec3 lightPosition_worldspace;
    float power;
};

struct Material {
    glm::vec4 Ka;
    glm::vec4 Kd;
    glm::vec4 Ks;
    float Ns;
};

const Material mtl{
    vec4{ 1.0, 1.0, 1.0, 1 },
    vec4{ 0.8, 0.8, 0.8, 1 },
    vec4{ 0.5, 0.5, 0.5, 1 },
    323.999994f
};

Light light{
    vec4{ 1, 1, 0, 1 },
    vec4{ 1, 1, 1, 1 },
    vec4{ 1, 1, 1, 1 },
    vec3{ 0, 0, 0 },
    400.0f
};

float wind_x = 0.0f;
float wind_z = 0.0f;
glm::vec3 slider_emitter_pos(0.0f, 40.0f, 0.0f);
int particles_slider = 1000;

void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);

bool game_paused = false;

bool use_sorting = false;
bool use_rotations = true;

float height_threshold = -10.0f;

glm::vec4 background_color = glm::vec4(0.2f, 0.4f, 0.6f, 0.0f);

void uploadMaterial(const Material& mtl) {
    glUniform4f(KaLocation, mtl.Ka.r, mtl.Ka.g, mtl.Ka.b, mtl.Ka.a);
    glUniform4f(KdLocation, mtl.Kd.r, mtl.Kd.g, mtl.Kd.b, mtl.Kd.a);
    glUniform4f(KsLocation, mtl.Ks.r, mtl.Ks.g, mtl.Ks.b, mtl.Ks.a);
    glUniform1f(NsLocation, mtl.Ns);
}

void uploadLight(const Light& light) {
    glUniform4f(t_LaLocation, light.La.r, light.La.g, light.La.b, light.La.a);
    glUniform4f(t_LdLocation, light.Ld.r, light.Ld.g, light.Ld.b, light.Ld.a);
    glUniform4f(t_LsLocation, light.Ls.r, light.Ls.g, light.Ls.b, light.Ls.a);
    glUniform3f(t_lightPositionLocation, light.lightPosition_worldspace.x,
                light.lightPosition_worldspace.y, light.lightPosition_worldspace.z);
    glUniform1f(t_lightPowerLocation, light.power);
}

void renderHelpingWindow() {
    static int counter = 0;

    ImGui::Begin("Helper Window");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

    ImGui::ColorEdit3("Background", &background_color[0]);

    ImGui::SliderFloat("x position", &slider_emitter_pos[0], -100.0f, 100.0f);
    ImGui::SliderFloat("y position", &slider_emitter_pos[1], -100.0f, 100.0f);
    ImGui::SliderFloat("z position", &slider_emitter_pos[2], -100.0f, 100.0f);
    ImGui::SliderInt("particles", &particles_slider, 0, 20000);


    if (ImGui::Button("+Wind_x"))
        wind_x++;
    ImGui::SameLine();
    if (ImGui::Button("-Wind_x"))
        wind_x--;
    ImGui::SameLine();
    if (ImGui::Button("+Wind_z"))
        wind_z++;
    ImGui::SameLine();
    if (ImGui::Button("-Wind_z"))
        wind_z--;

    ImGui::Checkbox("Use sorting", &use_sorting);
    ImGui::Checkbox("Use rotations", &use_rotations);

    ImGui::Text("Performance %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
 
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void createContext() {
    // Shaders
    particleShaderProgram = loadShaders(
        "shaders/ParticleShader.vertexshader",
        "shaders/ParticleShader.fragmentshader");

    terrainShaderProgram = loadShaders(
        "shaders/TerrainShader.vertexshader",
        "shaders/TerrainShader.fragmentshader");

    // Get pointers to uniforms
    p_modelMatrixLocation = glGetUniformLocation(particleShaderProgram, "M");
    p_viewMatrixLocation = glGetUniformLocation(particleShaderProgram, "V");
    p_projectionMatrixLocation = glGetUniformLocation(particleShaderProgram, "P");
    
    t_modelMatrixLocation = glGetUniformLocation(terrainShaderProgram, "M");
    t_viewMatrixLocation = glGetUniformLocation(terrainShaderProgram, "V");
    t_projectionMatrixLocation = glGetUniformLocation(terrainShaderProgram, "P");

    terrain = new Terrain(1);
    
    KaLocation = glGetUniformLocation(terrainShaderProgram, "mtl.Ka");
    KdLocation = glGetUniformLocation(terrainShaderProgram, "mtl.Kd");
    KsLocation = glGetUniformLocation(terrainShaderProgram, "mtl.Ks");
    NsLocation = glGetUniformLocation(terrainShaderProgram, "mtl.Ns");
    t_LaLocation = glGetUniformLocation(terrainShaderProgram, "light.La");
    t_LdLocation = glGetUniformLocation(terrainShaderProgram, "light.Ld");
    t_LsLocation = glGetUniformLocation(terrainShaderProgram, "light.Ls");
    t_lightPositionLocation = glGetUniformLocation(terrainShaderProgram, "light.lightPosition_worldspace");
    t_lightPowerLocation = glGetUniformLocation(terrainShaderProgram, "light.power");
    
    p_LaLocation = glGetUniformLocation(particleShaderProgram, "light.La");
    p_LdLocation = glGetUniformLocation(particleShaderProgram, "light.Ld");
    p_LsLocation = glGetUniformLocation(particleShaderProgram, "light.Ls");
    p_lightPositionLocation = glGetUniformLocation(particleShaderProgram, "light.lightPosition_worldspace");
    p_lightPowerLocation = glGetUniformLocation(particleShaderProgram, "light.power");


    diffuseColorSampler = glGetUniformLocation(particleShaderProgram, "diffuseColorSampler");
    specularColorSampler = glGetUniformLocation(particleShaderProgram, "specularColorSampler");
    rainTexture = loadSOIL("textures/reflection.png");
    terrainTexture = loadSOIL("textures/mirror.jpg");
    glfwSetKeyCallback(window, pollKeyboard);
}

void free() {
    glDeleteTextures(1, &rainTexture);
    glDeleteTextures(1, &terrainTexture);
    
    delete terrain;
    
    glDeleteProgram(particleShaderProgram);
    glDeleteProgram(terrainShaderProgram);
    glfwTerminate();
}

void mainLoop() {
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    camera->position = vec3(0, 10, 40);
    auto* droplet = new Drawable("models/droplet.obj");

    RainEmitter rain_emitter = RainEmitter(droplet,  particles_slider);

    float t = glfwGetTime();
    float step = 0;
    do {
        step += 0.01;
        
        rain_emitter.changeParticleNumber(particles_slider);
        rain_emitter.emitter_pos = slider_emitter_pos;
        rain_emitter.use_rotations = use_rotations;
        rain_emitter.use_sorting = use_sorting;
        rain_emitter.height_threshold = height_threshold;
        rain_emitter.wind_x = wind_x;
        rain_emitter.wind_z = wind_z;
        rain_emitter.terrain = terrain;

        float currentTime = glfwGetTime();
        //float dt = currentTime - t;
        float dt = 0.01f;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        light.lightPosition_worldspace = vec3(20*cos(step), 30, 20*sin(step));
        
        //*// terrain
        glUseProgram(terrainShaderProgram);
        
        light.lightPosition_worldspace = vec3(20*cos(step), 30, 20*sin(step));
        
        // light & material
        uploadLight(light);
        uploadMaterial(mtl);
        
        // camera
        camera->update();
        mat4 projectionMatrix = camera->projectionMatrix;
        mat4 viewMatrix = camera->viewMatrix;

        terrain->update(t, dt);

        glUniformMatrix4fv(t_viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(t_projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(t_modelMatrixLocation, 1, GL_FALSE, &terrain->modelMatrix[0][0]);
        
        terrain->draw();

        // Particles
        glUseProgram(particleShaderProgram);
        
        uploadLight(light);

        // camera
        camera->update();
        projectionMatrix = camera->projectionMatrix;
        viewMatrix = camera->viewMatrix;
        glm::mat4 modelMatrix = glm::mat4(1.0);

        glUniformMatrix4fv(p_viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(p_projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
        glUniformMatrix4fv(p_modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        

        //*/ Use particle based drawing
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rainTexture);
        glUniform1i(diffuseColorSampler, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, terrainTexture);
        glUniform1i(specularColorSampler, 1);
        
        if(!game_paused) {
            rain_emitter.updateParticles(currentTime, dt, camera->position);
        }
        rain_emitter.renderParticles(0);
        

        renderHelpingWindow();
        glfwPollEvents();
        glfwSwapBuffers(window);
        t = currentTime;

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
                            " If you have an Intel GPU, they are not 3.3 compatible." +
                            "Try the 2.1 version.\n"));
    }

    glfwMakeContextCurrent(window);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    //glFrontFace(GL_CCW);

    // enable point size when drawing points
    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Log
    logGLParameters();

    // Create camera
    camera = new Camera(window);

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
                                 camera->onMouseMove(xpos, ypos);
                             }
    );
}
void pollKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Task 2.1:
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        game_paused = !game_paused;
    }

    if (key == GLFW_KEY_GRAVE_ACCENT && action == GLFW_PRESS) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            camera->active = true;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera->active = false;
        }

    }
    
    if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        if (wind_x > -40.0f)
            wind_x -= 1.0f;
        else
            wind_x = -40.0f;
    }
    
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        if (wind_x < 40.0f)
            wind_x += 1.0f;
        else
            wind_x = 40.0f;
    }
    
    if (key == GLFW_KEY_I && action == GLFW_PRESS) {
        if (wind_z > -40.0f)
            wind_z -= 1.0f;
        else
            wind_z = -40.0f;
    }
    
    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        if (wind_z < 40.0f)
            wind_z += 1.0f;
        else
            wind_z = 40.0f;
    }

    
}

int main(void) {
    try {
        initialize();
        createContext();
        mainLoop();
        free();
    } catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }

    return 0;
}
