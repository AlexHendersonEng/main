#include <matplot/matplot.h>

#include <array>
#include <boost/numeric/odeint.hpp>
#include <vector>

using namespace boost::numeric::odeint;
using namespace matplot;

// State: [position, velocity]
using state_type = std::array<double, 2>;

// Mass-spring-damper parameters
const double m = 1.0;   // kg
const double c = 0.5;   // Ns/m
const double k = 10.0;  // N/m

// System equations:
// m*x'' + c*x' + k*x = 0
void dynamics(const state_type& x, state_type& dxdt, double /*t*/) {
  const double position = x[0];
  const double velocity = x[1];

  dxdt[0] = velocity;
  dxdt[1] = -(c / m) * velocity - (k / m) * position;
}

int main() {
  // Initial conditions:
  // x(0) = 1 m
  // v(0) = 0 m/s
  state_type x = {1.0, 0.0};

  // Simulation data
  std::vector<double> time;
  std::vector<double> position;
  std::vector<double> velocity;

  // Save every integration step
  auto observer = [&](const state_type& state, double t) {
    time.push_back(t);
    position.push_back(state[0]);
    velocity.push_back(state[1]);
  };

  // Integrate from t = 0 to 10 seconds
  integrate_const(runge_kutta4<state_type>(), dynamics, x,
                  0.0,   // start time
                  10.0,  // end time
                  0.01,  // time step
                  observer);

  // Plot displacement
  auto fig = figure(true);

  auto ax1 = subplot(2, 1, 0);
  plot(time, position);
  title("Mass-Spring-Damper System");
  xlabel("Time [s]");
  ylabel("Position [m]");
  grid(on);

  // Plot velocity
  auto ax2 = subplot(2, 1, 1);
  plot(time, velocity);
  xlabel("Time [s]");
  ylabel("Velocity [m/s]");
  grid(on);

  show();

  return 0;
}
