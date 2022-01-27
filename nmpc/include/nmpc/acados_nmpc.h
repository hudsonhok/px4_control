#pragma once

#include <vector>

// acados
#include "acados/utils/math.h"
#include "acados/utils/print.h"
#include "acados_c/external_function_interface.h"
#include "acados_c/ocp_nlp_interface.h"
#include "acados_solver_drone_w_disturbances.h"

/**
 * @brief Nonlinear Model Predictive Controller for UAS. Uses c code generated
 * from acados for the nmpc
 */
namespace px4_ctrl {

struct trajectory_setpoint {
  double pos_x;
  double pos_y;
  double pos_z;
  double vel_x;
  double vel_y;
  double vel_z;
  double q_roll;
  double q_pitch;
  double q_yaw;
};

struct model_parameters {
  double t_pitch;
  double k_pitch;
  double t_roll;
  double k_roll;
  double damp_x;
  double damp_y;
  double damp_z;
  double k_thrust;
  double gravity;
};

class AcadosNMPC {
 public:
  AcadosNMPC();
  ~AcadosNMPC();

  /**
   * @brief Initializes controller and sets the model parameters
   * @param model_params The model parameters
   * @returns True if successful
   */
  bool initializeController(const model_parameters &model_params);

  /**
   * @brief Sets the reference trajectory
   * @param trajectory The reference trajectory vector. Use a single element to
   * set a setpoint
   */
  void setTrajectory(const std::vector<trajectory_setpoint> &trajectory);

  /**
   * @brief Sets the current UAS state
   * @param state Current UAS state
   * @param disturbances Current estimation of external disturbances. Expects a
   * vector with three elements: {Fx, Fy, Fz}
   */
  void setCurrentState(const trajectory_setpoint &state,
                       const std::vector<double> &disturbances);

  /**
   * @brief If the NMPC problem is successfully solved, it updates the control
   * inputs. It advances the trajectory setpoint with every call. TODO: Maybe
   * allow the caller to set the initial point in the trajectory
   * @param ctrl The command vector with elements: {yaw_rate, pitch, roll,
   * thrust}
   * @returns True if successful
   */
  bool getCommands(std::vector<double> &ctrl);

 private:
  // Acados structures
  drone_w_disturbances_solver_capsule *acados_ocp_capsule;
  ocp_nlp_config *nlp_config;
  ocp_nlp_dims *nlp_dims;
  ocp_nlp_in *nlp_in;
  ocp_nlp_out *nlp_out;
  ocp_nlp_solver *nlp_solver;
  void *nlp_opts;

  // Vector with model parameters
  double *acados_model_parameters;

  // Trajectory variables
  std::vector<trajectory_setpoint> current_reference_trajectory;
  size_t trajectory_length;
  u_int64_t trajectory_index;

#ifndef TRACK_TIME
  double total_ocp_time;
  double min_ocp_time;
  double max_ocp_time;
  u_int64_t total_ocp_calls;
#endif

  /**
   * @brief Sets the reference trajectory. If the reference trajectory's size is
   * smaller than the discretization steps, the last value is repeated until all
   * steps are filled
   */
  void updateReference();

  /**
   * @brief Sets the initial conditions for acados
   * @param state_init Initial state of the drone contained in a setpoint struct
   */
  void updateInitialConditions(const trajectory_setpoint &state_init);

  /**
   * @brief Updates the disturbances
   * @param fdis_x Disturbance on x axis
   * @param fdis_y Disturbance on y axis
   * @param fdis_z Disturbance on z axis
   */
  void updateDisturbances(const double &fdis_x, const double &fdis_y,
                          const double &fdis_z);
};
}  // namespace px4_ctrl