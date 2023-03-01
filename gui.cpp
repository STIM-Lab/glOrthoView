#include "gui.h"

#include <iostream>

float ui_scale = 1.5f;                                  // scale value for the UI and UI text
//float a_slider_value;                                   // UI variable used to store the value of a floating point slider widget

bool reset = false;
extern float gui_VolumeSize[];
extern float gui_VolumeSlice[];
extern float coordinates[];

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* InitGLFW() {
    GLFWwindow* window;

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return NULL;

    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    window = glfwCreateWindow(1600, 1200, "ImGui GLFW+OpenGL3 Hello World Program", NULL, NULL);
    if (window == NULL)
        return NULL;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    return window;
}

/// <summary>
/// Initialize the GUI
/// </summary>
/// <param name="window">Pointer to the GLFW window that will be used for rendering</param>
/// <param name="glsl_version">Version of GLSL that will be used</param>
void InitUI(GLFWwindow* window, const char* glsl_version) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(ui_scale);
    ImGui::GetIO().FontGlobalScale = ui_scale;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", ui_scale * 16.0f);

}

/// <summary>
/// Destroys the ImGui rendering interface (usually called when the program closes)
/// </summary>
void DestroyUI() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

/// <summary>
/// This function renders the user interface every frame
/// </summary>
void RenderUI() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Display a Demo window showing what ImGui is capable of
    // See https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html for code details
    //ImGui::ShowDemoWindow();

    // Hello World GUI Window
    {

        float old_size = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale *= 0.7;
        ImGui::PushFont(ImGui::GetFont());

        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Volume Slicer");
        
        // Adjusting the size of the volume along each axis
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(230, 0, 0, 255));
        ImGui::Text("\t\tX\t\t");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 153, 51, 255));
        ImGui::Text("\t\tY\t\t");
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 102, 255, 255));
        ImGui::Text("\t\tZ");
        ImGui::PopStyleColor(3);
        ImGui::SliderFloat3("Volume Size", gui_VolumeSize, 0.25f, 2.0f);
        ImGui::SliderFloat3("Volume Slice", gui_VolumeSlice, 0.0f, 1.0f);
        ImGui::Spacing();
        reset = ImGui::Button("Reset", ImVec2(70, 35));
        ImGui::Spacing();

        if (ImGui::BeginTable("Coordinates", 2, ImGuiTableFlags_Resizable + ImGuiTableFlags_Borders, ImVec2(0.0f, 5.0f), 2.0f))
        {
            ImGui::TableSetupColumn("Axis");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("X");
            ImGui::TableNextColumn();
            ImGui::Text("%f", coordinates[0]);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Y");
            ImGui::TableNextColumn();
            ImGui::Text("%f", coordinates[1]);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Z");
            ImGui::TableNextColumn();
            ImGui::Text("%f", coordinates[2]);
            ImGui::EndTable();
        }

        ImGui::GetFont()->Scale = old_size;
        ImGui::PopFont();
        ImGui::End();
    }

    

    



    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);  // Render a separate window showing the FPS

    ImGui::Render();                                                            // Render all windows
}