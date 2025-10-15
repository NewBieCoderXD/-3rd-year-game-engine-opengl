#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

enum Axis { X, Y, Z };

const float big_g = 6.674 * std::pow(10, -11);

const float origin_mass = 5.972 * std::pow(10, 24);

const float SCALE = std::pow(10, 16);

const double TARGET_FPS = 60.0;
const double TARGET_FRAME_TIME_MS = 1000.0 / TARGET_FPS;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int GRID_X_COUNT = 5;
const int GRID_Y_COUNT = 5;
const int GRID_Z_COUNT = 5;
const int STEPS_PER_CURVE = 200;
const float X_MIN = -5.0f;
const float X_MAX = 5.0f;
const float Y_MIN = -5.0f;
const float Y_MAX = 5.0f;
const float Z_MIN = -5.0f;
const float Z_MAX = 5.0f;

float cameraRadius = 10.0f;
glm::quat cameraOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

bool mouseDragging = false;
double lastX = 0.0, lastY = 0.0;
const float ROT_SENSITIVITY = 0.005f;

GLuint gridVao = 0, gridVbo = 0;
GLuint sphereVao = 0, sphereVBO = 0, sphereEBO = 0;
GLuint shaderProgram = 0;

struct LineRange {
  GLsizei start;
  GLsizei count;
};
std::vector<LineRange> lineRanges;

struct Vertex {
  glm::vec3 pos;
};

template <typename T> int sign(T val) { return (T(0) < val) - (val < T(0)); }

glm::vec3 parametricFunction(float x, float y, float z, float time_diff,
                             float mass, Axis direction, glm::vec3 mass_pos) {
  glm::vec3 toward_mass = mass_pos - glm::vec3(x, y, z);
  float r = glm::length(toward_mass);
  float v = std::sqrt(2 * big_g * mass / r / SCALE);
  float dist_diff = v * time_diff / 1000.0;
  // std::cout << dist_diff << std::endl;
  glm::vec3 bent_pos = glm::vec3(x, y, z);
  bent_pos += toward_mass / r * dist_diff;
  return bent_pos;
}

bool out_of_range_vec3(glm::vec3 vec) {
  return vec.x > X_MAX || vec.x < X_MIN || vec.y > Y_MAX || vec.y < Y_MIN ||
         vec.z > Z_MAX || vec.z < Z_MIN;
}

std::vector<Vertex> generateGrid(float time_diff) {
  lineRanges.clear();
  std::vector<Vertex> verts;

  const float dx = (X_MAX - X_MIN) / float(GRID_X_COUNT - 1);
  const float dy = (Y_MAX - Y_MIN) / float(GRID_Y_COUNT - 1);
  const float dz = (Z_MAX - Z_MIN) / float(GRID_Z_COUNT - 1);

  for (int iz = 0; iz < GRID_Z_COUNT; ++iz) {
    float z = Y_MIN + iz * dz;
    for (int ix = 0; ix < GRID_X_COUNT; ++ix) {
      float x = X_MIN + ix * dx;
      GLsizei start = (GLsizei)verts.size();
      for (int s = 0; s < STEPS_PER_CURVE; ++s) {
        float y =
            Y_MIN + (Y_MAX - Y_MIN) * (float(s) / float(STEPS_PER_CURVE - 1));
        glm::vec3 p =
            parametricFunction(x, y, z, time_diff, origin_mass, Y, glm::vec3());

        if (out_of_range_vec3(p)) {
          continue;
        }
        verts.push_back({p});
      }
      GLsizei count = (GLsizei)verts.size() - start;
      lineRanges.push_back({start, count});
    }
  }

  for (int iz = 0; iz < GRID_Z_COUNT; ++iz) {
    float z = Z_MIN + iz * dz;
    for (int iy = 0; iy < GRID_Y_COUNT; ++iy) {
      float y = Y_MIN + iy * dy;
      GLsizei start = (GLsizei)verts.size();
      for (int s = 0; s < STEPS_PER_CURVE; ++s) {
        float x =
            X_MIN + (X_MAX - X_MIN) * (float(s) / float(STEPS_PER_CURVE - 1));
        glm::vec3 p =
            parametricFunction(x, y, z, time_diff, origin_mass, X, glm::vec3());

        if (out_of_range_vec3(p)) {
          continue;
        }
        verts.push_back({p});
      }
      GLsizei count = (GLsizei)verts.size() - start;
      lineRanges.push_back({start, count});
    }
  }

  for (int ix = 0; ix < GRID_X_COUNT; ++ix) {
    float x = X_MIN + ix * dx;
    for (int iy = 0; iy < GRID_Y_COUNT; ++iy) {
      float y = Y_MIN + iy * dy;
      GLsizei start = (GLsizei)verts.size();
      for (int s = 0; s < STEPS_PER_CURVE; ++s) {
        float z =
            Z_MIN + (Z_MAX - Z_MIN) * (float(s) / float(STEPS_PER_CURVE - 1));
        glm::vec3 p =
            parametricFunction(x, y, z, time_diff, origin_mass, Z, glm::vec3());

        if (out_of_range_vec3(p)) {
          continue;
        }
        verts.push_back({p});
      }
      GLsizei count = (GLsizei)verts.size() - start;
      lineRanges.push_back({start, count});
    }
  }

  return verts;
}

const char *vertexShaderSrc = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main(){
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)GLSL";

const char *fragmentShaderSrc = R"GLSL(
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main(){
    FragColor = vec4(uColor, 1.0);
}
)GLSL";

GLuint compileShader(GLenum type, const char *src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, nullptr);
  glCompileShader(s);
  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char buf[1024];
    glGetShaderInfoLog(s, 1024, nullptr, buf);
    std::cerr << "Shader compile error: " << buf << std::endl;
  }
  return s;
}

GLuint createProgram() {
  GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
  GLuint p = glCreateProgram();
  glAttachShader(p, vs);
  glAttachShader(p, fs);
  glLinkProgram(p);
  GLint ok = 0;
  glGetProgramiv(p, GL_LINK_STATUS, &ok);
  if (!ok) {
    char buf[1024];
    glGetProgramInfoLog(p, 1024, nullptr, buf);
    std::cerr << "Program link error: " << buf << std::endl;
  }
  glDeleteShader(vs);
  glDeleteShader(fs);
  return p;
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      mouseDragging = true;
      glfwGetCursorPos(window, &lastX, &lastY);
    } else if (action == GLFW_RELEASE) {
      mouseDragging = false;
    }
  }
}

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
  if (!mouseDragging)
    return;
  double dx = xpos - lastX;
  double dy = ypos - lastY;
  lastX = xpos;
  lastY = ypos;

  float yawAngle = -float(dx) * ROT_SENSITIVITY;
  glm::quat yaw = glm::angleAxis(yawAngle, glm::vec3(0.0f, 1.0f, 0.0f));

  glm::vec3 right =
      glm::normalize(cameraOrientation * glm::vec3(1.0f, 0.0f, 0.0f));
  float pitchAngle = -float(dy) * ROT_SENSITIVITY;
  glm::quat pitch = glm::angleAxis(pitchAngle, right);

  cameraOrientation = glm::normalize(yaw * pitch * cameraOrientation);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {}

std::pair<std::vector<Vertex>, std::vector<unsigned int>>
generateSphere(glm::vec3 center_pos, float radius) {
  int latSteps = 20;
  int lonSteps = 20;
  std::vector<Vertex> verts;

  for (int i = 0; i <= latSteps; ++i) {
    float phi = M_PI * float(i) / float(latSteps); // 0 to pi
    for (int j = 0; j <= lonSteps; ++j) {
      float theta = 2.0f * M_PI * float(j) / float(lonSteps); // 0 to 2pi
      Vertex v;
      v.pos.x = center_pos.x + radius * sin(phi) * cos(theta);
      v.pos.y = center_pos.y + radius * cos(phi);
      v.pos.z = center_pos.z + radius * sin(phi) * sin(theta);
      verts.push_back(v);
    }
  }

  std::vector<unsigned int> indices;

  for (int i = 0; i < latSteps; ++i) {
    for (int j = 0; j < lonSteps; ++j) {
      int first = i * (lonSteps + 1) + j;
      int second = first + lonSteps + 1;
      // two triangles per quad
      indices.push_back(first);
      indices.push_back(second);
      indices.push_back(first + 1);

      indices.push_back(second);
      indices.push_back(second + 1);
      indices.push_back(first + 1);
    }
  }
  return {verts, indices};
}

void processInput(GLFWwindow *window);

int main() {
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(
      WINDOW_WIDTH, WINDOW_HEIGHT, "Parametric Grid - Quaternion Orbit Camera",
      nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
  glLineWidth(1.0f);

  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);

  shaderProgram = createProgram();

  auto start_time = std::chrono::high_resolution_clock::now();

  // Grid
  std::vector<Vertex> gridVerts = generateGrid(0);

  glGenVertexArrays(1, &gridVao);
  glGenBuffers(1, &gridVbo);

  glBindVertexArray(gridVao);
  glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
  glBufferData(GL_ARRAY_BUFFER, gridVerts.size() * sizeof(Vertex),
               gridVerts.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  // glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Sphere
  auto spherePair = generateSphere(glm::vec3(), 3.0);
  std::vector<Vertex> sphereVerts = spherePair.first;
  std::vector<unsigned int> sphereIndices = spherePair.second;

  glGenVertexArrays(1, &sphereVao);
  glGenBuffers(1, &sphereVBO);
  glGenBuffers(1, &sphereEBO);

  glBindVertexArray(sphereVao);

  // Vertex
  glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
  glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(Vertex),
               sphereVerts.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  // Indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sphereIndices.size() * sizeof(unsigned int),
               sphereIndices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);

  glDisable(GL_CULL_FACE);

  cameraOrientation = glm::normalize(
      glm::quat(glm::vec3(glm::radians(-20.0f), glm::radians(30.0f), 0.0f)));

  auto lastFrameTime = std::chrono::high_resolution_clock::now();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    processInput(window);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);

    glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::vec3 camLocal = glm::vec3(0.0f, 0.0f, cameraRadius);
    glm::vec3 camPos = cameraOrientation * camLocal;
    glm::vec3 upDir =
        glm::normalize(cameraOrientation * glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f), upDir);
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), float(w) / float(h),
                                      0.1f, 200.0f);
    glm::mat4 mvp = proj * view * glm::mat4(1.0f);

    glUseProgram(shaderProgram);
    GLint locMVP = glGetUniformLocation(shaderProgram, "uMVP");
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(mvp));
    GLint locColor = glGetUniformLocation(shaderProgram, "uColor");
    glUniform3f(locColor, 0.8f, 0.8f, 0.8f);

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration<double, std::milli>(end_time - start_time)
            .count();

    gridVerts = generateGrid(fmod(duration, 1000000));

    // generate_mass(glm::vec3(), 3.0);

    glBindVertexArray(gridVao);
    glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
    glBufferData(GL_ARRAY_BUFFER, gridVerts.size() * sizeof(Vertex),
                 gridVerts.data(), GL_DYNAMIC_DRAW);
    for (size_t i = 0; i < lineRanges.size(); ++i) {
      const LineRange &lr = lineRanges[i];

      glUniform3f(locColor, 0.75f, 0.75f, 0.95f);

      // if (i < size_t(GRID_X_COUNT)) {
      //   glUniform3f(locColor, 0.75f, 0.75f, 0.95f);
      // } else {
      //   glUniform3f(locColor, 0.75f, 0.95f, 0.75f);
      // }
      glDrawArrays(GL_LINE_STRIP, lr.start, lr.count);
    }
    glBindVertexArray(0);

    glBindVertexArray(sphereVao);
    glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Limit frame rate
    auto currentFrameTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> deltaTime =
        currentFrameTime - lastFrameTime;
    lastFrameTime = currentFrameTime;
    if (deltaTime.count() < TARGET_FRAME_TIME_MS) {
      std::chrono::duration<double, std::milli> sleepTime(TARGET_FRAME_TIME_MS -
                                                          deltaTime.count());
      std::this_thread::sleep_for(sleepTime);
    }

    glfwSwapBuffers(window);
  }

  glDeleteBuffers(1, &gridVbo);
  glDeleteVertexArrays(1, &gridVao);
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}