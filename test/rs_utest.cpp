/*
 * Copyright 2017-2018 Scality
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <gtest/gtest.h>

#include "quadiron.h"

namespace gf = quadiron::gf;
namespace vec = quadiron::vec;

// See http://web.eecs.utk.edu/~plank/plank/papers/CS-03-504.html.
TEST(RsTest, TestVandermonde) // NOLINT
{
    const auto gf16(gf::create<gf::BinExtension<uint32_t>>(4));
    vec::Matrix<uint32_t> mat(gf16, 3, 3);

    mat.vandermonde_suitable_for_ec();

    ASSERT_EQ(mat.get(0, 0), 1);
    ASSERT_EQ(mat.get(0, 1), 1);
    ASSERT_EQ(mat.get(0, 2), 1);
    ASSERT_EQ(mat.get(1, 0), 15);
    ASSERT_EQ(mat.get(1, 1), 8);
    ASSERT_EQ(mat.get(1, 2), 6);
    ASSERT_EQ(mat.get(2, 0), 14);
    ASSERT_EQ(mat.get(2, 1), 9);
    ASSERT_EQ(mat.get(2, 2), 6);
}

/*
 * Example taken from
 * An Introduction to Galois Fields and Reed-Solomon Coding
 * James Westall James Martin
 *
 * Note: the Vandermonde matrix is not generated the same way as we do
 */
TEST(RsTest, TestMultiplication) // NOLINT
{
    const auto gf8(gf::create<gf::BinExtension<uint32_t>>(3));
    vec::Matrix<uint32_t> mat(gf8, 5, 3);
    vec::Vector<uint32_t> vec(gf8, 3);
    vec::Vector<uint32_t> output(gf8, 5);

    mat.set(0, 0, 1);
    mat.set(0, 1, 1);
    mat.set(0, 2, 6);
    mat.set(1, 0, 4);
    mat.set(1, 1, 3);
    mat.set(1, 2, 2);
    mat.set(2, 0, 5);
    mat.set(2, 1, 2);
    mat.set(2, 2, 2);
    mat.set(3, 0, 5);
    mat.set(3, 1, 3);
    mat.set(3, 2, 4);
    mat.set(4, 0, 4);
    mat.set(4, 1, 2);
    mat.set(4, 2, 4);

    vec.set(0, 4);
    vec.set(1, 5);
    vec.set(2, 6);

    mat.mul(&output, &vec);

    ASSERT_EQ(output.get(0), 3);
    ASSERT_EQ(output.get(1), 5);
    ASSERT_EQ(output.get(2), 4);
    ASSERT_EQ(output.get(3), 3);
    ASSERT_EQ(output.get(4), 2);
}

TEST(RsTest, TestGf256) // NOLINT
{
    auto gf256(gf::create<gf::BinExtension<uint32_t>>(8));

    ASSERT_EQ(gf256.mul(3, 7), 9);
    ASSERT_EQ(gf256.mul(13, 10), 114);
    ASSERT_EQ(gf256.div(13, 10), 40);
    ASSERT_EQ(gf256.div(3, 7), 211);
}
