#pragma once

#include "imgui.h"
#include "camera.hpp"
#include "../simulation/simulation_state.hpp"
#include "../core/vec2.hpp"
#include <vector>

// Render the flight path visualization
class FlightRenderer
{
public:
    float vector_scale;

    FlightRenderer() : vector_scale(0.05f) {}

    void render(const SimulationState &state, Camera &camera, bool show_vectors, ImVec2 canvas_p0, ImVec2 canvas_sz)
    {
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->PushClipRect(canvas_p0, canvas_p1, true);

        // Background
        draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

        // Auto-follow aircraft
        camera.followAircraft(static_cast<float>(state.position.x), static_cast<float>(state.position.y),
                              canvas_p0, canvas_p1, state.paused);

        // Draw ground line
        ImVec2 ground_p0 = camera.worldToScreen(-10000.0f, 0.0f, canvas_p0, canvas_p1);
        ImVec2 ground_p1 = camera.worldToScreen(10000.0f, 0.0f, canvas_p0, canvas_p1);
        draw_list->AddLine(ground_p0, ground_p1, IM_COL32(100, 200, 100, 255), 2.0f);

        // Draw grid lines
        drawGrid(draw_list, camera, canvas_p0, canvas_p1);

        // Draw flight path
        if (state.flightPath.size() > 1)
        {
            for (size_t i = 0; i < state.flightPath.size() - 1; i++)
            {
                ImVec2 p1 = camera.worldToScreen(state.flightPath[i].x, state.flightPath[i].z, canvas_p0, canvas_p1);
                ImVec2 p2 = camera.worldToScreen(state.flightPath[i + 1].x, state.flightPath[i + 1].z, canvas_p0, canvas_p1);
                draw_list->AddLine(p1, p2, IM_COL32(255, 255, 0, 255), 2.0f);
            }

            // Draw aircraft
            if (!state.flightPath.empty())
            {
                ImVec2 aircraft_pos = camera.worldToScreen(static_cast<float>(state.position.x),
                                                           static_cast<float>(state.position.y), canvas_p0, canvas_p1);
                draw_list->AddCircleFilled(aircraft_pos, 5.0f, IM_COL32(255, 0, 0, 255));

                // Draw force vectors
                if (show_vectors)
                {
                    drawForceVector(draw_list, aircraft_pos, state.F_thrust_viz, IM_COL32(0, 255, 0, 255), "Thrust");
                    drawForceVector(draw_list, aircraft_pos, state.F_drag_viz, IM_COL32(255, 128, 0, 255), "Drag");
                    drawForceVector(draw_list, aircraft_pos, state.F_lift_viz, IM_COL32(0, 255, 255, 255), "Lift");
                    drawForceVector(draw_list, aircraft_pos, state.F_weight_viz, IM_COL32(255, 0, 255, 255), "Weight");
                }
            }
        }

        draw_list->PopClipRect();
    }

private:
    void drawGrid(ImDrawList *draw_list, const Camera &camera, ImVec2 canvas_p0, ImVec2 canvas_p1)
    {
        int grid_spacing = 100;
        ImU32 grid_color = IM_COL32(80, 80, 80, 255);

        // Vertical grid lines
        for (int x = -10000; x <= 10000; x += grid_spacing)
        {
            ImVec2 p0 = camera.worldToScreen(static_cast<float>(x), -1000.0f, canvas_p0, canvas_p1);
            ImVec2 p1 = camera.worldToScreen(static_cast<float>(x), 10000.0f, canvas_p0, canvas_p1);
            draw_list->AddLine(p0, p1, grid_color, 1.0f);
        }

        // Horizontal grid lines
        for (int z = 0; z <= 10000; z += grid_spacing)
        {
            ImVec2 p0 = camera.worldToScreen(-10000.0f, static_cast<float>(z), canvas_p0, canvas_p1);
            ImVec2 p1 = camera.worldToScreen(10000.0f, static_cast<float>(z), canvas_p0, canvas_p1);
            draw_list->AddLine(p0, p1, grid_color, 1.0f);
        }
    }

    void drawForceVector(ImDrawList *draw_list, ImVec2 aircraft_pos, Vec2 force, ImU32 color, const char *label)
    {
        if (force.magnitude() < 0.1)
            return;

        float force_screen_x = static_cast<float>(force.x) * vector_scale;
        float force_screen_y = -static_cast<float>(force.y) * vector_scale;
        ImVec2 end_pos(aircraft_pos.x + force_screen_x, aircraft_pos.y + force_screen_y);

        // Draw line
        draw_list->AddLine(aircraft_pos, end_pos, color, 2.0f);

        // Draw arrowhead
        Vec2 dir = force.normalized();
        Vec2 perp(-dir.y, dir.x);
        float arrow_size = 8.0f;

        ImVec2 tip = end_pos;
        ImVec2 p1(end_pos.x - static_cast<float>(dir.x) * arrow_size + static_cast<float>(perp.x) * arrow_size * 0.5f,
                  end_pos.y + static_cast<float>(dir.y) * arrow_size - static_cast<float>(perp.y) * arrow_size * 0.5f);
        ImVec2 p2(end_pos.x - static_cast<float>(dir.x) * arrow_size - static_cast<float>(perp.x) * arrow_size * 0.5f,
                  end_pos.y + static_cast<float>(dir.y) * arrow_size + static_cast<float>(perp.y) * arrow_size * 0.5f);

        draw_list->AddTriangleFilled(tip, p1, p2, color);
        draw_list->AddText(ImVec2(end_pos.x + 5, end_pos.y - 10), color, label);
    }
};
