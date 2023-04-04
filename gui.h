#include <GL/glew.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "lib/ImGuiFileDialog/ImGuiFileDialog.h"

extern bool reset;
extern bool button_click;


void glfw_error_callback(int error, const char* description);

GLFWwindow* InitGLFW();

void InitUI(GLFWwindow* window, const char* glsl_version);
void DestroyUI();
void RenderUI();