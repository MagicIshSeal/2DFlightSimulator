#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Aerodynamic data table loaded from CSV
// Format: alpha (degrees), CL, CD
class AeroDataTable
{
public:
    struct DataPoint
    {
        double alpha; // Angle of attack in radians
        double CL;    // Lift coefficient
        double CD;    // Drag coefficient
    };

    // Load data from CSV file
    // Expected format: alpha,CL,CD (with optional header row)
    static AeroDataTable loadFromCSV(const std::string &filepath)
    {
        AeroDataTable table;
        std::ifstream file(filepath);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open aero data file: " + filepath);
        }

        std::string line;
        bool firstLine = true;

        while (std::getline(file, line))
        {
            // Skip empty lines
            if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos)
                continue;

            // Skip header if it contains non-numeric data
            if (firstLine)
            {
                firstLine = false;
                // Check if first character is a letter (header row)
                if (std::isalpha(line[0]))
                    continue;
            }

            std::stringstream ss(line);
            std::string token;
            DataPoint point;

            // Parse alpha (convert from degrees to radians)
            if (!std::getline(ss, token, ','))
                continue;
            point.alpha = std::stod(token) * M_PI / 180.0;

            // Parse CL
            if (!std::getline(ss, token, ','))
                continue;
            point.CL = std::stod(token);

            // Parse CD
            if (!std::getline(ss, token, ','))
                continue;
            point.CD = std::stod(token);

            table.data.push_back(point);
        }

        if (table.data.empty())
        {
            throw std::runtime_error("No valid data found in: " + filepath);
        }

        // Sort by alpha for interpolation
        std::sort(table.data.begin(), table.data.end(),
                  [](const DataPoint &a, const DataPoint &b)
                  { return a.alpha < b.alpha; });

        return table;
    }

    // Interpolate CL at given alpha (in radians)
    double getCL(double alpha) const
    {
        double CL = interpolate(alpha, [](const DataPoint &p)
                                { return p.CL; });

        // Clamp CL to minimum of 0 only when extrapolating beyond known data
        if (!data.empty() && (alpha < data.front().alpha || alpha > data.back().alpha))
        {
            return std::max(0.0, CL);
        }
        return CL;
    }

    // Interpolate CD at given alpha (in radians)
    double getCD(double alpha) const
    {
        double CD = interpolate(alpha, [](const DataPoint &p)
                                { return p.CD; });

        // When extrapolating, clamp CD to the last known value to prevent unrealistic behavior
        if (!data.empty())
        {
            if (alpha < data.front().alpha)
            {
                return data.front().CD;
            }
            if (alpha > data.back().alpha)
            {
                return data.back().CD;
            }
        }
        return CD;
    }

    // Get alpha range
    double getMinAlpha() const { return data.empty() ? 0.0 : data.front().alpha; }
    double getMaxAlpha() const { return data.empty() ? 0.0 : data.back().alpha; }

    bool isEmpty() const { return data.empty(); }

private:
    std::vector<DataPoint> data;

    // Linear interpolation helper with extrapolation
    template <typename Func>
    double interpolate(double alpha, Func getValue) const
    {
        if (data.empty())
            return 0.0;

        if (data.size() == 1)
            return getValue(data.front());

        // Extrapolate below minimum alpha using slope from first two points
        if (alpha < data.front().alpha)
        {
            double slope = (getValue(data[1]) - getValue(data[0])) / (data[1].alpha - data[0].alpha);
            double delta_alpha = alpha - data.front().alpha;
            return getValue(data.front()) + slope * delta_alpha;
        }

        // Extrapolate above maximum alpha using slope from last two points
        if (alpha > data.back().alpha)
        {
            size_t n = data.size();
            double slope = (getValue(data[n - 1]) - getValue(data[n - 2])) / (data[n - 1].alpha - data[n - 2].alpha);
            double delta_alpha = alpha - data.back().alpha;
            return getValue(data.back()) + slope * delta_alpha;
        }

        // Interpolate within data range
        for (size_t i = 0; i < data.size() - 1; i++)
        {
            if (alpha >= data[i].alpha && alpha <= data[i + 1].alpha)
            {
                // Linear interpolation
                double t = (alpha - data[i].alpha) / (data[i + 1].alpha - data[i].alpha);
                return getValue(data[i]) + t * (getValue(data[i + 1]) - getValue(data[i]));
            }
        }

        return getValue(data.back());
    }
};
