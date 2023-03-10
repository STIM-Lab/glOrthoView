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
float gui_VolumeSize[] = {1.0f, 1.0f, 1.0f};            // initialize the volume size to 1 (uniform)
float gui_VolumeSlice[] = { 0.5f, 0.5f, 0.5f };         // current volume slice being displayed [0.0, 1.0]
float coordinates[] = {0.0f, 0.0f, 0.0f};
float ortho_world[2];                                   // stores the size of the orthographic viewports in world space
bool draw_dot = false;
bool move_plane = false;
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
        draw_dot = true;
        move_plane = true;
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

static glm::mat4 createModel(int horz_axis, int vert_axis) {
    ////////////////////////////////    Model Matrix - Planes   /////////////////////////////////////////
    glm::mat4 model_matrix;

    glm::mat4 rotation;
    glm::mat4 scale;
    glm::mat4 translation;
    float map;

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {                      
        rotation = glm::mat4(1.0f);
        scale = glm::scale(glm::mat4(1.0f), glm::vec3(gui_VolumeSize[0], gui_VolumeSize[1], 1.0f));
        map = (gui_VolumeSlice[2] * (gui_VolumeSize[2])) - (gui_VolumeSize[2] / 2);
        translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, map));
    }

    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {                  
        rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
        scale = glm::scale(glm::mat4(1.0f), glm::vec3(gui_VolumeSize[0], 1.0f, gui_VolumeSize[2]));
        map = (gui_VolumeSlice[1] * (gui_VolumeSize[1])) - (gui_VolumeSize[1] / 2);
        translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, map, 0.0f));
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {                  
        rotation = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0.0, 1.0, 0.0));
        rotation = glm::rotate(rotation, glm::radians(270.0f), glm::vec3(0.0, 0.0, 1.0));
        rotation = glm::rotate(rotation, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
        scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, gui_VolumeSize[1], gui_VolumeSize[2]));
        map = (gui_VolumeSlice[0] * (gui_VolumeSize[0])) - (gui_VolumeSize[0] / 2);
        translation = glm::translate(glm::mat4(1.0f), glm::vec3(map, 0.0f, 0.0f));
    }

    model_matrix = translation * scale * rotation;

    return model_matrix;
}

static glm::mat4 createVP(int horz_axis, int vert_axis) {

    glm::mat4 Mview;
    glm::mat4 Mproj;
    Mproj = glm::ortho(-0.5 * ortho_world[0], 0.5 * ortho_world[0], -0.5 * ortho_world[1], 0.5 * ortho_world[1], 0.0, 10000.0);

    // X-Y axis
    if (horz_axis == 0 && vert_axis == 1) {
        Mview = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 1.0f),                // eye
            glm::vec3(0.0f, 0.0f, 0.0f),                // center
            glm::vec3(0.0f, 1.0f, 0.0f));               // up
    }
    
    // X-Z axis
    else if (horz_axis == 0 && vert_axis == 2) {
        Mview = glm::lookAt(
            glm::vec3(0.0f, 1.0f, 0.0f),                // eye
            glm::vec3(0.0f, 0.0f, 0.0f),                // center
            glm::vec3(0.0f, 0.0f, 1.0f));               // up
    }

    // Y-Z axis
    else if (horz_axis == 1 && vert_axis == 2) {
        Mview = glm::lookAt(
            glm::vec3(1.0f, 0.0f, 0.0f),                // eye
            glm::vec3(0.0f, 0.0f, 0.0f),                // center
            glm::vec3(0.0f, 0.0f, 1.0f));               // up
    }

    return Mproj * Mview;
}

static glm::mat4 createVp_3D() {
    glm::mat4 Mproj = glm::ortho(-0.5 * ortho_world[0], 0.5 * ortho_world[0], -0.5 * ortho_world[1], 0.5 * ortho_world[1], 0.0, 10000.0);
    
    // initialize the camera for 3D view
    float vs_max = std::max(gui_VolumeSize[0], std::max(gui_VolumeSize[1], gui_VolumeSize[2]));         // find the maximum size of the volume
    cam.setPosition(2 * vs_max, 2 * vs_max, 2 * vs_max);                                                // eye
    cam.LookAt(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);                                                     // center and up

    glm::mat4 Mview3D = glm::lookAt(cam.getPosition(), cam.getLookAt(), cam.getUp());

    return Mproj * Mview3D;
}


static void reset_plane() {
    ////////////////////////////////////////    Reset    ////////////////////////////////////////
    float vs_max = std::max(gui_VolumeSize[0], std::max(gui_VolumeSize[1], gui_VolumeSize[2]));         // find the maximum size of the volume
    // reset the whole window to initial state if reset button is pressed
    for (int i = 0; i < 3; i++) {
        gui_VolumeSize[i] = 1.0f;
        gui_VolumeSlice[i] = 0.5f;
        coordinates[i] = 0.0f;
    }
    cam.setPosition(2 * vs_max, 2 * vs_max, 2 * vs_max);
    cam.LookAt(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    draw_dot = false;
}
//static void drawScene(glm::mat4 VP) {
//    
//    
//    glBegin(GL_LINES);
//    if (axis == 0)  glColor3f(0.9f, 0.0f, 0.0f);
//    else if (axis == 1) glColor3f(0.0f, 0.6f, 0.2f);
//    else glColor3f(0.0f, 0.4f, 1.0f);
//    glVertex3f(x1, y1, z1);
//    glVertex3f(x2, y2, z2);
//    //glTranslatef();
//    glEnd();
//}


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
    vol.load("data/*.bmp");                                             // uncomment to load from the demo image stack
    //vol.generate_rgb(256, 256, 256, 8);                                     // generate an RGB grid texture


    // generate the basic geometry and materials for rendering
    tira::glGeometry rect = tira::glGeometry::GenerateRectangle<float>();   // create a rectangle for rendering volume cross-sections
    tira::glMaterial material("slicer.shader");                             // slicer shader renders a cross-section of the geometry
    material.SetTexture("volumeTexture", vol, GL_RGB, GL_NEAREST);          // bind the volume to the material

    // generate a dot when user clicks on the plane
    tira::glGeometry sphere = tira::glGeometry::GenerateSphere<float>(20, 20);
    tira::glShader dot("dot.shader");


    



    // Projection Matrix
    glm::mat4 Mproj;
        

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
        

        // reset 3D plane to initial state if reset button is pressed
        if (reset) reset_plane();



        //////////////////////////////    Projection Matrix    //////////////////////////////////////
        float aspect = (float)display_w / (float)display_h;

        
        int ortho_pixel[2] = { display_w / 2, display_h / 2 };      // store the size of the orthographic viewports in pixels


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


        float S = std::max(Sxy, std::max(Sxz, Syz));                                // S is now the size of whichever dimension of the volume is touching the boundary of the viewport
        

        if (aspect > 1) {
            ortho_world[0] = aspect * S;
            ortho_world[1] = S;
        }
        else {
            ortho_world[0] = S;
            ortho_world[1] = (1.0 / aspect) * S;
        }
        
       

        ////////////////////////////////////    Coordinate selection    /////////////////////////////
        
        // if clicked on XY plane
        if (mouse_x > ortho_pixel[0] && mouse_y < ortho_pixel[1] && draw_dot) {                     // maps the cursor's position to (-1, 1) coordinates
            coordinates[0] = ((mouse_x / ortho_pixel[0]) - 1) * ortho_world[0] - (ortho_world[0] / 2.0f);
            coordinates[1] = -((mouse_y / ortho_pixel[1]) * ortho_world[1] - (ortho_world[1] / 2.0f));
            coordinates[2] = gui_VolumeSlice[2] - 0.5f;
            /*if (abs(coordinates[0]) <= gui_VolumeSize[0] / 2 && abs(coordinates[1]) <= gui_VolumeSize[1] / 2)
            {
                trans_xz = glm::translate(trans_xz, glm::vec3(0.0f, coordinates[1], 0.0f));
                trans_yz = glm::translate(trans_yz, glm::vec3(coordinates[0], 0.0f, 0.0f));
            }*/
        }

        // if clicked on XZ plane

        if (mouse_x > ortho_pixel[0] && mouse_y > ortho_pixel[1] && draw_dot) {                     // maps the cursor's position to (-1, 1) coordinates
            coordinates[0] = ((mouse_x / ortho_pixel[0]) - 1) * ortho_world[0] - (ortho_world[0] / 2.0f);
            coordinates[1] = gui_VolumeSlice[1] - 0.5f;
            coordinates[2] = -(((mouse_y / ortho_pixel[1]) - 1 ) * ortho_world[1] - (ortho_world[1] / 2.0f));
            /*if (abs(coordinates[0]) < 0.5 && abs(coordinates[2]) < 0.5)
            {
                trans_xy = glm::translate(trans_xy, glm::vec3(0.0f, 0.0f, coordinates[2]));
                trans_yz = glm::translate(trans_yz, glm::vec3(coordinates[0], 0.0f, 0.0f));
            }*/
        }

        // if clicked on YZ plane
        if (mouse_x < ortho_pixel[0] && mouse_y > ortho_pixel[1] && draw_dot) {                     // maps the cursor's position to (-1, 1) coordinates
            coordinates[0] = gui_VolumeSlice[0] - 0.5f;
            coordinates[1] = (mouse_x / ortho_pixel[0]) * ortho_world[0] - (ortho_world[0] / 2.0f);
            coordinates[2] = -(((mouse_y / ortho_pixel[1]) - 1) * ortho_world[1] - (ortho_world[1] / 2.0f));
            /*if (abs(coordinates[1]) < 0.5 && abs(coordinates[2]) < 0.5)
            {
                trans_xz = glm::translate(trans_xz, glm::vec3(0.0f, coordinates[1], 0.0f));
                trans_xy = glm::translate(trans_xy, glm::vec3(0.0f, 0.0f, coordinates[2]));
            }*/
        }

        // mapping the cursor's position back to the local space position
        glm::mat4 trans_sph = glm::translate(glm::mat4(1.0f), glm::vec3(coordinates[0], coordinates[1], coordinates[2]));
        glm::mat4 scale_sph = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f));

        // Model matrix: translation + rotation + scaling
        glm::mat4 model_xy = createModel(0, 1);
        glm::mat4 model_xz = createModel(0, 2);
        glm::mat4 model_yz = createModel(1, 2);
        
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                               // clear the Viewport using the clear color
        glViewport(display_w / 2, display_h / 2, display_w / 2, display_h / 2);


        /****************************************************/
        /*      Draw Stuff To The Viewport                  */
        /****************************************************/

        // Bind the volume material and render all of the viewports

        glm::vec3 volume_size = glm::vec3(gui_VolumeSize[0], gui_VolumeSize[1], gui_VolumeSize[2]);

        material.Begin();
        {

            // Upper Right (X-Y) Viewport
            glViewport(display_w / 2, display_h / 2, display_w / 2, display_h / 2);
            material.SetUniformMat4f("M", model_xy);
            material.SetUniformMat4f("VP", createVP(0, 1));
            material.SetUniform3f("S", gui_VolumeSize[0], gui_VolumeSize[1], gui_VolumeSize[2]);
            rect.Draw();

            // Lower Right (X-Z) Viewport
            glViewport(display_w / 2, 0, display_w / 2, display_h / 2);
            material.SetUniformMat4f("M", model_xz);
            material.SetUniformMat4f("VP", createVP(0, 2));
            material.SetUniform3f("S", gui_VolumeSize[0], gui_VolumeSize[1], gui_VolumeSize[2]);
            rect.Draw();

            // Lower Left (Y-Z) Viewport
            glViewport(0, 0, display_w / 2, display_h / 2);
            material.SetUniformMat4f("M", model_yz);
            material.SetUniformMat4f("VP", createVP(1, 2));
            material.SetUniform3f("S", gui_VolumeSize[0], gui_VolumeSize[1], gui_VolumeSize[2]);
            rect.Draw();

            // Render the upper left (3D) view
            glViewport(0, display_h / 2, display_w / 2, display_h / 2);
            
            
            // draw the XY plane
            material.SetUniformMat4f("M", model_xy);
            material.SetUniformMat4f("VP", createVp_3D());
            rect.Draw();

            // draw the XZ plane
            material.SetUniformMat4f("M", model_xz);
            material.SetUniformMat4f("VP", createVp_3D());
            rect.Draw();

            // draw the YZ plane
            material.SetUniformMat4f("M", model_yz);
            material.SetUniformMat4f("VP", createVp_3D());
            rect.Draw();
            
            /*if (draw_dot) {
                dot.Bind();
                dot.SetUniformMat4f("Trans", Mproj * Mview3D * trans_sph * scale_sph);
                sphere.Draw();
            }*/
            
        }
        material.End();
        

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());     // draw the GUI data from its buffer

        glfwSwapBuffers(window);                                    // swap the double buffer
    }


    DestroyUI();                                                    // Clear the ImGui user interface

    glfwDestroyWindow(window);                                      // Destroy the GLFW rendering window
    glfwTerminate();                                                // Terminate GLFW

    return 0;
}