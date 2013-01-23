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

#include <tensor/tensor.h>

namespace tensor_test {

  Tensor<double> random_permutation(int n, int iterations)
  {
    RSparse output = RTensor::eye(n);
    if (n > 1) {
      if (iterations <= 0)
	iterations = 2*n;
      Indices rows = iota(0, n-1);
      Tensor<double> diagonal = ones(n);
      while (iterations--) {
	Indices columns = rows;
	int i = rand<int>(0, n);
	int j = (i + rand<int>(1, n)) % n;
	output = mmult(RSparse(rows, columns, diag), output);
      }
    }
    return output;
  }

} // namespace tensor_test
