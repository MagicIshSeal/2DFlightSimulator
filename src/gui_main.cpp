// FlightDynamics GUI - Vector-Based 2D Flight Simulator with Dear ImGui
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

// Simulation
#include "simulation/simulation_state.hpp"
#include "simulation/physics_update.hpp"

// Graphics
#include "graphics/camera.hpp"
#include "graphics/flight_renderer.hpp"
#include "graphics/ui_panels.hpp"

// Input
#include "input/camera_input.hpp"

// Utils
#include "utils/aircraft_config_manager.hpp"

int main(int, char **)
{
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return -1;
    }

    // Setup OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    SDL_Window *window = SDL_CreateWindow("FlightDynamics - 2D Flight Simulator", 1280, 720, window_flags);
    if (window == NULL)
    {
        SDL_Log("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL)
    {
        SDL_Log("SDL_GL_CreateContext Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Initialize systems
    SimulationState sim_state;
    Camera camera;
    FlightRenderer renderer;
    CameraInput camera_input;
    UIState ui_state;

    // Load aircraft configurations
    auto configs = AircraftConfigManager::scanConfigs();
    for (const auto &config : configs)
    {
        ui_state.aircraft_configs.push_back({config.name, config.filepath});
        ui_state.aircraft_name_storage.push_back(config.name);
    }
    for (const auto &name : ui_state.aircraft_name_storage)
    {
        ui_state.aircraft_names.push_back(name.c_str());
    }

    // Performance tracking
    Uint64 last_frame_time = SDL_GetPerformanceCounter();
    Uint64 perf_frequency = SDL_GetPerformanceFrequency();
    float frame_times[60] = {0};
    int frame_time_index = 0;
    int frame_count = 0;

    // Main loop
    bool done = false;
    while (!done)
    {
        // Measure frame time
        Uint64 current_frame_time = SDL_GetPerformanceCounter();
        float delta_time = (float)(current_frame_time - last_frame_time) / perf_frequency;
        last_frame_time = current_frame_time;

        frame_times[frame_time_index] = delta_time * 1000.0f;
        frame_time_index = (frame_time_index + 1) % 60;
        frame_count++;

        if (frame_count % 10 == 0)
        {
            float sum = 0.0f;
            for (int i = 0; i < 60; i++)
                sum += frame_times[i];
            ui_state.avg_frame_time = sum / 60.0f;
            ui_state.avg_fps = ui_state.avg_frame_time > 0.0f ? 1000.0f / ui_state.avg_frame_time : 0.0f;
        }

        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Reset if requested
        if (sim_state.reset_requested)
        {
            sim_state.reset();
        }

        // Update simulation
        updatePhysics(sim_state);

        // Render UI panels
        renderControlPanel(sim_state, ui_state);

        // Flight Path Visualization
        ImGui::SetNextWindowPos(ImVec2(420, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(850, 500), ImGuiCond_FirstUseEver);
        ImGui::Begin("Flight Path Visualization");

        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        if (canvas_sz.x < 50.0f)
            canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f)
            canvas_sz.y = 50.0f;

        // Handle input
        ImGui::SetCursorScreenPos(canvas_p0);
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        bool is_hovered = ImGui::IsItemHovered();
        camera_input.handleInput(camera, canvas_p0, canvas_sz, is_hovered);

        // Render flight visualization
        renderer.render(sim_state, camera, ui_state.show_vectors, canvas_p0, canvas_sz);

        ImGui::Text("Controls: Left-click drag to pan, Mouse wheel to zoom");
        ImGui::Text("Zoom: %.2fx | Position: (%.0f, %.0f) m", camera.view_scale, sim_state.position.x, sim_state.position.y);
        ImGui::Checkbox("Show Force Vectors", &ui_state.show_vectors);
        if (ui_state.show_vectors)
        {
            ImGui::SameLine();
            ImGui::SliderFloat("Vector Scale", &renderer.vector_scale, 0.001f, 0.2f, "%.3f", ImGuiSliderFlags_Logarithmic);
        }
        ImGui::Checkbox("Auto-Follow Aircraft", &camera.auto_follow);
        ImGui::SameLine();
        if (ImGui::Button("Reset View"))
        {
            camera.reset();
        }
        ImGui::SameLine();
        if (ImGui::Button("Center on Aircraft"))
        {
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
            camera.centerOnAircraft(static_cast<float>(sim_state.position.x), static_cast<float>(sim_state.position.y),
                                    canvas_p0, canvas_sz);
        }

        ImGui::End();

        // Instrumentation Panel
        renderInstrumentationPanel(sim_state);

        // Optional windows
        if (ui_state.show_demo)
            ImGui::ShowDemoWindow(&ui_state.show_demo);
        if (ui_state.show_metrics)
            ImGui::ShowMetricsWindow(&ui_state.show_metrics);

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(ui_state.clear_color.x * ui_state.clear_color.w,
                     ui_state.clear_color.y * ui_state.clear_color.w,
                     ui_state.clear_color.z * ui_state.clear_color.w,
                     ui_state.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
