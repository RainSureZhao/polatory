#pragma once

#include <Eigen/Core>
#include <polatory/geometry/point3d.hpp>
#include <polatory/polynomial/monomial_basis.hpp>
#include <polatory/polynomial/polynomial_basis_base.hpp>
#include <polatory/types.hpp>

namespace polatory::polynomial {

class orthonormal_basis : public polynomial_basis_base {
 public:
  orthonormal_basis(int dimension, int degree, const geometry::points3d& points)
      : polynomial_basis_base(dimension, degree), mono_basis_(dimension, degree) {
    auto pt = mono_basis_.evaluate(points);
    auto u_hat = Eigen::MatrixXd(pt.rows(), pt.cols());

    auto size = basis_size();

    c_hat_ = Eigen::MatrixXd::Identity(size, size);

    common::valuesd u = pt.row(0);
    auto u_norm = u.norm();
    c_hat_(0, 0) /= u_norm;
    u_hat.row(0) = u / u_norm;

    for (index_t i = 1; i < size; i++) {
      u = pt.row(i);
      for (index_t j = 0; j < i; j++) {
        for (index_t k = j; k < i; k++) {
          c_hat_(i, j) -= u_hat.row(k).dot(pt.row(i)) * c_hat_(k, j);
        }
        u += c_hat_(i, j) * pt.row(j);
      }
      u_norm = u.norm();
      c_hat_.row(i) /= u_norm;
      u_hat.row(i) = u / u_norm;
    }
  }

  Eigen::MatrixXd evaluate(const geometry::points3d& points) const {
    auto pt = mono_basis_.evaluate(points);

    return c_hat_ * pt;
  }

 private:
  const monomial_basis mono_basis_;

  Eigen::MatrixXd c_hat_;
};

}  // namespace polatory::polynomial
