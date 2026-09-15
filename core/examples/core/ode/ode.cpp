#include <matplot/matplot.h>

#include <array>
#include <boost/numeric/odeint.hpp>
#include <vector>

using namespace boost::numeric::odeint;
using namespace matplot;

template <typename T>
class Component {
 public:
  virtual ~Component() = default;

  virtual void derivatives(const T& states, T& derivatives, double t,
                           size_t& index) = 0;
};

// State:
// [x, v, dx/dc, dv/dc, dx/dk, dv/dk]
using state_type = std::array<double, 6>;

class MassSpringDamper : public Component<state_type> {
 public:
  MassSpringDamper(const double mass, const double damping,
                   const double stiffness)
      : mass_(mass), damping_(damping), stiffness_(stiffness) {}

  void derivatives(const state_type& states, state_type& derivatives, double t,
                   size_t& index) override {
    const double position = states[index];
    const double velocity = states[index + 1];

    derivatives[index] = velocity;
    derivatives[index + 1] =
        -(damping_ / mass_) * velocity - (stiffness_ / mass_) * position;

    index += 2;
  }

 protected:
  double mass_;
  double damping_;
  double stiffness_;
};

class MassSpringDamperWithSensitivity : public MassSpringDamper {
 public:
  MassSpringDamperWithSensitivity(const double mass, const double damping,
                                  const double stiffness)
      : MassSpringDamper(mass, damping, stiffness) {}

  void derivatives(const state_type& states, state_type& derivatives, double t,
                   size_t& index) override {
    MassSpringDamper::derivatives(states, derivatives, t, index);

    const double x = states[index - 2];
    const double v = states[index - 1];
    const double dx_dc = states[index];
    const double dv_dc = states[index + 1];
    const double dx_dk = states[index + 2];
    const double dv_dk = states[index + 3];

    // Parameter sensitivity dynamics: s' = (df/dy)s + df/dp
    derivatives[index] = dv_dc;
    derivatives[index + 1] =
        (-1.0 / mass_) * (damping_ * dv_dc + stiffness_ * dx_dc + v);

    derivatives[index + 2] = dv_dk;
    derivatives[index + 3] =
        (-1.0 / mass_) * (damping_ * dv_dk + stiffness_ * dx_dk + x);

    index += 4;
  }
};

// Mass-spring-damper parameters
MassSpringDamperWithSensitivity mass_spring_damper(1.0, 0.5, 10.0);

// System equations:
// m*x'' + c*x' + k*x = 0
void dynamics(const state_type& states, state_type& derivatives, double t) {
  size_t index = 0;
  mass_spring_damper.derivatives(states, derivatives, t, index);
}

int main() {
  // Initial conditions:
  // x(0) = 1 m
  // v(0) = 0 m/s
  state_type x = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  // Simulation data
  std::vector<double> time;
  std::vector<double> position;
  std::vector<double> velocity;
  std::vector<double> dx_dc_hist;
  std::vector<double> dv_dc_hist;
  std::vector<double> dx_dk_hist;
  std::vector<double> dv_dk_hist;

  // Save every integration step
  auto observer = [&](const state_type& states, const double t) {
    time.push_back(t);
    position.push_back(states[0]);
    velocity.push_back(states[1]);
    dx_dc_hist.push_back(states[2]);
    dv_dc_hist.push_back(states[3]);
    dx_dk_hist.push_back(states[4]);
    dv_dk_hist.push_back(states[5]);
  };

  // Integrate from t = 0 to 10 seconds
  integrate_const(runge_kutta4<state_type>(), dynamics, x,
                  0.0,   // start time
                  10.0,  // end time
                  0.01,  // time step
                  observer);

  // Plot displacement/velocity
  const auto fig = figure(true);

  auto ax1 = subplot(2, 1, 0);
  plot(time, position);
  xlabel("Time [s]");
  ylabel("Position [m]");
  grid(on);

  // Plot velocity
  auto ax2 = subplot(2, 1, 1);
  plot(time, velocity);
  xlabel("Time [s]");
  ylabel("Velocity [m/s]");
  grid(on);

  fig->draw();

  // Plot parameter sensitivities
  const auto fig_sens = figure(true);

  auto ax3 = subplot(2, 1, 0);
  plot(time, dx_dc_hist, "b-");
  hold(on);
  plot(time, dv_dc_hist, "r-");
  hold(off);
  xlabel("Time [s]");
  ylabel("Sensitivity");
  legend({"dx/dc", "dv/dc"});
  grid(on);

  auto ax4 = subplot(2, 1, 1);
  plot(time, dx_dk_hist, "b-");
  hold(on);
  plot(time, dv_dk_hist, "r-");
  hold(off);
  xlabel("Time [s]");
  ylabel("Sensitivity");
  legend({"dx/dk", "dv/dk"});
  grid(on);

  show();

  return 0;
}
