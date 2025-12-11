#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm>

// Aircraft configuration entry
struct AircraftConfigEntry
{
    std::string name;
    std::string filepath;
};

// Manages loading and scanning of aircraft configuration files
class AircraftConfigManager
{
public:
    static void createDefaultConfigs(const std::string &dir)
    {
        if (!std::filesystem::exists(dir))
        {
            std::filesystem::create_directories(dir);
            SDL_Log("Created config directory: %s", dir.c_str());
        }

        struct DefaultConfig
        {
            std::string name;
            std::string json;
        };

        std::vector<DefaultConfig> defaults = {
            {"aircraft_config.json",
             "{\n"
             "    \"mass\": 120.0,\n"
             "    \"S\": 1.60,\n"
             "    \"CL_alpha\": 5.7,\n"
             "    \"CD0\": 0.025,\n"
             "    \"k\": 0.04,\n"
             "    \"maxThrust\": 500.0\n"
             "}\n"},
            {"aircraft_light.json",
             "{\n"
             "    \"mass\": 80.0,\n"
             "    \"S\": 1.20,\n"
             "    \"CL_alpha\": 6.0,\n"
             "    \"CD0\": 0.020,\n"
             "    \"k\": 0.035,\n"
             "    \"maxThrust\": 350.0\n"
             "}\n"},
            {"aircraft_heavy.json",
             "{\n"
             "    \"mass\": 180.0,\n"
             "    \"S\": 2.00,\n"
             "    \"CL_alpha\": 5.5,\n"
             "    \"CD0\": 0.030,\n"
             "    \"k\": 0.045,\n"
             "    \"maxThrust\": 700.0\n"
             "}\n"}};

        for (const auto &def : defaults)
        {
            std::string filepath = dir + "/" + def.name;
            if (!std::filesystem::exists(filepath))
            {
                std::ofstream file(filepath);
                if (file.is_open())
                {
                    file << def.json;
                    file.close();
                    SDL_Log("Created default config: %s", filepath.c_str());
                }
                else
                {
                    SDL_Log("Warning: Failed to create config file: %s", filepath.c_str());
                }
            }
        }
    }

    static std::vector<AircraftConfigEntry> scanConfigs()
    {
        std::vector<AircraftConfigEntry> configs;

        // Try multiple possible locations
        std::vector<std::string> possible_paths = {"config", "../config", "../../config"};
        std::string config_dir;

        for (const auto &path : possible_paths)
        {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                config_dir = path;
                break;
            }
        }

        // Create config directory if not found
        if (config_dir.empty())
        {
            config_dir = "config";
            SDL_Log("Config directory not found, creating default configs at: %s", config_dir.c_str());
            createDefaultConfigs(config_dir);
        }

        if (!config_dir.empty())
        {
            SDL_Log("Found config directory at: %s", std::filesystem::absolute(config_dir).string().c_str());
            createDefaultConfigs(config_dir);

            for (const auto &entry : std::filesystem::directory_iterator(config_dir))
            {
                if (entry.path().extension() == ".json")
                {
                    std::string filename = entry.path().stem().string();
                    std::string filepath = entry.path().string();
                    std::replace(filepath.begin(), filepath.end(), '\\', '/');

                    SDL_Log("Found aircraft config: %s -> %s", filename.c_str(), filepath.c_str());

                    if (filename.find("aircraft_") == 0)
                    {
                        filename = filename.substr(9);
                    }
                    if (!filename.empty())
                    {
                        filename[0] = std::toupper(filename[0]);
                    }
                    configs.push_back({filename, filepath});
                }
            }
        }
        else
        {
            SDL_Log("Warning: Could not find or create config directory");
        }

        std::sort(configs.begin(), configs.end(),
                  [](const AircraftConfigEntry &a, const AircraftConfigEntry &b)
                  { return a.name < b.name; });

        if (configs.empty())
        {
            SDL_Log("No aircraft configs found, using embedded default");
            configs.push_back({"Default (Embedded)", ""});
        }
        else
        {
            SDL_Log("Loaded %zu aircraft configurations", configs.size());
        }

        return configs;
    }
};
