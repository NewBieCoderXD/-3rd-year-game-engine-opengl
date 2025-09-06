#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <cmath>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>

#include <chrono>
#include <iostream>
#include <vector>

// Interactive
// Key up -> increase triangle size
// Key down -> decrease triangle size

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

float triangleSize = 30;

const float TRI_DISTANCE_PERC = 0.7;

int windowWidth = SCR_WIDTH;
int windowHeight = SCR_HEIGHT;

bool is_idle = false;

double mouseX = 0;
double mouseY = 0;

const char *vexterShaderSource = R"(#version 330 core
layout (location = 0) in vec2 aPos;

out vec2 fragPos;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 0.0, 1.0);
})";

const char *fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;

uniform vec2 fragPos;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    float hue = atan(fragPos.y-1, fragPos.x-1) / (2.0 * 3.1415926) + 0.5;
    float saturation = 1.0;
    float value = 1.0;

    vec3 color = hsv2rgb(vec3(hue, saturation, value));
    FragColor = vec4(color, 1.0);
})";

std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

void mouse_callback(GLFWwindow *window, double x, double y) {
  mouseX = x / (double)windowWidth * 2.0 - 1.0;
  mouseY = -y / (double)windowHeight * 2.0 + 1.0;
  is_idle = false;
}

GLuint compileShader(GLenum type, const char *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info[512];
    glGetShaderInfoLog(shader, 512, nullptr, info);
    std::cerr << "Shader compilation failed:\n" << info << std::endl;
  }
  return shader;
}

int main() {
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // build and compile our shader zprogram
  // ------------------------------------
  // Shader ourShader("5.1.transform.vs", "5.1.transform.fs");
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vexterShaderSource);
  GLuint fragmentShader =
      compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

  GLuint ourShader = glCreateProgram();
  glAttachShader(ourShader, vertexShader);
  glAttachShader(ourShader, fragmentShader);
  glLinkProgram(ourShader);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  unsigned int VBO, VAO;

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  float pixelHeight = triangleSize / (float)windowHeight;

  float pixelWidth = triangleSize / (float)windowWidth;

  std::vector<float> vertices;
  vertices.push_back(0);
  vertices.push_back(0);

  vertices.push_back(0);
  vertices.push_back(pixelHeight * std::sqrt(3) / 2);

  vertices.push_back(pixelWidth / 2.0);
  vertices.push_back(0);

  // render container
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  // position attribute (2 floats per vertex)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  // tell opengl for each sampler to which texture unit it belongs to (only has
  // to be done once)
  // -------------------------------------------------------------------------------------------
  // ourShader.setInt("texture1", 0);
  // ourShader.setInt("texture2", 1);

  GLuint modelLoc = glGetUniformLocation(ourShader, "model");
  glm::mat4 modelMatrix = glm::mat4(1.0f);
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

  GLint fragPosLoc = glGetUniformLocation(ourShader, "fragPos");

  glfwSetCursorPosCallback(window, mouse_callback);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    if (is_idle) {
      processInput(window);
    }

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(ourShader);
    glBindVertexArray(VAO);

    for (int i = 0; i < windowWidth; i += TRI_DISTANCE_PERC * triangleSize) {
      for (int j = 0; j < windowHeight; j += TRI_DISTANCE_PERC * triangleSize) {
        double positionX = (double)i / windowWidth * 2.0 - 1.0;
        double positionY = (double)j / windowHeight * 2.0 - 1.0;

        float radius =
            sqrt(pow(mouseX - positionX, 2) + pow(mouseY - positionY, 2));

        float angleToMouse = atan2(mouseY - positionY, mouseX - positionX);

        std::chrono::steady_clock::time_point end =
            std::chrono::steady_clock::now();

        long dt =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                .count();

        float y = angleToMouse - cos(3 * radius + dt * 0.005) * M_PI +
                  M_PI * cos(abs(positionX) + abs(positionY) + log(dt) * 0.01);
        // float y = angle;
        // float y = angleToMouse+sin(radius*5) * M_PI;

        glm::mat4 rotated = glm::rotate(
            glm::translate(glm::mat4(1.0f), glm::vec3(positionX, positionY, 0)),
            y, glm::vec3(0, 0, 1));

        double scale = 1.0 + 0.7 * exp(-radius);

        modelMatrix = glm::scale(rotated, glm::vec3(scale));
        // modelMatrix = rotated;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glUniform2f(fragPosLoc, positionX, positionY);

        glDrawArrays(GL_TRIANGLES, 0, 3);
      }
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();

    is_idle = true;
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    triangleSize += 0.1;
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    triangleSize -= 0.1;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  windowWidth = width;
  windowHeight = height;
  glViewport(0, 0, width, height);
}