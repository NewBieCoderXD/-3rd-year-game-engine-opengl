#include <cmath>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_s.h>

#include <iostream>

#include <vector>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const float PIXEL_SIZE = 30;

int windowWidth = SCR_WIDTH;
int windowHeight = SCR_HEIGHT;

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
  Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

  unsigned int VBO, VAO, EBO, instanceVBO;
  ;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenBuffers(1, &instanceVBO);

  float pixelHeight = PIXEL_SIZE / (float)windowHeight;

  float pixelWidth = PIXEL_SIZE / (float)windowWidth;

  std::vector<float> vertices;
  vertices.push_back(-pixelWidth / 2.0);
  vertices.push_back(0);

  vertices.push_back(0);
  vertices.push_back(pixelHeight * std::sqrt(3) / 2);

  vertices.push_back(pixelWidth / 2.0);
  vertices.push_back(0);

  std::vector<unsigned int> indices = {0, 1, 2};

  std::vector<glm::mat4> transforms;

  for (int i = 0; i < windowWidth; i += 50) {
    for (int j = 0; j < windowHeight; j += 50) {
      float x = (i / (float)windowWidth) * 2.0f - 1.0f;
      float y = (j / (float)windowHeight) * 2.0f - 1.0f;

      transforms.push_back(glm::mat4(1.0f));
    }
  }

  // render container
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int),
               indices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4),
               transforms.data(), GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  for (unsigned int i = 0; i < 4; i++) {
    glEnableVertexAttribArray(2 + i);                     // locations 2,3,4,5
    glVertexAttribPointer(2 + i,                          // attribute location
                          4,                              // size: vec4
                          GL_FLOAT,                       // type
                          GL_FALSE,                       // normalized
                          sizeof(glm::mat4),              // stride
                          (void *)(i * sizeof(glm::vec4)) // offset
    );
    glVertexAttribDivisor(2 + i, 1); // 1 per instance
  }
  glBindVertexArray(0);

  // tell opengl for each sampler to which texture unit it belongs to (only has
  // to be done once)
  // -------------------------------------------------------------------------------------------
  ourShader.use();
  // ourShader.setInt("texture1", 0);
  // ourShader.setInt("texture2", 1);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0,
                            transforms.size());

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

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