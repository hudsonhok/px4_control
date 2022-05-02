#include "state_estimation/sensors/base_sensor.h"

namespace px4_ctrl {
class MavrosGlobalLocal : public BaseSensor {
 public:
  MavrosGlobalLocal() {}
  MavrosGlobalLocal(Eigen::MatrixXd R_mat) : R_mat_nom(R_mat), R_mat_cur(R_mat) {}
  ~MavrosGlobalLocal() {}

  /**
   * @brief Returns the H matrix and expected sensor measurement for the
   * predicted state
   * @param state Predicted state
   * @param H_mat Sensor's H matrix
   * @param y_pred Sensor's expected measurement at the predicted state
   */
  virtual void correctionData(const eskf_state &state, Eigen::MatrixXd &H_mat,
                              Eigen::MatrixXd &y_expected) override {
    H_mat = getHmat(state);
    y_expected = getYExpected(state);
  }

  Eigen::MatrixXd getCurrentR() const { return R_mat_cur; }

 private:
  Eigen::MatrixXd R_mat_nom, R_mat_cur;
  static const int measurement_size = 7;
  static const int state_size = 20;
  static const int error_state_size = 18;

  /**
   * @brief Calculates the H matrix using the predicted state
   * @param state Predicted state
   * @returns The H matrix at the provided state
   */
  Eigen::MatrixXd getHmat(const eskf_state &state) {
    // Quaternion derivatives of R transpose
    Eigen::Matrix3d R_w = dRdqw(state.attitude).transpose();
    Eigen::Matrix3d R_x = dRdqx(state.attitude).transpose();
    Eigen::Matrix3d R_y = dRdqy(state.attitude).transpose();
    Eigen::Matrix3d R_z = dRdqz(state.attitude).transpose();

    // Construct H matrix
    Eigen::Matrix<double, measurement_size, state_size> H_mat;
    H_mat.setZero();

    // Position
    H_mat.block(0, 0, 3, 3) = Eigen::Matrix3d::Identity();

    // Attitude
    H_mat.block(3, 3, 4, 4) = Eigen::Matrix4d::Identity();

    // Error state derivative of the state
    Eigen::Matrix<double, state_size, error_state_size> Xddx;
    Xddx.setZero();

    // Position
    Xddx.block(0, 0, 3, 3) = Eigen::Matrix3d::Identity();

    // Velocity
    Xddx.block(3, 3, 3, 3) = Eigen::Matrix3d::Identity();

    // Attitude
    Xddx(3, 6) = -0.5 * state.attitude.x();
    Xddx(3, 7) = -0.5 * state.attitude.y();
    Xddx(3, 8) = -0.5 * state.attitude.z();

    Xddx(4, 6) = 0.5 * state.attitude.w();
    Xddx(4, 7) = -0.5 * state.attitude.z();
    Xddx(4, 8) = 0.5 * state.attitude.y();

    Xddx(5, 6) = 0.5 * state.attitude.z();
    Xddx(5, 7) = 0.5 * state.attitude.w();
    Xddx(5, 8) = -0.5 * state.attitude.x();

    Xddx(6, 6) = -0.5 * state.attitude.y();
    Xddx(6, 7) = 0.5 * state.attitude.x();
    Xddx(6, 8) = 0.5 * state.attitude.w();

    // Disturbances
    Xddx.block(7, 9, 3, 3) = Eigen::Matrix3d::Identity();

    return H_mat * Xddx;
  }

  /**
   * @brief Calculates the expected sensor measurement using the predicted state
   * @param state Predicted state
   * @returns The expected measurement at the provided state
   */
  Eigen::MatrixXd getYExpected(const eskf_state &state) {
    Eigen::Matrix<double, measurement_size, 1> y_exp;

    y_exp.segment(0, 3) = state.position;
    y_exp(3) = state.attitude.w();
    y_exp(4) = state.attitude.x();
    y_exp(5) = state.attitude.y();
    y_exp(6) = state.attitude.z();

    return y_exp;
  }
};
}  // namespace px4_ctrl