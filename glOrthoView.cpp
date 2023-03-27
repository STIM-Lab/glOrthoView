/// This is an OpenGL "Hello World!" file that provides simple examples of integrating ImGui with GLFW
/// for basic OpenGL applications. The file also includes headers for the TIRA::GraphicsGL classes, which
/// provide an basic OpenGL front-end for creating materials and models for rendering.


#include "tira/graphics_gl.h"



#include <iostream>
#include <string>
#include <stdio.h>
#include "gui.h"


GLFWwindow* window;                                     // pointer to the GLFW window that will be created (used in GLFW calls to request properties)
const char* glsl_version = "#version 130";              // specify the version of GLSL
ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);   // specify the OpenGL color used to clear the back buffer
float gui_VolumeSize[] = { 1.0f, 1.0f, 1.0f };            // initialize the volume size to 1 (uniform)
float gui_VolumeSlice[] = { 0.5f, 0.5f, 0.5f };         // current volume slice being displayed [0.0, 1.0]
float coords[] = { 0.0f, 0.0f, 0.0f };
bool window_focused = false;
tira::camera cam;                                       // create a perspective camera for 3D visualization of the volume
bool right_mouse_pressed = false;                       // flag indicates when the right mouse button is being dragged
bool left_mouse_pressed = false;                        // flag indicates when the left mouse button is being dragged

double mouse_x, mouse_y;
double THETA = 0.02;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        right_mouse_pressed = true;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);                   // save the mouse position when the right button is pressed
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        right_mouse_pressed = false;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        left_mouse_pressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        left_mouse_pressed = false;
}
        
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (right_mouse_pressed) {
        double dx = xpos - mouse_x;
        double dy = ypos - mouse_y;

        mouse_x = xpos;
        mouse_y = ypos;

        cam.OrbitFocus(-THETA * dx, THETA * dy);
    }
}

glm::vec2 VolSizeMax(float aspect, glm::vec3 volume_size) {
    // calculate the aspect ratios for each plane
    float xy_aspect = volume_size.x / volume_size.y;
    float xz_aspect = volume_size.x / volume_size.z;
    float yz_aspect = volume_size.y / volume_size.z;

    float Sxy, Sxz, Syz;                                // S will be the size of the viewport along its smallest dimension
    if (xy_aspect < aspect) {
        Sxy = volume_size.y;
    }
    else {
        Sxy = volume_size.x;
    }

    if (xz_aspect < aspect) {
        Sxz = volume_size.z;
    }
    else {
        Sxz = volume_size.x;
    }

    if (yz_aspect < aspect) {
        Syz = volume_size.z;
    }
    else {
        Syz = volume_size.y;
    }

    float S = std::max(Sxy, std::max(Sxz, Syz));       // S is now the size of whichever dimension of the volume is touching the boundary of the viewport
    glm::vec2 ortho_world(1.0f);

    if (aspect > 1) {
        ortho_world.x = aspect * S;
        ortho_world.y = S;
    }
    else {
        ortho_world.x = S;
        ortho_world.y = (1.0 / aspect) * S;
    }
    return ortho_world;                                       
}

glm::mat4 createProjectionMatrix(float aspect, glm::vec3 volume_size) {

    glm::mat4 projection_matrix;

    glm::vec2 ortho_world = VolSizeMax(aspect, volume_size);                   // stores the size of the orthographic viewports in world space
    
    projection_matrix = glm::ortho(-0.5 * ortho_world.x, 0.5 * ortho_world.x, -0.5 * ortho_world.y, 0.5 * ortho_world.y, 0.0, 10000.0);

    return projection_matrix;
}

glm::mat4 createViewMatrix(int horz_axis, int vert_axis) {
    glm::mat4 View(1.0f);

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {
        View = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 1.0f),                // eye
            glm::vec3(0.0f, 0.0f, 0.0f),                // center
            glm::vec3(0.0f, 1.0f, 0.0f));               // up
    }

    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {
        View = glm::lookAt(
            glm::vec3(0.0f, -1.0f, 0.0f),                // eye
            glm::vec3(0.0f, 0.0f, 0.0f),                // center
            glm::vec3(0.0f, 0.0f, 1.0f));               // up
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {
        View = glm::lookAt(
            glm::vec3(1.0f, 0.0f, 0.0f),                // eye
            glm::vec3(0.0f, 0.0f, 0.0f),                // center
            glm::vec3(0.0f, 0.0f, 1.0f));               // up
    }

    return View;
}

glm::mat4 createRotationMatrix(int horz_axis, int vert_axis) {
    glm::mat4 Rotation_Matrix(1.0f);

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {
        Rotation_Matrix = glm::mat4(1.0f);
    }

    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {
        Rotation_Matrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {
        Rotation_Matrix = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0.0, 1.0, 0.0));
        Rotation_Matrix = glm::rotate(Rotation_Matrix, glm::radians(270.0f), glm::vec3(0.0, 0.0, 1.0));
        Rotation_Matrix = glm::rotate(Rotation_Matrix, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
    }

    return Rotation_Matrix;
}

glm::mat4 createScaleMatrix(int horz_axis, int vert_axis, glm::vec3 volume_size) {
    glm::mat4 Scale_Matrix(1.0f);

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {
        Scale_Matrix = glm::scale(glm::mat4(1.0f), glm::vec3(volume_size.x, volume_size.y, 1.0f));
    }

    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {
        Scale_Matrix = glm::scale(glm::mat4(1.0f), glm::vec3(volume_size.x, 1.0f, volume_size.z));
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {
        Scale_Matrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, volume_size.y, volume_size.z));
    }

    return Scale_Matrix;
}

glm::mat4 createTransMatrix(int horz_axis, int vert_axis, glm::vec3 volume_size, glm::vec3 plane_positions) {
    // Translation matrix
    // change the position of plane when the slice slider changes
    // when size is changed, maps from (0,1) to (-VolumeSize/2 , VolumeSize/2)

    glm::mat4 Translation_matrix(1.0f);
    float map;

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {
        map = (plane_positions.z * (volume_size.z)) - (volume_size.z / 2);
        Translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, map));
    }

    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {
        map = (plane_positions.y * (volume_size.y)) - (volume_size.y / 2);
        Translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, map, 0.0f));
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {
        map = (plane_positions.x * (volume_size.x)) - (volume_size.x / 2);
        Translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(map, 0.0f, 0.0f));
    }

    return Translation_matrix;
}

void SetGlobalVariables(glm::vec3 plane_position, glm::vec3 coordinates) {
    gui_VolumeSlice[0] = plane_position.x;
    gui_VolumeSlice[1] = plane_position.y;
    gui_VolumeSlice[2] = plane_position.z;

    coords[0] = coordinates.x;
    coords[1] = coordinates.y;
    coords[2] = coordinates.z;
}

void MaxRange(glm::vec3 &num, glm::vec3 MaxRange) {
    int sign;
    if (abs(num.x) > MaxRange.x) {
        sign = (num.x >= 0) ? 1 : -1;
        num.x = sign * MaxRange.x;
    }
    if (abs(num.y) > MaxRange.y) {
        sign = (num.y >= 0) ? 1 : -1;
        num.y = sign * MaxRange.y;
    }
    if (abs(num.z) > MaxRange.z) {
        sign = (num.z >= 0) ? 1 : -1;
        num.z = sign * MaxRange.z;
    }

}
bool IsInRange(glm::vec3 num, glm::vec3 MaxRange) {
    if (abs(num.x) < MaxRange.x && abs(num.y) < MaxRange.y && abs(num.z) < MaxRange.z)
        return true;
    return false;
}

void coordinates_select(GLFWwindow* window, glm::vec3 &coordinates, int display_w, int display_h, glm::vec3 volume_size, glm::vec3 &plane_position) {
    
    double coord_x, coord_y;
    glfwGetCursorPos(window, &coord_x, &coord_y);
    float aspect = (float)display_w / (float)display_h;
    glm::vec2 ortho_world = VolSizeMax(aspect, volume_size);
    int half_disp_w = display_w / 2;
    int half_disp_h = display_h / 2;
    bool InRange = false;


    // XY plane
    // window x: (800, 1600)   y: (0, 600)
    // maps all coordinates to the VolumeSize * aspect ratio
    if (coord_x > half_disp_w && coord_y < half_disp_h && left_mouse_pressed) {
        coordinates.x = ((coord_x / half_disp_w) - 1) * ortho_world.x - (ortho_world.x / 2.0f);
        coordinates.y = -((coord_y / half_disp_h) * ortho_world.y - (ortho_world.y / 2.0f));
        coordinates.z = plane_position.z - 0.5f;
    }
    // XZ plane
    // window x: (800, 1600)   y: (600, 1200)
    // maps all coordinates to the VolumeSize * aspect ratio
    if (coord_x > half_disp_w && coord_y > half_disp_h && left_mouse_pressed) {
        coordinates.x = ((coord_x / half_disp_w) - 1) * ortho_world.x - (ortho_world.x / 2.0f);
        coordinates.y = plane_position.y - 0.5f;
        coordinates.z = -(((coord_y / half_disp_h) - 1) * ortho_world.y - (ortho_world.y/ 2.0f));
    }

    // YZ plane
    // window x: (800, 1600)   y: (0, 600)
    // maps all coordinates to the VolumeSize * aspect ratio
    if (coord_x < half_disp_w && coord_y > half_disp_h && left_mouse_pressed) {
        coordinates.x = plane_position.x - 0.5f;
        coordinates.y = (coord_x / half_disp_w) * ortho_world.x - (ortho_world.x / 2.0f);
        coordinates.z = -(((coord_y / half_disp_h) - 1) * ortho_world.y - (ortho_world.y / 2.0f));
    }

    // maps back to (0,1)
    MaxRange(coordinates, volume_size / 2.0f);                          // if any of the selected coordinates are outside the volume, sets it the nearest value
    InRange = IsInRange(coordinates, volume_size / 2.0f);               // checks if all the selected coordinates are in the volume range

    if(InRange)                                                         // changes the plane position only when user clicks on the volume area
    {
        plane_position.x = coordinates.x / volume_size.x + 0.5f;
        plane_position.y = coordinates.y / volume_size.y + 0.5f;
        plane_position.z = coordinates.z / volume_size.z + 0.5f;
    }

}

void resetPlane(float vs_max) {
    for (int i = 0; i < 3; i++) {
        gui_VolumeSize[i] = 1.0f;
        gui_VolumeSlice[i] = 0.5f;
        coords[i] = 0.0f;
    }
    cam.setPosition(2 * vs_max, 2 * vs_max, 2 * vs_max);
    cam.LookAt(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

void inline draw_axes(tira::glGeometry cylinder, tira::glShader shader, glm::mat4 projection, glm::mat4 view, glm::vec3 volume_size, glm::vec3 plane_positions) {
    glm::mat4 rotation(1.0f);
    glm::mat4 translation(1.0f);
    glm::mat4 scale(1.0f);
    glm::mat4 M(1.0f);
    shader.Bind();

    // X axis
    rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
    scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.05f, 0.05f)) * createScaleMatrix(0, 1, volume_size);
    translation = createTransMatrix(0, 1, volume_size, plane_positions) * createTransMatrix(0, 2, volume_size, plane_positions);
    translation *= glm::translate(glm::mat4(1.0f), glm::vec3(-volume_size.x / 2.0f, 0.0f, 0.0f));
    M = translation * scale * rotation;
    shader.SetUniformMat4f("MVP", projection * view * M);
    shader.SetUniform1i("axis", 0);
    cylinder.Draw();

    // Y axis
    rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 1.0f, 0.05f)) * createScaleMatrix(1, 2, volume_size);
    translation = createTransMatrix(1, 2, volume_size, plane_positions) * createTransMatrix(0, 1, volume_size, plane_positions);
    translation *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -volume_size.y / 2, 0.0f));
    M = translation * scale * rotation;
    shader.SetUniformMat4f("MVP", projection * view * M);
    shader.SetUniform1i("axis", 1);
    cylinder.Draw();

    // Z axis
    rotation = glm::mat4(1.0f);
    scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 1.0f)) * createScaleMatrix(1, 2, volume_size);
    translation = createTransMatrix(1, 2, volume_size, plane_positions) * createTransMatrix(0, 2, volume_size, plane_positions);
    translation *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -volume_size.z / 2.0f));
    M = translation * scale * rotation;
    shader.SetUniformMat4f("MVP", projection * view * M);
    shader.SetUniform1i("axis", 2);
    cylinder.Draw();
}


/// <summary>
/// Renders all three planes using different viewports 
/// </summary>
/// <param name="volume_size"> Volume sizes along each axis including Sx, Sy, Sz </param>
/// <param name="plane_positions"> Position of each plane inside the volume - ranges (0, Sx) (0, Sy) (0, Sz) </param>
/// <param name="V"> The view matrix </param>
/// <param name="P"> The projection matrix </param>
/// <param name="rect"></param>
/// <param name="material"></param>
void inline RenderSlices(glm::vec3 volume_size, glm::vec3 plane_positions, glm::mat4 V, glm::mat4 P,
    tira::glGeometry rect, tira::glMaterial material, tira::glGeometry cylinder, tira::glShader shader) {

    glm::mat4 M1, M2, M3;                                   // create a model matrix
    glm::mat4 rotation;                                     // create a rotation matrix
    glm::mat4 scale;                                        // create a scale matrix
    glm::mat4 translation;                                  // create a translation matrix

    material.Begin();
    {
        // create a model matrix that scales and orients the XY plane
        rotation = createRotationMatrix(0, 1);
        scale = createScaleMatrix(0, 1, volume_size);
        translation = createTransMatrix(0, 1, volume_size, plane_positions);
        M1 = translation * scale * rotation;
        // render - Upper Right (X-Y) Viewport
        material.SetUniformMat4f("MVP", P * V * M1);
        material.SetUniform1i("axis", 2);
        material.SetUniform1f("slider", plane_positions.z);
        rect.Draw();
        
        
        // create a model matrix that scales and orients the XZ plane
        rotation = createRotationMatrix(0, 2);
        scale = createScaleMatrix(0, 2, volume_size);
        translation = createTransMatrix(0, 2, volume_size, plane_positions);
        M2 = translation * scale * rotation;
        // render - Lower Right (X-Z) Viewport
        material.SetUniformMat4f("MVP", P * V * M2);
        material.SetUniform1i("axis", 1);
        material.SetUniform1f("slider", plane_positions.y);
        rect.Draw();

        // create a model matrix that scales and orients the YZ plane
        rotation = createRotationMatrix(1, 2);
        scale = createScaleMatrix(1, 2, volume_size);
        translation = createTransMatrix(1, 2, volume_size, plane_positions);
        M3 = translation * scale * rotation;
        // render - Lower Left (Y-Z) Viewport
        material.SetUniformMat4f("MVP", P * V * M3);
        material.SetUniform1i("axis", 0);
        material.SetUniform1f("slider", plane_positions.x);
        rect.Draw();
        
    }
    material.End();
    draw_axes(cylinder, shader, P, V, volume_size, plane_positions);
}


int main(int argc, char** argv)
{
    // Initialize OpenGL
    window = InitGLFW();                                // create a GLFW window

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    InitUI(window, glsl_version);                       // initialize ImGui

    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    // Load or create an example volume
    tira::glVolume<unsigned char> vol;
    vol.load("data/*.bmp");                                                    // uncomment to load from the demo image stack
    //vol.generate_rgb(256, 256, 256);                                           // generate an RGB grid texture


    // generate the basic geometry and materials for rendering
    tira::glGeometry rect = tira::glGeometry::GenerateRectangle<float>();           // create a rectangle for rendering volume cross-sections
    tira::glMaterial material("slicer.shader");                                     // slicer shader renders a cross-section of the geometry
    material.SetTexture("volumeTexture", vol, GL_RGB, GL_NEAREST);                  // bind the volume to the material

    /////////////////////////////////////////////////////////////////
    // ////////////////////////////////////////////////////////// //
   // Create a new Cylinder object
    tira::glGeometry cylinder = tira::glGeometry::GenerateCylinder<float>(10, 20);
    tira::glShader cylinder_shader("axes.shader");



    // initialize the camera for 3D view
    float vs_max = std::max(gui_VolumeSize[0], std::max(gui_VolumeSize[1], gui_VolumeSize[2]));         // find the maximum size of the volume
    cam.setPosition(2 * vs_max, 2 * vs_max, 2 * vs_max);                                                // eye
    cam.LookAt(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);                                                     // center and up

    int cnt;
    // Main event loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        RenderUI();                                                         // render the user interface (the entire thing is rendered every frame)
        int display_w, display_h;                                           // size of the frame buffer (openGL display)
        glfwGetFramebufferSize(window, &display_w, &display_h);             // get the frame buffer size

        //glViewport(0, 0, display_w, display_h);                           // specifies the area of the window where OpenGL can render
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
       

        // Reset
        if (reset) resetPlane(vs_max);

        glm::vec3 volume_size = glm::vec3(gui_VolumeSize[0], gui_VolumeSize[1], gui_VolumeSize[2]);
        glm::vec3 plane_position = glm::vec3(gui_VolumeSlice[0], gui_VolumeSlice[1], gui_VolumeSlice[2]);
        glm::vec3 coordinates = glm::vec3(coords[0], coords[1], coords[2]);
        

        // Projection Matrix
        float aspect = (float)display_w / (float)display_h;
        glm::mat4 Mproj = createProjectionMatrix(aspect, volume_size);
        

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                               // clear the Viewport using the clear color



        /****************************************************/
        /*      Draw Stuff To The Viewport                  */
        /****************************************************/

        glm::mat4 ViewMatrix(1.0f);

        // coordination selection is not applied when user clicks on the imgui window
        if (!window_focused)                                      
            coordinates_select(window, coordinates, display_w, display_h, volume_size, plane_position);


        // Sets global varilabes (gui_VolumeSlice and coords) to the updated values and view on imgui window
        SetGlobalVariables(plane_position, coordinates);


        // Bind the volume material and render all of the viewports
        
        // render - Upper Right (X-Y) Viewport
        glViewport(display_w / 2, display_h / 2, display_w / 2, display_h / 2);
        ViewMatrix = createViewMatrix(0, 1);
        RenderSlices(volume_size, plane_position, ViewMatrix, Mproj, rect, material, cylinder, cylinder_shader);

        // render - Lower Right (X-Z) Viewport
        glViewport(display_w / 2, 0, display_w / 2, display_h / 2);
        ViewMatrix = createViewMatrix(0, 2);
        RenderSlices(volume_size, plane_position, ViewMatrix, Mproj, rect, material, cylinder, cylinder_shader);

        // render - Lower Left (Y-Z) Viewport
        glViewport(0, 0, display_w / 2, display_h / 2);
        ViewMatrix = createViewMatrix(1, 2);
        RenderSlices(volume_size, plane_position, ViewMatrix, Mproj, rect, material, cylinder, cylinder_shader);

        // Render the upper left (3D) view
        glViewport(0, display_h / 2, display_w / 2, display_h / 2);
        glm::mat4 Mview3D = glm::lookAt(cam.getPosition(), cam.getLookAt(), cam.getUp());
        RenderSlices(volume_size, plane_position, Mview3D, Mproj, rect, material, cylinder, cylinder_shader);
        
        
        



        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());     // draw the GUI data from its buffer
        glfwSwapBuffers(window);                                    // swap the double buffer
    }


    DestroyUI();                                                    // Clear the ImGui user interface

    glfwDestroyWindow(window);                                      // Destroy the GLFW rendering window
    glfwTerminate();                                                // Terminate GLFW

    return 0;
}