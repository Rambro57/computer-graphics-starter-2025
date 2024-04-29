#include "Editor.hpp"

#include <SDL.h>
#include <GL/glew.h>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

namespace Canis
{
    Editor::Editor(Window *_window, World *_world)
    {
        m_window = _window;
        m_world = _world;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        const char *glsl_version = "#version 330";
        ImGui_ImplSDL2_InitForOpenGL((SDL_Window *)m_window->GetSDLWindow(), (SDL_GLContext)m_window->GetGLContext());
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void Editor::Draw()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        //ImGui::ShowDemoWindow(&showExtra);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            int count = m_world->GetEntitiesSize();

            if (count == 0)
                return;

            Entity *entity = m_world->GetEntity(m_index);

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            if (ImGui::Button("Back"))
            {
                m_index--;

                if (m_index < 0)
                    m_index = count - 1;
            }
            ImGui::SameLine();
            ImGui::Text("Entity ID: %d", m_index);
            ImGui::SameLine();
            if (ImGui::Button("Next"))
            {
                m_index++;

                if (m_index >= count)
                    m_index = 0;
            }

            ImGui::Checkbox("active", &(entity->active));
            ImGui::InputText("tag", &(entity->tag));

            if (ImGui::CollapsingHeader("Transform"))
            {
                ImGui::InputFloat3("Position", glm::value_ptr(entity->transform.position), "%.3f");
                ImGui::InputFloat3("Rotation", glm::value_ptr(entity->transform.rotation));
                ImGui::InputFloat3("Scale", glm::value_ptr(entity->transform.scale));
            }

            if (ImGui::CollapsingHeader("Material"))
            {
                ImGui::InputFloat3("Color", glm::value_ptr(entity->color));
            }

            // ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        // glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        // glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // SDL_GL_SwapWindow((SDL_Window*)m_window->GetSDLWindow());
    }
} // end of Canis namespace