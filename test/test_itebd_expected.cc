// -*- mode: c++; fill-column: 80; c-basic-offset: 2; indent-tabs-mode: nil -*-
/*
    Copyright (c) 2010 Juan Jose Garcia Ripoll

    Tensor is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published
    by the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "loops.h"
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <mps/quantum.h>
#include <mps/itebd.h>

namespace tensor_test {

  using namespace mps;
  using namespace tensor;

  template<class Tensor>
  const iTEBD<Tensor> random_product(int d, bool same = false, Tensor *pA=0, Tensor *pB=0)
  {
      Tensor A = Tensor::random(d);
      Tensor B = same? A : Tensor::random(d);
      Tensor nA = A / norm2(A);
      Tensor nB = B / norm2(B);
      if (pA) *pA = nA;
      if (pB) *pB = nB;
      return iTEBD<Tensor>(A, B);
  }

  template<class Tensor>
  void test_expected_product_norm(int d)
  {
    for (int i = 1; i < 100; i++) {
      Tensor id = Tensor::eye(d,d);
      typename Tensor::elt_t one = id(0,0);
      iTEBD<Tensor> psiAA = random_product<Tensor>(d, true);
      iTEBD<Tensor> psiAB = random_product<Tensor>(d, false);
      /* The expected value of the identity is the norm */
      EXPECT_TRUE(simeq(one, psiAA.expected_value(id)));
      EXPECT_TRUE(simeq(one, psiAB.expected_value(id)));
      /* Test translational invariance */
      EXPECT_CEQ(psiAA.expected_value(id, 0), psiAA.expected_value(id, 1));
      EXPECT_CEQ(psiAA.expected_value(id, 0), psiAA.expected_value(id, 2));
      EXPECT_CEQ(psiAA.expected_value(id, 1), psiAA.expected_value(id, -1));
      /* Test translational invariance */
      EXPECT_CEQ(psiAB.expected_value(id, 0), psiAA.expected_value(id, 2));
      EXPECT_CEQ(psiAB.expected_value(id, 1), psiAA.expected_value(id, 3));
      EXPECT_CEQ(psiAB.expected_value(id, 1), psiAA.expected_value(id, -1));
    }
  }

  template<class Tensor>
  Tensor projector(const Tensor &A)
  {
    const Tensor Am = reshape(A, A.size(), 1);
    return mmult(Am, adjoint(Am));
  }

  /*
   * When computing correlations, ensure that operators act on the right
   * sites and that expectation values are translationally invariant with
   * period two.
   */
  template<class Tensor>
  void test_expected_projectors(int d)
  {
    for (int i = 1; i < 100; i++) {
      Tensor A, B;
      iTEBD<Tensor> psi = random_product<Tensor>(d, true, &A, &B);
      Tensor PA = projector(A); // Projector onto A
      Tensor PB = projector(B); // Projector onto B
      Tensor id = Tensor::eye(d,d);
      Tensor PnA = id - PA; // Orthogonal projectors
      Tensor PnB = id - PB;
      typename Tensor::elt_t one = number_one<typename Tensor::elt_t>();
      typename Tensor::elt_t zero = number_zero<typename Tensor::elt_t>();

      EXPECT_CEQ(one, psi.expected_value(PA, 0));
      EXPECT_CEQ(zero, psi.expected_value(PnA, 0));
      EXPECT_CEQ(one, psi.expected_value(PB, 1));
      EXPECT_CEQ(zero, psi.expected_value(PnB, 1));

      EXPECT_CEQ(one, psi.expected_value(PA, PB));
      EXPECT_CEQ(zero, psi.expected_value(PA, PnB));
      EXPECT_CEQ(zero, psi.expected_value(PnA, PB));
      EXPECT_CEQ(zero, psi.expected_value(PnA, PnB));

      EXPECT_CEQ(one, psi.expected_value(PA, id));
      EXPECT_CEQ(one, psi.expected_value(id, PB));
      EXPECT_CEQ(zero, psi.expected_value(id, PnB));
      EXPECT_CEQ(zero, psi.expected_value(PnA, id));

      EXPECT_CEQ(one, psi.expected_value(PA, 0));
      EXPECT_CEQ(zero, psi.expected_value(PnA, 0));
      EXPECT_CEQ(one, psi.expected_value(PB, 1));
      EXPECT_CEQ(zero, psi.expected_value(PnB, 1));
    }
  }

  /*
   * Verify the implementation of expected12() by checking with kronecker
   * products of projectors.
   */
  template<class Tensor>
  void test_expected12_projectors(int d)
  {
    for (int i = 1; i < 100; i++) {
      Tensor A, B;
      iTEBD<Tensor> psi = random_product<Tensor>(d, false, &A, &B);
      Tensor PA = projector(A); // Projector onto A
      Tensor PB = projector(B); // Projector onto B
      Tensor id = Tensor::eye(d,d);
      Tensor PnA = id - PA; // Orthogonal projectors
      Tensor PnB = id - PB;
      typename Tensor::elt_t one = number_one<typename Tensor::elt_t>();
      typename Tensor::elt_t zero = number_zero<typename Tensor::elt_t>();

      EXPECT_CEQ(one, psi.expected_value12(kron2(PA, PB)));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PA, PnB)));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PnA, PB)));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PnA, PnB)));

      EXPECT_CEQ(one, psi.expected_value12(kron2(PA, id)));
      EXPECT_CEQ(one, psi.expected_value12(kron2(id, PB)));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(id, PnB)));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PnA, id)));

      EXPECT_CEQ(one, psi.expected_value12(kron2(PB, PA), 1));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PnB, PA), 1));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PB, PnA), 1));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PnB, PnA), 1));

      EXPECT_CEQ(one, psi.expected_value12(kron2(id, PA), 1));
      EXPECT_CEQ(one, psi.expected_value12(kron2(PB, id), 1));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(PnB, id), 1));
      EXPECT_CEQ(zero, psi.expected_value12(kron2(id, PnA), 1));
    }
  }

  /*
   * Verify the implementation of expected12() by checking with kronecker
   * products of projectors.
   */
  template<class Tensor>
  void test_energy_projectors(int d)
  {
    for (int i = 1; i < 100; i++) {
      Tensor A, B;
      iTEBD<Tensor> psi = random_product<Tensor>(d, true, &A, &B);
      Tensor PA = projector(A); // Projector onto A
      Tensor id = Tensor::eye(d,d);
      Tensor PnA = id - PA; // Orthogonal projectors
      typename Tensor::elt_t one = number_one<typename Tensor::elt_t>();
      typename Tensor::elt_t zero = number_zero<typename Tensor::elt_t>();

      EXPECT_CEQ(2.0, psi.energy(kron(PA, PA)));
      EXPECT_CEQ(0.0, psi.energy(kron(PA, PnA)));
      EXPECT_CEQ(0.0, psi.energy(kron(PnA, PA)));
      EXPECT_CEQ(0.0, psi.energy(kron(PnA, PnA)));

      EXPECT_CEQ(2.0, psi.energy(kron(PA, id)));
      EXPECT_CEQ(2.0, psi.energy(kron(id, PA)));
      EXPECT_CEQ(0.0, psi.energy(kron(id, PnA)));
      EXPECT_CEQ(0.0, psi.energy(kron(PnA, id)));
    }
  }

  ////////////////////////////////////////////////////////////
  /// ITEBD WITH REAL TENSORS
  ///

  TEST(RiTEBDTest, RiTEBDNormProduct) {
    test_over_integers(1, 6, test_expected_product_norm<RTensor>);
  }

  TEST(RiTEBDTest, RiTEBDExpectedProjectors) {
    test_over_integers(1, 6, test_expected_projectors<RTensor>);
  }

  TEST(RiTEBDTest, RiTEBDExpected12Projectors) {
    test_over_integers(1, 6, test_expected12_projectors<RTensor>);
  }

  TEST(RiTEBDTest, RiTEBDEnergyProjectors) {
    test_over_integers(1, 6, test_energy_projectors<RTensor>);
  }

  ////////////////////////////////////////////////////////////
  /// ITEBD WITH COMPLEX TENSORS
  ///

  TEST(CiTEBDTest, CiTEBDNormProduct) {
    test_over_integers(1, 6, test_expected_product_norm<CTensor>);
  }

  TEST(CiTEBDTest, CiTEBDExpectedProjectors) {
    test_over_integers(1, 6, test_expected_projectors<CTensor>);
  }

  TEST(CiTEBDTest, CiTEBDExpected12Projectors) {
    test_over_integers(1, 6, test_expected12_projectors<CTensor>);
  }

  TEST(CiTEBDTest, CiTEBDEnergyProjectors) {
    test_over_integers(1, 6, test_energy_projectors<CTensor>);
  }

}