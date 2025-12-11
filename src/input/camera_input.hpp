#pragma once

#include "imgui.h"
#include "../graphics/camera.hpp"
#include <algorithm>

// Handle mouse and keyboard input for camera control
class CameraInput
{
public:
    void handleInput(Camera &camera, ImVec2 canvas_p0, ImVec2 canvas_sz, bool is_hovered)
    {
        // Mouse dragging for panning
        if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            camera.is_dragging = true;
            camera.drag_start_pos = ImGui::GetMousePos();
            camera.drag_start_offset = camera.view_offset;
            camera.auto_follow = false;
        }

        if (camera.is_dragging)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                camera.view_offset.x = camera.drag_start_offset.x + (mouse_pos.x - camera.drag_start_pos.x);
                camera.view_offset.y = camera.drag_start_offset.y + (mouse_pos.y - camera.drag_start_pos.y);
            }
            else
            {
                camera.is_dragging = false;
            }
        }

        // Mouse wheel for zooming
        if (is_hovered)
        {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f)
            {
                float zoom_factor = wheel > 0 ? 1.1f : 0.9f;
                camera.view_scale *= zoom_factor;
                camera.view_scale = std::max(0.1f, std::min(camera.view_scale, 10.0f));
            }
        }
    }
};
