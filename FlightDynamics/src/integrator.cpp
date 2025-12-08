#include "integrator.hpp"
#include <cmath>

struct Derivatives {
    double dx;
    double dz;
    double dV;
    double dgamma;
};

// Compute derivatives for given state and forces
Derivatives computeDerivatives(AircraftState state,
                               double L, double D, double W, double T,
                               double mass) {
    Derivatives d;
    d.dx = state.V * cos(state.gamma);
    d.dz = state.V * sin(state.gamma);
    d.dV = (T - D - W * sin(state.gamma)) / mass;
    // Avoid division by zero when velocity is zero
    if (std::abs(state.V) < 1e-9) {
        d.dgamma = 0.0;
    } else {
        d.dgamma = (L - W * cos(state.gamma)) / (mass * state.V);
    }
    return d;
}

AircraftState trapezoidalStep(AircraftState state,
                              double L, double D, double W, double T,
                              double mass, double dt) {
    // Step 1: compute derivative at current state
    Derivatives d1 = computeDerivatives(state, L, D, W, T, mass);

    // Step 2: predictor step (Euler)
    AircraftState state_star;
    state_star.x = state.x + dt * d1.dx;
    state_star.z = state.z + dt * d1.dz;
    state_star.V = state.V + dt * d1.dV;
    state_star.gamma = state.gamma + dt * d1.dgamma;

    // Step 3: compute derivative at predicted state
    Derivatives d2 = computeDerivatives(state_star, L, D, W, T, mass);

    // Step 4: update state using average slope
    AircraftState nextState;
    nextState.x = state.x + dt * 0.5 * (d1.dx + d2.dx);
    nextState.z = state.z + dt * 0.5 * (d1.dz + d2.dz);
    nextState.V = state.V + dt * 0.5 * (d1.dV + d2.dV);
    nextState.gamma = state.gamma + dt * 0.5 * (d1.dgamma + d2.dgamma);

    return nextState;
}
