/* Imgui headers */
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers


/* Std headers */
#include <stdio.h>

/* Aravis headers */
#include <arv.h>

/* Cystom class headers*/
#include "Camera.hpp"
#include "Texture.hpp"
#include "Image.hpp"
#include "Debug.hpp"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // states
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //TO DO MAKE CHOOSE CAMERA

    arv_update_device_list();
    unsigned int indexes = arv_get_n_devices();
    Camera* camera;
    Image* image;
    Texture* texture;

    if(indexes > 0){
        camera = new Camera(0);
        image = camera->image;
        texture = new Texture(image);

        // starts stream
        camera->startVideo();
    }else{
        printf("Not available camera\n");
        assert(1==0);
    }


    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /*Stream window*/
        ImGui::Begin("Stream");  

        if(image->imageUpdate == true){
            texture->updateTexture(camera->camConfig.currentPixelFormat);
        }
        ImGui::Image((void*)(intptr_t)texture->getTexture(), ImVec2(image->imageWidth, image->imageHeight));
        ImGui::End();

        /* Controller window*/
        ImGui::Begin("Controller"); 

        /*Pixel format*/
        ImGui::Text("Format pixel:");

        if (ImGui::BeginCombo("##combo", camera->camConfig.currentPixelFormatString)) 
        {
            for (int n = 0; n < camera->camConfig.numPixelFormats; n++)
            {
                bool is_selected = (camera->camConfig.currentPixelFormatString == camera->camConfig.availablePixelFormatsStrings[n]); 
                if (ImGui::Selectable(camera->camConfig.availablePixelFormatsStrings[n], is_selected))
                    camera->setPixelFormat(n);

                if (is_selected)
                    ImGui::SetItemDefaultFocus();   
            }
            ImGui::EndCombo();
        }

        ImGui::Dummy(ImVec2(5,5));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(5,5));

        /*Gain*/
        ImGui::Text("Gain:");

        if(camera->camConfig.gainAvailable){
            float valueGain = camera->camConfig.currentGain;
            if (ImGui::SliderFloat("##Gain", &valueGain, camera->camConfig.minGain,
                camera->camConfig.maxGain, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
                D("gain changed to %f\n", value);
                camera->setGain(valueGain);
            }
        }else{
            ImGui::Text("Gain feature not support");
        }

        ImGui::Dummy(ImVec2(5,5));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(5,5));

        /*FrameRate*/

        ImGui::Text("FrameRate:");

        if(camera->camConfig.frameRateAvailable){
        float valueFrameRate = camera->camConfig.currentFrameRate;
        if (ImGui::SliderFloat("##Frame rate", &valueFrameRate, camera->camConfig.minFrameRate,
            camera->camConfig.maxFrameRate, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
            D("frame rate changed to %f\n", value);
            camera->setFrameRate(valueFrameRate);
        }
        }else{
            ImGui::Text("FrameRate feature not supporte");
        }


        ImGui::End();


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    delete(camera);
    delete(texture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}