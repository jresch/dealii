// ---------------------------------------------------------------------
//
// Copyright (C) 2017 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------

// Check projection property of VectorTools::interpolate for
// Hdiv conforming spaces on something nontrivial.

#include <deal.II/base/quadrature_lib.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/fe/fe_nedelec.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_raviart_thomas.h>
#include <deal.II/fe/mapping_q.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria.h>
#include <deal.II/lac/vector.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/fe_field_function.h>

#include "../tests.h"

template <int dim>
class F : public Function<dim>
{
public:
  F(const unsigned int q) : Function<dim>(dim), q(q) {}

  virtual double value(const Point<dim> &p, const unsigned int) const
  {
    double v = 0;
    for (unsigned int d = 0; d < dim; ++d)
      for (unsigned int i = 0; i <= q; ++i)
        v += (d + 1) * (i + 1) * std::pow(p[d], 1. * i);
    return v;
  }

private:
  const unsigned int q;
};

template <int dim, typename T>
void test(const FiniteElement<dim> &fe,
          const T &f,
          const unsigned int n_comp,
          const unsigned int order_mapping,
          bool distort_mesh)
{
  deallog << "dim " << dim << " " << fe.get_name() << std::endl;

  Triangulation<dim> triangulation;
  GridGenerator::hyper_cube(triangulation, -0.3, 0.7);
  triangulation.refine_global(dim == 2 ? 2 : 1);
  if (distort_mesh)
    GridTools::distort_random(0.03, triangulation);

  MappingQ<dim> mapping(order_mapping);

  DoFHandler<dim> dof_handler(triangulation);
  dof_handler.distribute_dofs(fe);

  Vector<double> interpolant(dof_handler.n_dofs());
  VectorTools::interpolate(mapping, dof_handler, f, interpolant);

  // Check that VectorTools::interpolate is in fact a
  // projection, i.e. applying the interpolation twice results in the same
  // vector:

  Functions::FEFieldFunction<dim> f2(dof_handler, interpolant, mapping);

  Vector<double> interpolant2(dof_handler.n_dofs());
  VectorTools::interpolate(
    mapping, dof_handler, f2, interpolant2);

  interpolant2 -= interpolant;
  deallog << "Check projection property: " << interpolant2.linfty_norm()
          << std::endl;
}


int main ()
{
  deallog.depth_console(3);

  test<2>(FE_RaviartThomas<2>(0), F<2>(1), 2, 1, false);
  test<2>(FE_RaviartThomas<2>(1), F<2>(0), 2, 2, false);
  test<2>(FE_RaviartThomas<2>(1), F<2>(2), 2, 2, false);

  test<3>(FE_RaviartThomas<3>(0), F<3>(0), 3, 1, false);
  test<3>(FE_RaviartThomas<3>(1), F<3>(0), 3, 2, false);
  test<3>(FE_RaviartThomas<3>(1), F<3>(2), 3, 2, false);
}
