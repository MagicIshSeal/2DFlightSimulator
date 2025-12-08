#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "../src/integrator.hpp"
#include <cmath>

const double tol = 1e-6;

TEST_CASE("Trapezoidal step - constant zero forces") {
    AircraftState state{0.0, 0.0, 10.0, 0.0}; // initial x, z, V, gamma
    double L = 0.0, D = 0.0, W = 0.0, T = 0.0;
    double mass = 1.0;
    double dt = 1.0;

    AircraftState nextState = trapezoidalStep(state, L, D, W, T, mass, dt);

    // With zero forces and gamma=0, x should increase by V*dt, z should stay same
    REQUIRE( std::abs(nextState.x - (state.x + state.V * dt)) < tol );
    REQUIRE( std::abs(nextState.z - state.z) < tol );
    REQUIRE( std::abs(nextState.V - state.V) < tol );
    REQUIRE( std::abs(nextState.gamma - state.gamma) < tol );
}

TEST_CASE("Trapezoidal step - constant horizontal thrust") {
    AircraftState state{0.0, 0.0, 0.0, 0.0}; 
    double L = 0.0, D = 0.0, W = 0.0, T = 10.0; // N
    double mass = 2.0; // kg
    double dt = 1.0;

    AircraftState nextState = trapezoidalStep(state, L, D, W, T, mass, dt);

    // Acceleration a = T/m = 5 m/s²
    double expectedV = state.V + 5.0 * dt; // trapezoidal vs Euler is same for constant acceleration
    // For trapezoidal: x increases by average velocity * dt = (V0 + V1)/2 * dt
    double expectedX = state.x + 0.5 * (state.V + expectedV) * dt; // = 0 + 2.5 * 1 = 2.5
    double expectedZ = state.z; // no vertical motion

    REQUIRE( std::abs(nextState.V - expectedV) < tol );
    REQUIRE( std::abs(nextState.x - expectedX) < tol );
    REQUIRE( std::abs(nextState.z - expectedZ) < tol );
}

TEST_CASE("Trapezoidal step - vertical lift") {
    AircraftState state{0.0, 0.0, 10.0, 0.0};
    double mass = 1.0;
    double dt = 1.0;
    double L = 10.0, D = 0.0, W = 0.0, T = 0.0;

    AircraftState nextState = trapezoidalStep(state, L, D, W, T, mass, dt);

    // gamma should increase slightly: dgamma/dt = L/(m*V) = 10/10 = 1 rad/s
    double expectedGamma = state.gamma + 1.0 * dt; 
    REQUIRE( std::abs(nextState.gamma - expectedGamma) < tol );
}

TEST_CASE("Trapezoidal step - small dt convergence to Euler") {
    AircraftState state{0.0, 0.0, 10.0, 0.0};
    double mass = 1.0;
    double dt = 1e-6;
    double L = 0.0, D = 0.0, W = 0.0, T = 0.0;

    AircraftState eulerState = state;
    eulerState.x += dt * eulerState.V * cos(eulerState.gamma);
    eulerState.z += dt * eulerState.V * sin(eulerState.gamma);
    eulerState.V += dt * (T - D - W*sin(eulerState.gamma))/mass;
    eulerState.gamma += dt * (L - W*cos(eulerState.gamma))/(mass * eulerState.V);

    AircraftState trapState = trapezoidalStep(state, L, D, W, T, mass, dt);

    REQUIRE( std::abs(trapState.x - eulerState.x) < tol );
    REQUIRE( std::abs(trapState.z - eulerState.z) < tol );
    REQUIRE( std::abs(trapState.V - eulerState.V) < tol );
    REQUIRE( std::abs(trapState.gamma - eulerState.gamma) < tol );
}
