#pragma once

#include <mutex>
#include <thread>

// Acados NMPC
#include "nmpc/acados_nmpc.h"

// ROS
#include "ros/ros.h"

// ROS messages
#include <mavros_msgs/State.h>

#include "px4_control_msgs/DroneState.h"
#include "px4_control_msgs/Setpoint.h"
#include "px4_control_msgs/Trajectory.h"

/**
 * @brief PX4 Nonlinear Model Predictive Controller. Uses c code generated from
 * acados for the nmpc
 */
namespace px4_ctrl {
class PX4Control {
 public:
  PX4Control(ros::NodeHandle &nh, const double &rate);
  ~PX4Control();

  std::thread publisher_worker_t;

 private:
  // ROS Subscribers
  ros::Subscriber mavros_status_sub;
  ros::Subscriber drone_state_sub;
  ros::Subscriber setpoint_sub;
  ros::Subscriber trajectory_sub;

  // ROS Publishers
  ros::Publisher att_control_pub;
  ros::Publisher vel_control_pub;

  // ROS Services
  ros::ServiceClient set_mode_client;

  // Callbacks
  void mavrosStatusCallback(const mavros_msgs::State::ConstPtr &msg);
  void droneStateCallback(const px4_control_msgs::DroneState &msg);
  void setpointCallback(const px4_control_msgs::Setpoint &msg);
  void trajectoryCallback(const px4_control_msgs::Trajectory &msg);

  /**
   * @brief If tracking is enabled, it publishes the UAS controls. Should run as
   * long as the node is alive
   * @param pub_rate Publishing rate
   */
  void controlPublisher(const double &pub_rate);

  /** @brief Loads the node parameters
   */
  void loadParameters();

  // PX4
  mavros_msgs::State current_status;

  // Controller
  AcadosNMPC *nmpc_controller;
  bool enable_controller;

  model_parameters model_params;
  trajectory_setpoint drone_state;
  std::vector<double> disturbances;
  std::vector<trajectory_setpoint> current_reference_trajectory;

  std::unique_ptr<std::mutex> status_mutex;
  std::unique_ptr<std::mutex> drone_state_mutex;
};
}  // namespace px4_ctrl