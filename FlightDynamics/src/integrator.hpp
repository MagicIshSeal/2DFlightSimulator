#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

struct AircraftState {
    double x;   // horizontal position [m]
    double z;   // vertical position [m]
    double V;   // speed [m/s]
    double gamma; // flight path angle [rad]
};

AircraftState eulerStep(AircraftState state,
                        double L, double D, double W, double T,
                        double mass, double dt);

AircraftState trapezoidalStep(AircraftState state,
                              double L, double D, double W, double T,
                              double mass, double dt);

#endif
