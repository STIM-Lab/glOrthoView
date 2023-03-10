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
float coordinates[] = { 0.0f, 0.0f, 0.0f };
int window_size[] = { 1600, 1200 };

tira::camera cam;                                       // create a perspective camera for 3D visualization of the volume
bool right_mouse_pressed = false;                       // flag indicates when the right mouse button is being dragged
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
        glfwGetCursorPos(window, &mouse_x, &mouse_y);                   // save the mouse position when the left button is pressed
    }
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

glm::mat4 createProjectionMatrix(float aspect) {

    glm::mat4 projection_matrix;

    // calculate the aspect ratios for each plane
    float xy_aspect = gui_VolumeSize[0] / gui_VolumeSize[1];
    float xz_aspect = gui_VolumeSize[0] / gui_VolumeSize[2];
    float yz_aspect = gui_VolumeSize[1] / gui_VolumeSize[2];

    float Sxy, Sxz, Syz;                                                                    // S will be the size of the viewport along its smallest dimension
    if (xy_aspect < aspect) {
        Sxy = gui_VolumeSize[1];
    }
    else {
        Sxy = gui_VolumeSize[0];
    }

    if (xz_aspect < aspect) {
        Sxz = gui_VolumeSize[2];
    }
    else {
        Sxz = gui_VolumeSize[0];
    }

    if (yz_aspect < aspect) {
        Syz = gui_VolumeSize[2];
    }
    else {
        Syz = gui_VolumeSize[1];
    }


    float S = std::max(Sxy, std::max(Sxz, Syz));                    // S is now the size of whichever dimension of the volume is touching the boundary of the viewport

    float ortho_world[2];                                           // stores the size of the orthographic viewports in world space
    if (aspect > 1) {
        ortho_world[0] = aspect * S;
        ortho_world[1] = S;
    }
    else {
        ortho_world[0] = S;
        ortho_world[1] = (1.0 / aspect) * S;
    }
    
    projection_matrix = glm::ortho(-0.5 * ortho_world[0], 0.5 * ortho_world[0], -0.5 * ortho_world[1], 0.5 * ortho_world[1], 0.0, 10000.0);

    return projection_matrix;
}

glm::mat4 createViewMatrix(int horz_axis, int vert_axis) {
    glm::mat4 View;

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
            glm::vec3(0.0f, 1.0f, 0.0f),                // eye
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
    glm::mat4 Rotation_Matrix;

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

glm::mat4 createScaleMatrix(int horz_axis, int vert_axis) {
    glm::mat4 Scale_Matrix;

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {
        Scale_Matrix = glm::scale(glm::mat4(1.0f), glm::vec3(gui_VolumeSize[0], gui_VolumeSize[1], 1.0f));
    }

    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {
        Scale_Matrix = glm::scale(glm::mat4(1.0f), glm::vec3(gui_VolumeSize[0], 1.0f, gui_VolumeSize[2]));
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {
        Scale_Matrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, gui_VolumeSize[1], gui_VolumeSize[2]));
    }

    return Scale_Matrix;
}

glm::mat4 createTransMatrix(int horz_axis, int vert_axis) {
    // Translation matrix
    // change the position of plane when the slice slider changes
    // when size is changed, maps from (0,1) to (-VolumeSize/2 , VolumeSize/2)

    glm::mat4 Translation_matrix;
    float map;

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {
        map = (gui_VolumeSlice[2] * (gui_VolumeSize[2])) - (gui_VolumeSize[2] / 2);
        Translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, map));
    }

    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {
        map = (gui_VolumeSlice[1] * (gui_VolumeSize[1])) - (gui_VolumeSize[1] / 2);
        Translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, map, 0.0f));
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {
        map = (gui_VolumeSlice[0] * (gui_VolumeSize[0])) - (gui_VolumeSize[0] / 2);
        Translation_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(map, 0.0f, 0.0f));
    }

    return Translation_matrix;
}

void resetPlane(float vs_max) {
    for (int i = 0; i < 3; i++) {
        gui_VolumeSize[i] = 1.0f;
        gui_VolumeSlice[i] = 0.5f;
        coordinates[i] = 0.0f;
    }
    cam.setPosition(2 * vs_max, 2 * vs_max, 2 * vs_max);
    cam.LookAt(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
}

int RenderSlices(glm::vec3 volume_size, glm::vec3 plane_positions, glm::mat4 V, glm::mat4 P, int display_w, int display_h) {

    // Load or create an example volume
    tira::glVolume<unsigned char> vol;
    vol.load("data/*.bmp");                                                   // uncomment to load from the demo image stack
    //vol.generate_rgb(256, 256, 256, 8);                                     // generate an RGB grid texture


    //bind the material
    
    // generate the basic geometry and materials for rendering
    tira::glGeometry rect = tira::glGeometry::GenerateRectangle<float>();   // create a rectangle for rendering volume cross-sections
    tira::glMaterial material("slicer.shader");                             // slicer shader renders a cross-section of the geometry
    material.SetTexture("volumeTexture", vol, GL_RGB, GL_NEAREST);          // bind the volume to the material



    glm::mat4 M;                                            // create a model matrix
    glm::mat4 rotation;                                     // create a rotation matrix
    glm::mat4 scale;                                        // create a scale matrix
    glm::mat4 translation;                                  // create a translation matrix




    // create a model matrix that scales and orients the XY plane
    rotation = createRotationMatrix(0, 1);
    scale = createScaleMatrix(0, 1);
    translation = createTransMatrix(0, 1);
    M = translation * scale * rotation;
    // render
    material.Begin();
    {
        glViewport(display_w / 2, display_h / 2, display_w / 2, display_h / 2);
        material.SetUniformMat4f("MVP", P * createViewMatrix(0, 1) * M);
        material.SetUniform1i("axis", 2);
        material.SetUniform1f("slider", gui_VolumeSlice[2]);
        rect.Draw();


        // create a matrix that scales and orients the XZ plane
        rotation = createRotationMatrix(0, 2);
        scale = createScaleMatrix(0, 1);
        translation = createTransMatrix(0, 2);
        M = translation * scale * rotation;
        // render
        glViewport(display_w / 2, 0, display_w / 2, display_h / 2);
        material.SetUniformMat4f("MVP", P * createViewMatrix(0, 2) * M);
        material.SetUniform1i("axis", 1);
        material.SetUniform1f("slider", gui_VolumeSlice[1]);
        rect.Draw();


        // create a matrix that scales and orients the YZ plane
        rotation = createRotationMatrix(1, 2);
        scale = createScaleMatrix(0, 1);
        translation = createTransMatrix(1, 2);
        M = translation * scale * rotation;
        // render
        glViewport(0, 0, display_w / 2, display_h / 2);
        material.SetUniformMat4f("MVP", P * createViewMatrix(1, 2) * model_yz);
        material.SetUniform1i("axis", 0);
        material.SetUniform1f("slider", gui_VolumeSlice[0]);
        rect.Draw();

    

        // Upper Right (X-Y) Viewport
        

        // Lower Right (X-Z) Viewport
        

        // Lower Left (Y-Z) Viewport
        

        // Render the upper left (3D) view
        //glViewport(0, display_h / 2, display_w / 2, display_h / 2);


        //// draw the XY plane
        //material.SetUniformMat4f("MVP", P * Mview3D * model_xy);
        //material.SetUniform1i("axis", 2);
        //material.SetUniform1f("slider", gui_VolumeSlice[2]);
        //rect.Draw();

        //// draw the XZ plane
        //material.SetUniformMat4f("MVP", P * Mview3D * model_xz);
        //material.SetUniform1i("axis", 1);
        //material.SetUniform1f("slider", gui_VolumeSlice[1]);
        //rect.Draw();

        //// draw the YZ plane
        //material.SetUniformMat4f("MVP", P * Mview3D * model_yz);
        //material.SetUniform1i("axis", 0);
        //material.SetUniform1f("slider", gui_VolumeSlice[0]);
        //rect.Draw();

    }
    material.End();

    return 1;
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



    // initialize the camera for 3D view
    float vs_max = std::max(gui_VolumeSize[0], std::max(gui_VolumeSize[1], gui_VolumeSize[2]));         // find the maximum size of the volume
    cam.setPosition(2 * vs_max, 2 * vs_max, 2 * vs_max);                                                // eye
    cam.LookAt(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);                                                     // center and up


    // Main event loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        RenderUI();                                                 // render the user interface (the entire thing is rendered every frame)
        int display_w, display_h;                                   // size of the frame buffer (openGL display)
        glfwGetFramebufferSize(window, &display_w, &display_h);     // get the frame buffer size

        //glViewport(0, 0, display_w, display_h);                     // specifies the area of the window where OpenGL can render
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
       


        // Reset
        if (reset) resetPlane(vs_max);



        // Projection Matrix
        float aspect = (float)display_w / (float)display_h;

        glm::mat4 Mproj = createProjectionMatrix(aspect);
        
        glm::mat4 Mview3D = glm::lookAt(cam.getPosition(), cam.getLookAt(), cam.getUp());


        // Model matrix: translation + rotation + scaling
        glm::mat4 model_xy = createTransMatrix(0, 1) * createScaleMatrix(0, 1) * createRotationMatrix(0, 1);
        glm::mat4 model_xz = createTransMatrix(0, 2) * createScaleMatrix(0, 2) * createRotationMatrix(0, 2);
        glm::mat4 model_yz = createTransMatrix(1, 2) * createScaleMatrix(1, 2) * createRotationMatrix(1, 2);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                               // clear the Viewport using the clear color
        glViewport(display_w / 2, display_h / 2, display_w / 2, display_h / 2);


        /****************************************************/
        /*      Draw Stuff To The Viewport                  */
        /****************************************************/

        // Bind the volume material and render all of the viewports
        

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());     // draw the GUI data from its buffer

        glfwSwapBuffers(window);                                    // swap the double buffer
    }


    DestroyUI();                                                    // Clear the ImGui user interface

    glfwDestroyWindow(window);                                      // Destroy the GLFW rendering window
    glfwTerminate();                                                // Terminate GLFW

    return 0;
}