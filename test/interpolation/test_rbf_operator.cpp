#include <gtest/gtest.h>

#include <Eigen/Core>
#include <polatory/interpolation/rbf_direct_evaluator.hpp>
#include <polatory/interpolation/rbf_operator.hpp>
#include <polatory/model.hpp>
#include <polatory/point_cloud/random_points.hpp>
#include <polatory/rbf/cov_exponential.hpp>
#include <polatory/types.hpp>

#include "../random_anisotropy.hpp"

using polatory::index_t;
using polatory::model;
using polatory::common::valuesd;
using polatory::geometry::sphere3d;
using polatory::interpolation::rbf_direct_evaluator;
using polatory::interpolation::rbf_operator;
using polatory::point_cloud::random_points;
using polatory::rbf::cov_exponential;

namespace {

void test_poly_degree(int poly_degree, index_t n_points) {
  auto absolute_tolerance = 1e-6;

  cov_exponential rbf({1.0, 0.2});
  rbf.set_anisotropy(random_anisotropy());

  model model(rbf, 3, poly_degree);
  model.set_nugget(0.01);

  auto points = random_points(sphere3d(), n_points);

  valuesd weights = valuesd::Random(n_points + model.poly_basis_size());

  rbf_direct_evaluator direct_eval(model, points);
  direct_eval.set_weights(weights);
  direct_eval.set_field_points(points);

  rbf_operator<> op(model, points);

  valuesd direct_op_weights = direct_eval.evaluate() + weights.head(n_points) * model.nugget();
  valuesd op_weights = op(weights);

  EXPECT_EQ(n_points + model.poly_basis_size(), op_weights.rows());

  auto max_residual = (op_weights.head(n_points) - direct_op_weights).lpNorm<Eigen::Infinity>();
  EXPECT_LT(max_residual, absolute_tolerance);

  // TODO(mizuno): Test the polynomial part.
}

}  // namespace

TEST(rbf_operator, trivial) {
  test_poly_degree(-1, 1024);
  test_poly_degree(0, 1024);
  test_poly_degree(1, 1024);
  test_poly_degree(2, 1024);
}
