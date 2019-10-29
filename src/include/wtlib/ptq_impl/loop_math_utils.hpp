#ifndef PTQ_IMPL_LOOP_MATH_OPERATIONS_HPP
#define PTQ_IMPL_LOOP_MATH_OPERATIONS_HPP

#include <cassert>
#include <cmath>

#include <eigen3/Eigen/Dense>

namespace wtlib::ptq_impl
{

/**
 * @brief    Implement helper functions for computing the coefficients used in
 *           the Loop wavelet transform.
 *
 */
class Loop_math
{
public:
static double alpha(int n)
{
  double nd {static_cast<double>(n)};

  double eta = 0.375 + 0.25 * std::cos(2 * M_PI / nd);

  return 0.375 + eta * eta;
}

static double beta(int n)
{
  return (alpha(n) - 0.375) / 0.625;
}

static double gamma(int n)
{
  return (1.0 - alpha(n)) / static_cast<double>(n);
}

static double delta(int n)
{
  return (1.0 - beta(n)) / static_cast<double>(n);
}

static Eigen::Vector4d get_weight(int n0, int n1, int n2, int n3)
{
  double n0d = static_cast<double>(n0);
  double n1d = static_cast<double>(n1);
  double n2d = static_cast<double>(n2);
  double n3d = static_cast<double>(n3);

  double alpha0 = alpha(n0);
  double alpha1 = alpha(n1);
  double alpha2 = alpha(n2);
  double alpha3 = alpha(n3);

  double gamma0 = gamma(n0);
  double gamma1 = gamma(n1);
  double gamma2 = gamma(n2);
  double gamma3 = gamma(n3);

  double delta0 = delta(n0);
  double delta1 = delta(n1);

  Eigen::Matrix4d A;
  A(0, 0) = alpha0 * alpha0 + gamma1 * gamma1 + gamma2 * gamma2 + gamma3 * gamma3 + (n0d - 3.0) * 0.00390625 + n0d * (0.15625);
  A(1, 1) = gamma0 * gamma0 + alpha1 * alpha1 + gamma2 * gamma2 + gamma3 * gamma3 + (n1d - 3.0) * 0.00390625 + n1d * (0.15625);
  A(2, 2) = gamma0 * gamma0 + gamma1 * gamma1 + alpha2 * alpha2 + (n2d - 2.0) * 0.00390625 + n2d * (0.15625);
  A(3, 3) = gamma0 * gamma0 + gamma1 * gamma1 + alpha3 * alpha3 + (n3d - 2.0) * 0.00390625 + n3d * (0.15625);
  A(0, 1) = A(1, 0) = alpha0 * gamma0 + gamma1 * alpha1 + gamma2 * gamma2 + gamma3 * gamma3 + 0.328125;
  A(0, 2) = A(2, 0) = alpha0 * gamma0 + gamma1 * gamma1 + gamma2 * alpha2 + 0.33203125;
  A(0, 3) = A(3, 0) = alpha0 * gamma0 + gamma1 * gamma1 + gamma3 * alpha3 + 0.33203125;
  A(1, 2) = A(2, 1) = gamma0 * gamma0 + alpha1 * gamma1 + alpha2 * gamma2 + 0.33203125;
  A(1, 3) = A(3, 1) = gamma0 * gamma0 + alpha1 * gamma1 + alpha3 * gamma3 + 0.33203125;
  A(2, 3) = A(3, 2) = gamma0 * gamma0 + gamma1 * gamma1 + 0.015625;

  Eigen::Vector4d B;
  B(0) = -(alpha0 * delta0 + gamma1 * delta1 + 0.375);
  B(1) = -(gamma0 * delta0 + alpha1 * delta1 + 0.375);
  B(2) = B(3) = -(gamma0 * delta0 + gamma1 * delta1 + 0.125);

  return A.inverse() * B;
}

};  // class Loop_math

}  // namespace wtlib::ptq_impl
#endif  // define PTQ_IMPL_LOOP_MATH_OPERATIONS_HPP