#pragma once

#include "imgui.h"

// Camera/view controls for the flight visualization
class Camera
{
public:
    ImVec2 view_offset;
    float view_scale;
    bool is_dragging;
    ImVec2 drag_start_pos;
    ImVec2 drag_start_offset;
    bool auto_follow;

    Camera()
        : view_offset(0.0f, 0.0f),
          view_scale(1.0f),
          is_dragging(false),
          drag_start_pos(0.0f, 0.0f),
          drag_start_offset(0.0f, 0.0f),
          auto_follow(true)
    {
    }

    // Convert world coordinates to screen coordinates
    ImVec2 worldToScreen(float world_x, float world_z, ImVec2 canvas_p0, ImVec2 canvas_p1) const
    {
        float screen_x = canvas_p0.x + view_offset.x + world_x * view_scale;
        float screen_y = canvas_p1.y + view_offset.y - world_z * view_scale; // Invert Y
        return ImVec2(screen_x, screen_y);
    }

    // Auto-follow the aircraft
    void followAircraft(float aircraft_x, float aircraft_y, ImVec2 canvas_p0, ImVec2 canvas_p1, bool paused)
    {
        if (!auto_follow || paused)
            return;

        ImVec2 aircraft_screen = worldToScreen(aircraft_x, aircraft_y, canvas_p0, canvas_p1);
        float margin = 100.0f;

        if (aircraft_screen.x < canvas_p0.x + margin)
        {
            view_offset.x += (canvas_p0.x + margin - aircraft_screen.x);
        }
        else if (aircraft_screen.x > canvas_p1.x - margin)
        {
            view_offset.x -= (aircraft_screen.x - (canvas_p1.x - margin));
        }

        if (aircraft_screen.y < canvas_p0.y + margin)
        {
            view_offset.y += (canvas_p0.y + margin - aircraft_screen.y);
        }
        else if (aircraft_screen.y > canvas_p1.y - margin)
        {
            view_offset.y -= (aircraft_screen.y - (canvas_p1.y - margin));
        }
    }

    // Center camera on aircraft
    void centerOnAircraft(float aircraft_x, float aircraft_y, ImVec2 canvas_p0, ImVec2 canvas_sz)
    {
        ImVec2 center_screen(canvas_p0.x + canvas_sz.x * 0.5f, canvas_p0.y + canvas_sz.y * 0.5f);
        view_offset.x = center_screen.x - canvas_p0.x - aircraft_x * view_scale;
        view_offset.y = center_screen.y - (canvas_p0.y + canvas_sz.y) + aircraft_y * view_scale;
        auto_follow = true;
    }

    void reset()
    {
        view_offset = ImVec2(0.0f, 0.0f);
        view_scale = 1.0f;
    }
};
