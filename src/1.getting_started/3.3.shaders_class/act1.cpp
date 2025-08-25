#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <learnopengl/shader_s.h>

#include <cmath>
#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

int add_triangle(float leftmost_x, float leftmost_y, float width,
                 std::vector<float> &array_of_triangles) {
  if (leftmost_y >= 1.0f || leftmost_x + width <= -1.0 || leftmost_x >= 1.0 ||
      leftmost_y + static_cast<float>(width / 2.0f * sqrt(3.0)) <= -1.0) {
    // std::cout << "x: " << leftmost_x << ", y: " << leftmost_y << "\n";
    return 0;
  }
  if (width <= 0.01) {
    // std::cout << "leftmost: " << leftmost_x << ", " << leftmost_y
    //           << " width: " << width << "\n";
    array_of_triangles.push_back(leftmost_x);
    array_of_triangles.push_back(leftmost_y);
    array_of_triangles.push_back(0.0f);
    array_of_triangles.push_back(leftmost_x + width / 2.0f);
    array_of_triangles.push_back(leftmost_y +
                                 static_cast<float>(width / 2.0f * sqrt(3.0)));
    array_of_triangles.push_back(0.0f);
    array_of_triangles.push_back(leftmost_x + width);
    array_of_triangles.push_back(leftmost_y);
    array_of_triangles.push_back(0.0f);
    return 1;
  }
  return add_triangle(leftmost_x, leftmost_y, width / 2.0, array_of_triangles) +
         add_triangle(leftmost_x + width / 4.0f,
                      leftmost_y + static_cast<float>(width / 4.0 * sqrt(3.0)),
                      width / 2.0, array_of_triangles) +
         add_triangle(leftmost_x + width / 2.0f, leftmost_y, width / 2.0,
                      array_of_triangles);
}

bool is_idle = false;

float x = 0;
float y = 0;

float width = 0.70;

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

  // build and compile our shader program
  // ------------------------------------
  Shader ourShader(
      "3.3.shader.vs",
      "3.3.shader.fs"); // you can name your shader files however you like

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);

  std::vector<float> vertices = {};

  // vertices = {};

  // vertices.push_back(-1);
  // vertices.push_back(-1);
  // vertices.push_back(0.0f);
  // vertices.push_back(0);
  // vertices.push_back(1);
  // vertices.push_back(0.0f);
  // vertices.push_back(1);
  // vertices.push_back(-1);
  // vertices.push_back(0.0f);

  // float vertices[] = {
  //     // positions         // colors
  //      0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
  //     -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
  //      0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top
  // };

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // color attribute
  // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
  //                       (void *)(3 * sizeof(float)));
  // glEnableVertexAttribArray(1);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally
  // modify this VAO, but this rarely happens. Modifying other VAOs requires a
  // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
  // VBOs) when it's not directly necessary.
  glBindVertexArray(0);

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    if (is_idle) {
      // input
      // -----
      processInput(window);
      // glfw: swap buffers and poll IO events (keys pressed/released, mouse
      // moved etc.)
      // -------------------------------------------------------------------------------
      glfwPollEvents();
      continue;
    }

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // render the triangle
    std::cout << "loop" << "\n";
    vertices = {};
    int triangles_count =
        add_triangle(-width / 2.0f + x * width,
                     static_cast<float>(-width / 2.0f / sqrt(3.0f)) + y * width,
                     width, vertices);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    std::cout << "number of triangles: " << triangles_count << "\n";

    ourShader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glfwSwapBuffers(window);

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

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    width += 0.05 * width;
    is_idle = false;
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    width -= 0.05 * width;
    is_idle = false;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    y -= 0.01 / width;
    is_idle = false;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    y += 0.01 / width;
    is_idle = false;
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    x -= 0.01 / width;
    is_idle = false;
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    x += 0.01 / width;
    is_idle = false;
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}
