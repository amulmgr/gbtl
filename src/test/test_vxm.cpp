/*
 * GraphBLAS Template Library (GBTL), Version 3.0
 *
 * Copyright 2020 Carnegie Mellon University, Battelle Memorial Institute, and
 * Authors.
 *
 * THIS MATERIAL WAS PREPARED AS AN ACCOUNT OF WORK SPONSORED BY AN AGENCY OF
 * THE UNITED STATES GOVERNMENT.  NEITHER THE UNITED STATES GOVERNMENT NOR THE
 * UNITED STATES DEPARTMENT OF ENERGY, NOR THE UNITED STATES DEPARTMENT OF
 * DEFENSE, NOR CARNEGIE MELLON UNIVERSITY, NOR BATTELLE, NOR ANY OF THEIR
 * EMPLOYEES, NOR ANY JURISDICTION OR ORGANIZATION THAT HAS COOPERATED IN THE
 * DEVELOPMENT OF THESE MATERIALS, MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR
 * ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS,
 * OR USEFULNESS OR ANY INFORMATION, APPARATUS, PRODUCT, SOFTWARE, OR PROCESS
 * DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY OWNED
 * RIGHTS.
 *
 * Released under a BSD-style license, please see LICENSE file or contact
 * permission@sei.cmu.edu for full terms.
 *
 * [DISTRIBUTION STATEMENT A] This material has been approved for public release
 * and unlimited distribution.  Please see Copyright notice for non-US
 * Government use and distribution.
 *
 * This Software includes and/or makes use of the following Third-Party Software
 * subject to its own license:
 *
 * 1. Boost Unit Test Framework
 * (https://www.boost.org/doc/libs/1_45_0/libs/test/doc/html/utf.html)
 * Copyright 2001 Boost software license, Gennadiy Rozental.
 *
 * DM20-0442
 */

#include <graphblas/graphblas.hpp>

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE vxm_test_suite

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(BOOST_TEST_MODULE)

//****************************************************************************

namespace
{
    std::vector<std::vector<double> > m3x3_dense = {{12, 0, 7},
                                                    {1,  0, 0},
                                                    {3,  6, 9}};

    std::vector<std::vector<double> > m4x3_dense = {{5, 6, 4},
                                                    {0, 7, 5},
                                                    {1, 0, 0},
                                                    {2, 0, 1}};
    std::vector<double> u3_dense = {1, 1, 0};
    std::vector<double> u4_dense = {0, 0, 1, 1};

    std::vector<double> ans3_dense = {13, 0, 7};
    std::vector<double> ans4_dense = { 3, 0, 1};

    static std::vector<double> v4_ones = {1, 1, 1, 1};
    static std::vector<double> v3_ones = {1, 1, 1};
}

//****************************************************************************
// Tests without mask
//****************************************************************************

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_bad_dimensions)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);
    grb::Matrix<double, grb::DirectedMatrixTag> mB(m4x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);

    grb::Vector<double, grb::SparseTag> w4(4);
    grb::Vector<double, grb::SparseTag> w3(3);

    // incompatible input dimensions
    BOOST_CHECK_THROW(
        (grb::vxm(w3,
                  grb::NoMask(),
                  grb::NoAccumulate(),
                  grb::ArithmeticSemiring<double>(), u3, mB)),
        grb::DimensionException);

    // incompatible output matrix dimensions
    BOOST_CHECK_THROW(
        (grb::vxm(w4,
                  grb::NoMask(),
                  grb::NoAccumulate(),
                  grb::ArithmeticSemiring<double>(), u3, mA)),
        grb::DimensionException);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_reg)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);
    grb::Matrix<double, grb::DirectedMatrixTag> mB(m4x3_dense, 0.);
    grb::Vector<double> u3(u3_dense, 0.);
    grb::Vector<double> u4(u4_dense, 0.);
    grb::Vector<double> result(3);
    grb::Vector<double> ansA(ans3_dense, 0.);
    grb::Vector<double> ansB(ans4_dense, 0.);

    grb::vxm(result,
             grb::NoMask(),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u3, mA);
    BOOST_CHECK_EQUAL(result, ansA);

    grb::vxm(result,
             grb::NoMask(),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u4, mB);
    BOOST_CHECK_EQUAL(result, ansB);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_stored_zero_result)
{
    // Build some matrices.
    std::vector<std::vector<int> > A = {{1, 1, 0, 0},
                                        {1, 2, 2, 0},
                                        {0, 2, 3, 3},
                                        {0, 0, 3, 4}};
    std::vector<int> u4_dense =  { 1,-1, 0, 0};
    grb::Matrix<int, grb::DirectedMatrixTag> mA(A, 0);
    grb::Vector<int> u(u4_dense, 0);
    grb::Vector<int> result(4);

    // use a different sentinel value so that stored zeros are preserved.
    int const NIL(666);
    std::vector<int> ans = {  0,  -1, -2, NIL};
    grb::Vector<int> answer(ans, NIL);

    grb::vxm(result,
             grb::NoMask(),
             grb::NoAccumulate(), // Second<int>(),
             grb::ArithmeticSemiring<int>(), u, mA);
    BOOST_CHECK_EQUAL(result, answer);
    BOOST_CHECK_EQUAL(result.nvals(), 3);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_a_transpose)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);
    grb::Matrix<double, grb::DirectedMatrixTag> mB(m4x3_dense, 0.);
    grb::Vector<double> u3(u3_dense, 0.);
    grb::Vector<double> u4(u4_dense, 0.);
    grb::Vector<double> result3(3);
    grb::Vector<double> result4(4);

    std::vector<double> ansAT_dense = {12, 1, 9};
    std::vector<double> ansBT_dense = {11, 7, 1, 2};

    grb::Vector<double> ansA(ansAT_dense, 0.);
    grb::Vector<double> ansB(ansBT_dense, 0.);

    grb::vxm(result3,
             grb::NoMask(),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(),
             u3, transpose(mA));
    BOOST_CHECK_EQUAL(result3, ansA);

    BOOST_CHECK_THROW(
        (grb::vxm(result4,
                  grb::NoMask(),
                  grb::NoAccumulate(),
                  grb::ArithmeticSemiring<double>(),
                  u4, transpose(mB))),
        grb::DimensionException);

    grb::vxm(result4,
             grb::NoMask(),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(),
             u3, transpose(mB));
    BOOST_CHECK_EQUAL(result4, ansB);
}

//****************************************************************************
// Tests using a mask with REPLACE
//****************************************************************************

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_replace_bad_dimensions)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m4(u4_dense, 0.);
    grb::Vector<double, grb::SparseTag> w3(3);

    // incompatible mask matrix dimensions
    BOOST_CHECK_THROW(
        (grb::vxm(w3,
                  m4,
                  grb::NoAccumulate(),
                  grb::ArithmeticSemiring<double>(), u3, mA,
                  grb::REPLACE)),
        grb::DimensionException);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_replace_reg)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m3(u3_dense, 0.);
    BOOST_CHECK_EQUAL(m3.nvals(), 2);

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 1, 0};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 m3,
                 grb::Second<double>(),
                 grb::ArithmeticSemiring<double>(), u3, mA,
                 grb::REPLACE);

        BOOST_CHECK_EQUAL(result.nvals(), 2);
        BOOST_CHECK_EQUAL(result, answer);
    }

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 0, 0};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 m3,
                 grb::NoAccumulate(),
                 grb::ArithmeticSemiring<double>(), u3, mA,
                 grb::REPLACE);

        BOOST_CHECK_EQUAL(result.nvals(), 1);
        BOOST_CHECK_EQUAL(result, answer);
    }
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_replace_reg_stored_zero)
{
    // tests a computed and stored zero
    // tests a stored zero in the mask

    // Build some matrices.
    std::vector<std::vector<int> > A_dense = {{1, 1, 0, 0},
                                              {1, 2, 2, 0},
                                              {0, 2, 3, 3},
                                              {0, 0, 3, 4}};
    std::vector<int> u4_dense =  { 1,-1, 1, 0};
    std::vector<int> mask_dense =  { 1, 1, 1, 0};
    std::vector<int> ones_dense =  { 1, 1, 1, 1};
    grb::Matrix<int, grb::DirectedMatrixTag> A(A_dense, 0);
    grb::Vector<int> u(u4_dense, 0);
    grb::Vector<int> mask(mask_dense, 0);
    grb::Vector<int> result(ones_dense, 0);

    BOOST_CHECK_EQUAL(mask.nvals(), 3);  // [ 1 1 1 - ]
    mask.setElement(3, 0);
    BOOST_CHECK_EQUAL(mask.nvals(), 4);  // [ 1 1 1 0 ]

    int const NIL(666);
    std::vector<int> ans = { 0, 1, 1,  NIL};
    grb::Vector<int> answer(ans, NIL);

    grb::vxm(result,
             mask,
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u, A,
             grb::REPLACE);
    BOOST_CHECK_EQUAL(result.nvals(), 3);
    BOOST_CHECK_EQUAL(result, answer);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_replace_a_transpose)
{
    // Build some matrices.
    std::vector<std::vector<int> > A_dense = {{1, 1, 0, 0},
                                              {1, 2, 2, 0},
                                              {0, 2, 3, 3},
                                              {0, 0, 3, 4},
                                              {1, 1, 1, 1}};
    std::vector<int> u4_dense =  { 1,-1, 1, 0};
    std::vector<int> mask_dense =  { 1, 0, 1, 0, 1};
    std::vector<int> ones_dense =  { 1, 1, 1, 1, 1};
    grb::Matrix<int, grb::DirectedMatrixTag> A(A_dense, 0);
    grb::Vector<int> u(u4_dense, 0);
    grb::Vector<int> mask(mask_dense, 0);
    grb::Vector<int> result(ones_dense, 0);

    int const NIL(666);
    std::vector<int> ans = { 0, NIL, 1,  NIL, 1};
    grb::Vector<int> answer(ans, NIL);

    grb::vxm(result,
             mask,
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u, transpose(A),
             grb::REPLACE);

    BOOST_CHECK_EQUAL(result, answer);
}

//****************************************************************************
// Tests using a mask with MERGE semantics
//****************************************************************************

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_bad_dimensions)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m4(u4_dense, 0.);
    grb::Vector<double, grb::SparseTag> w3(v3_ones, 0.);

    // incompatible mask matrix dimensions
    BOOST_CHECK_THROW(
        (grb::vxm(w3,
                  m4,
                  grb::NoAccumulate(),
                  grb::ArithmeticSemiring<double>(), u3, mA)),
        grb::DimensionException);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_reg)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m3(u3_dense, 0.); // [1 1 -]
    BOOST_CHECK_EQUAL(m3.nvals(), 2);

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 1, 1};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 m3,
                 grb::Second<double>(),
                 grb::ArithmeticSemiring<double>(), u3, mA);

        BOOST_CHECK_EQUAL(result.nvals(), 3);
        BOOST_CHECK_EQUAL(result, answer);
    }

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 0, 1};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 m3,
                 grb::NoAccumulate(),
                 grb::ArithmeticSemiring<double>(), u3, mA);

        BOOST_CHECK_EQUAL(result.nvals(), 2);
        BOOST_CHECK_EQUAL(result, answer);
    }
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_reg_stored_zero)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m3(u3_dense); // [1 1 0]
    BOOST_CHECK_EQUAL(m3.nvals(), 3);

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 1, 1};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 m3,
                 grb::Second<double>(),
                 grb::ArithmeticSemiring<double>(), u3, mA);

        BOOST_CHECK_EQUAL(result.nvals(), 3);
        BOOST_CHECK_EQUAL(result, answer);
    }

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 0, 1};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 m3,
                 grb::NoAccumulate(),
                 grb::ArithmeticSemiring<double>(), u3, mA);

        BOOST_CHECK_EQUAL(result.nvals(), 2);
        BOOST_CHECK_EQUAL(result, answer);
    }
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_masked_a_transpose)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.); // [1 1 -]
    grb::Vector<double, grb::SparseTag> m3(u3_dense, 0.); // [1 1 -]
    grb::Vector<double> result(v3_ones, 0.);

    BOOST_CHECK_EQUAL(m3.nvals(), 2);

    std::vector<double> ans = {12, 1, 1};
    grb::Vector<double> answer(ans, 0.);

    grb::vxm(result,
             m3,
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u3, transpose(mA));

    BOOST_CHECK_EQUAL(result.nvals(), 3);
    BOOST_CHECK_EQUAL(result, answer);
}

//****************************************************************************
// Tests using a complemented mask with REPLACE semantics
//****************************************************************************

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_replace_bad_dimensions)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m4(u4_dense, 0.);
    grb::Vector<double, grb::SparseTag> w3(3);

    // incompatible mask matrix dimensions
    BOOST_CHECK_THROW(
        (grb::vxm(w3,
                  grb::complement(m4),
                  grb::NoAccumulate(),
                  grb::ArithmeticSemiring<double>(), u3, mA,
                  grb::REPLACE)),
        grb::DimensionException);
}
//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_replace_reg)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);

    std::vector<double> scmp_mask_dense = {0., 0., 1.};
    grb::Vector<double, grb::SparseTag> m3(scmp_mask_dense, 0.);
    BOOST_CHECK_EQUAL(m3.nvals(), 1);

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 1, 0};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 grb::complement(m3),
                 grb::Second<double>(),
                 grb::ArithmeticSemiring<double>(), u3, mA,
                 grb::REPLACE);

        BOOST_CHECK_EQUAL(result.nvals(), 2);
        BOOST_CHECK_EQUAL(result, answer);
    }

    {
        grb::Vector<double> result(v3_ones, 0.);

        std::vector<double> ans = {13, 0, 0};
        grb::Vector<double> answer(ans, 0.);

        grb::vxm(result,
                 grb::complement(m3),
                 grb::NoAccumulate(),
                 grb::ArithmeticSemiring<double>(), u3, mA,
                 grb::REPLACE);

        BOOST_CHECK_EQUAL(result.nvals(), 1);
        BOOST_CHECK_EQUAL(result, answer);
    }
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_replace_reg_stored_zero)
{
    // tests a computed and stored zero
    // tests a stored zero in the mask
    // Build some matrices.
    std::vector<std::vector<int> > A_dense = {{1, 1, 0, 0},
                                              {1, 2, 2, 0},
                                              {0, 2, 3, 3},
                                              {0, 0, 3, 4}};
    std::vector<int> u4_dense =  { 1,-1, 1, 0};
    std::vector<int> scmp_mask_dense =  { 0, 0, 0, 1 };
    std::vector<int> ones_dense =  { 1, 1, 1, 1};
    grb::Matrix<int, grb::DirectedMatrixTag> A(A_dense, 0);
    grb::Vector<int> u(u4_dense, 0);
    grb::Vector<int> mask(scmp_mask_dense); //, 0);
    grb::Vector<int> result(ones_dense, 0);

    BOOST_CHECK_EQUAL(mask.nvals(), 4);  // [ 0 0 0 1 ]

    int const NIL(666);
    std::vector<int> ans = { 0, 1, 1,  NIL};
    grb::Vector<int> answer(ans, NIL);

    grb::vxm(result,
             grb::complement(mask),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u, A,
             grb::REPLACE);
    BOOST_CHECK_EQUAL(result.nvals(), 3);
    BOOST_CHECK_EQUAL(result, answer);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_replace_a_transpose)
{
    // Build some matrices.
    std::vector<std::vector<int> > A_dense = {{1, 1, 0, 0},
                                              {1, 2, 2, 0},
                                              {0, 2, 3, 3},
                                              {0, 0, 3, 4},
                                              {1, 1, 1, 1}};
    std::vector<int> u4_dense =  { 1,-1, 1, 0};
    std::vector<int> mask_dense =  { 0, 1, 0, 1, 0};
    std::vector<int> ones_dense =  { 1, 1, 1, 1, 1};
    grb::Matrix<int, grb::DirectedMatrixTag> A(A_dense, 0);
    grb::Vector<int> u(u4_dense, 0);
    grb::Vector<int> mask(mask_dense, 0);
    grb::Vector<int> result(ones_dense, 0);

    int const NIL(666);
    std::vector<int> ans = { 0, NIL, 1,  NIL, 1};
    grb::Vector<int> answer(ans, NIL);

    grb::vxm(result,
             grb::complement(mask),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u, transpose(A),
             grb::REPLACE);

    BOOST_CHECK_EQUAL(result, answer);
}

//****************************************************************************
// Tests using a complemented mask (with merge semantics)
//****************************************************************************

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_bad_dimensions)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m4(u4_dense, 0.);
    grb::Vector<double, grb::SparseTag> w3(v3_ones, 0.);

    // incompatible mask matrix dimensions
    BOOST_CHECK_THROW(
        (grb::vxm(w3,
                  grb::complement(m4),
                  grb::NoAccumulate(),
                  grb::ArithmeticSemiring<double>(), u3, mA)),
        grb::DimensionException);
}
//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_reg)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m3(u3_dense, 0.); // [1 1 -]
    grb::Vector<double> result(v3_ones, 0.);

    BOOST_CHECK_EQUAL(m3.nvals(), 2);

    std::vector<double> ans = {1, 1, 7}; // if accum is NULL then {1, 0, 7};
    grb::Vector<double> answer(ans, 0.);

    grb::vxm(result,
             grb::complement(m3),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u3, mA);

    BOOST_CHECK_EQUAL(result.nvals(), 3); //2);
    BOOST_CHECK_EQUAL(result, answer);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_reg_stored_zero)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.);
    grb::Vector<double, grb::SparseTag> m3(u3_dense); // [1 1 0]
    grb::Vector<double> result(v3_ones, 0.);

    BOOST_CHECK_EQUAL(m3.nvals(), 3);

    std::vector<double> ans = {1, 1, 7}; // if accum is NULL then {1, 0, 7};
    grb::Vector<double> answer(ans, 0.);

    grb::vxm(result,
             grb::complement(m3),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u3, mA);

    BOOST_CHECK_EQUAL(result.nvals(), 3); //2);
    BOOST_CHECK_EQUAL(result, answer);
}

//****************************************************************************
BOOST_AUTO_TEST_CASE(test_vxm_scmp_masked_a_transpose)
{
    grb::Matrix<double, grb::DirectedMatrixTag> mA(m3x3_dense, 0.);

    grb::Vector<double, grb::SparseTag> u3(u3_dense, 0.); // [1 1 -]
    grb::Vector<double, grb::SparseTag> m3(u3_dense, 0.); // [1 1 -]
    grb::Vector<double> result(v3_ones, 0.);

    BOOST_CHECK_EQUAL(m3.nvals(), 2);

    std::vector<double> ans = {1, 1, 9};
    grb::Vector<double> answer(ans, 0.);

    grb::vxm(result,
             grb::complement(m3),
             grb::NoAccumulate(),
             grb::ArithmeticSemiring<double>(), u3, transpose(mA));

    BOOST_CHECK_EQUAL(result.nvals(), 3);
    BOOST_CHECK_EQUAL(result, answer);
}


BOOST_AUTO_TEST_SUITE_END()
