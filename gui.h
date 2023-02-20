#include <GL/glew.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

extern float a_slider_value;
extern float slider_2;
extern float slider_3;
extern float x_size;
extern float y_size;
extern float z_size;

void glfw_error_callback(int error, const char* description);

GLFWwindow* InitGLFW();

void InitUI(GLFWwindow* window, const char* glsl_version);
void DestroyUI();
void RenderUI();