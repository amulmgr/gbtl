/*
 * GraphBLAS Template Library, Version 2.0
 *
 * Copyright 2018 Carnegie Mellon University, Battelle Memorial Institute, and
 * Authors. All Rights Reserved.
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
 * RIGHTS..
 *
 * Released under a BSD (SEI)-style license, please see license.txt or contact
 * permission@sei.cmu.edu for full terms.
 *
 * This release is an update of:
 *
 * 1. GraphBLAS Template Library (GBTL)
 * (https://github.com/cmu-sei/gbtl/blob/1.0.0/LICENSE) Copyright 2015 Carnegie
 * Mellon University and The Trustees of Indiana. DM17-0037, DM-0002659
 *
 * DM18-0559
 */

#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <iterator>
#include <iostream>
#include <chrono>

#include <graphblas/detail/logging.h>
#include <graphblas/types.hpp>
#include <graphblas/algebra.hpp>

#include "sparse_helpers.hpp"
#include "LilSparseMatrix.hpp"


//****************************************************************************

namespace GraphBLAS
{
    namespace backend
    {

        //**********************************************************************
        //**********************************************************************

        //**********************************************************************
        // Perform C = A*B where C, A and B must all be unique
        template<typename TScalarT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void ATB_NoMask_kernel(
            LilSparseMatrix<TScalarT>       &T,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B)
        {
            for (IndexType k = 0; k < A.nrows(); ++k)
            {
                if (A[k].empty() || B[k].empty()) continue;

                for (auto const &Ak_elt : A[k])
                {
                    IndexType    i(std::get<0>(Ak_elt));
                    AScalarT  a_ki(std::get<1>(Ak_elt));

                    // T[i] += (a_ki*B[k])  // must reduce in D3, hence T.
                    axpy(T[i], semiring, a_ki, B[k]);
                }
            }
        }

        //**********************************************************************
        // Perform C = A*B where C, A and B must all be unique
        template<typename TScalarT,
                 typename MScalarT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void ATB_Mask_kernel(
            LilSparseMatrix<TScalarT>       &T,
            LilSparseMatrix<MScalarT> const &M,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B)
        {
            for (IndexType k = 0; k < A.nrows(); ++k)
            {
                if (A[k].empty() || B[k].empty()) continue;

                for (auto const &Ak_elt : A[k])
                {
                    IndexType    i(std::get<0>(Ak_elt));
                    AScalarT  a_ki(std::get<1>(Ak_elt));

                    if (M[i].empty()) continue;

                    // T[i] += M[i] .* (a_ki*B[k])  // must reduce in D3, hence T.
                    masked_axpy(T[i], M[i], false, semiring, a_ki, B[k]);
                }
            }
        }

        //**********************************************************************
        // Perform C = A*B where C, A and B must all be unique
        template<typename TScalarT,
                 typename MScalarT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void ATB_CompMask_kernel(
            LilSparseMatrix<TScalarT>       &T,
            LilSparseMatrix<MScalarT> const &M,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B)
        {
            for (IndexType k = 0; k < A.nrows(); ++k)
            {
                if (A[k].empty() || B[k].empty()) continue;

                for (auto const &Ak_elt : A[k])
                {
                    IndexType    i(std::get<0>(Ak_elt));
                    AScalarT  a_ki(std::get<1>(Ak_elt));

                    // T[i] += !M[i] .* (a_ki*B[k])  // must reduce in D3, hence T.
                    masked_axpy(T[i], M[i], true, semiring, a_ki, B[k]);
                }
            }
        }

        //**********************************************************************
        //**********************************************************************
        //**********************************************************************

        //**********************************************************************
        template<typename CScalarT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void sparse_mxm_NoMask_NoAccum_ATB(
            LilSparseMatrix<CScalarT>       &C,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B)
        {
            // C = A +.* B
            // short circuit conditions
            if ((A.nvals() == 0) || (B.nvals() == 0))
            {
                C.clear();
                return;
            }

            // =================================================================
            typedef typename SemiringT::result_type D3ScalarType;
            LilSparseMatrix<D3ScalarType> T(C.nrows(), C.ncols());

            ATB_NoMask_kernel(T, semiring, A, B);

            for (IndexType i = 0; i < C.nrows(); ++i)
            {
                // C[i] = T[i]
                C.setRow(i, T[i]);
            }

            GRB_LOG_VERBOSE("C: " << C);
        }

        //**********************************************************************
        template<typename CScalarT,
                 typename AccumT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void sparse_mxm_NoMask_Accum_ATB(
            LilSparseMatrix<CScalarT>       &C,
            AccumT                    const &accum,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B)
        {
            // C = C + (A +.* B)
            // short circuit conditions
            if ((A.nvals() == 0) || (B.nvals() == 0))
            {
                return; // do nothing
            }

            // =================================================================

            typedef typename SemiringT::result_type D3ScalarType;
            LilSparseMatrix<D3ScalarType> T(C.nrows(), C.ncols());

            ATB_NoMask_kernel(T, semiring, A, B);

            for (IndexType i = 0; i < C.nrows(); ++i)
            {
                if (!T[i].empty())
                    // C[i] = C[i] + T[i]
                    C.mergeRow(i, T[i], accum);
            }

            GRB_LOG_VERBOSE("C: " << C);
        }

        //**********************************************************************
        template<typename CScalarT,
                 typename MScalarT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void sparse_mxm_Mask_NoAccum_ATB(
            LilSparseMatrix<CScalarT>       &C,
            LilSparseMatrix<MScalarT> const &M,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B,
            OutputControlEnum                outp)
        {
            // C<M,z> = A' +.* B
            //        =               [M .* (A' +.* B)], z = "replace"
            //        = [!M .* C]  U  [M .* (A' +.* B)], z = "merge"
            // short circuit conditions
            if ((outp == REPLACE) &&
                ((A.nvals() == 0) || (B.nvals() == 0) || (M.nvals() == 0)))
            {
                C.clear();
                return;
            }
            else if (!(outp == REPLACE) && (M.nvals() == 0))
            {
                return; // do nothing
            }

            // =================================================================

            typedef typename SemiringT::result_type D3ScalarType;
            LilSparseMatrix<D3ScalarType> T(C.nrows(), C.ncols());
            typename LilSparseMatrix<CScalarT>::RowType C_row;

            ATB_Mask_kernel(T, M, semiring, A, B);

            if (!(outp == REPLACE))
            {
                for (IndexType i = 0; i < C.nrows(); ++i)
                {
                    // C[i] = (!M[i] .* C[i])  U  T[i], z = "merge"
                    C_row.clear();
                    masked_merge(C_row, M[i], false, C[i], T[i]);
                    C.setRow(i, C_row);
                }
            }
            else
            {
                for (IndexType i = 0; i < C.nrows(); ++i)
                {
                    C.setRow(i, T[i]);
                }
            }

            GRB_LOG_VERBOSE("C: " << C);
        }

        //**********************************************************************
        template<typename CScalarT,
                 typename MScalarT,
                 typename AccumT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void sparse_mxm_Mask_Accum_ATB(
            LilSparseMatrix<CScalarT>       &C,
            LilSparseMatrix<MScalarT> const &M,
            AccumT                    const &accum,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B,
            OutputControlEnum                outp)
        {
            // C<M,z> = C + (A +.* B)
            //        =               [M .* [C + (A +.* B)]], z = "replace"
            //        = [!M .* C]  U  [M .* [C + (A +.* B)]], z = "merge"
            // short circuit conditions
            if ((outp == REPLACE) && (M.nvals() == 0))
            {
                C.clear();
                return;
            }
            else if (!(outp == REPLACE) && (M.nvals() == 0))
            {
                return; // do nothing
            }

            // =================================================================

            typedef typename SemiringT::result_type D3ScalarType;
            LilSparseMatrix<D3ScalarType> T(C.nrows(), C.ncols());

            typedef decltype(accum(std::declval<CScalarT>(),
                               std::declval<TScalarType>())) ZScalarType;
            typename LilSparseMatrix<ZScalarType>::RowType  Z_row;
            typename LilSparseMatrix<CScalarT>::RowType     C_row;

            ATB_Mask_kernel(T, M, semiring, A, B);

            for (IndexType i = 0; i < C.nrows(); ++i)
            {
                // Z[i] = (M[i] .* C[i]) + T[i]
                Z_row.clear();
                masked_accum(Z_row, M[i], false, accum, C[i], T[i]);

                if (!(outp == REPLACE)) /* z = merge */
                {
                    // C[i] = [!M .* C]  U  Z[i], z = "merge"
                    C_row.clear();
                    masked_merge(C_row, M[i], false, C[i], Z_row);
                    C.setRow(i, C_row);
                }
                else // z = replace
                {
                    // C[i] = Z[i]
                    C.setRow(i, Z_row);
                }
            }

            GRB_LOG_VERBOSE("C: " << C);
        }

        //**********************************************************************
        template<typename CScalarT,
                 typename MScalarT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void sparse_mxm_CompMask_NoAccum_ATB(
            LilSparseMatrix<CScalarT>       &C,
            LilSparseMatrix<MScalarT> const &M,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B,
            OutputControlEnum                outp)
        {
            // C<M,z> = A' +.* B
            //        =              [!M .* (A' +.* B)], z = "replace"
            //        = [M .* C]  U  [!M .* (A' +.* B)], z = "merge"
            // short circuit conditions
            if ((outp == REPLACE) && ((A.nvals() == 0) || (B.nvals() == 0)))
            {
                C.clear();
                return;
            }

            // =================================================================

            typedef typename SemiringT::result_type D3ScalarType;
            LilSparseMatrix<D3ScalarType> T(C.nrows(), C.ncols());
            typename LilSparseMatrix<CScalarT>::RowType C_row;

            ATB_CompMask_kernel(T, M, semiring, A, B);

            for (IndexType i = 0; i < C.nrows(); ++i)
            {
                if ((outp == REPLACE) || M[i].empty())
                {
                    // C[i] = T[i]
                    C.setRow(i, T[i]);
                }
                else
                {
                    // C[i] = [M .* C]  U  T[i], z = "merge"
                    C_row.clear();
                    masked_merge(C_row, M[i], true, C[i], T[i]);
                    C.setRow(i, C_row);
                }
            }

            GRB_LOG_VERBOSE("C: " << C);
        }

        //**********************************************************************
        template<typename CScalarT,
                 typename MScalarT,
                 typename AccumT,
                 typename SemiringT,
                 typename AScalarT,
                 typename BScalarT>
        inline void sparse_mxm_CompMask_Accum_ATB(
            LilSparseMatrix<CScalarT>       &C,
            LilSparseMatrix<MScalarT> const &M,
            AccumT                    const &accum,
            SemiringT                        semiring,
            LilSparseMatrix<AScalarT> const &A,
            LilSparseMatrix<BScalarT> const &B,
            OutputControlEnum                outp)
        {
            // C<M,z> = C + (A +.* B)
            //        =              [!M .* [C + (A +.* B)]], z = "replace"
            //        = [M .* C]  U  [!M .* [C + (A +.* B)]], z = "merge"
            // short circuit conditions
            if ((outp == REPLACE) && (M.nvals() == 0) &&
                ((A.nvals() == 0) || (B.nvals() == 0)))
            {
                return; // do nothing
            }

            // =================================================================

            typedef typename SemiringT::result_type D3ScalarType;
            LilSparseMatrix<D3ScalarType> T(C.nrows(), C.ncols());

            typedef decltype(accum(std::declval<CScalarT>(),
                               std::declval<TScalarType>())) ZScalarType;
            typename LilSparseMatrix<ZScalarType>::RowType  Z_row;
            typename LilSparseMatrix<CScalarT>::RowType     C_row;

            ATB_CompMask_kernel(T, M, semiring, A, B);

            for (IndexType i = 0; i < C.nrows(); ++i)
            {
                // Z[i] = (!M .* C) + T[i]
                Z_row.clear();
                masked_accum(Z_row, M[i], true, accum, C[i], T[i]);

                if ((outp == REPLACE) || M[i].empty())
                {
                    // C[i] = Z[i]
                    C.setRow(i, Z_row);
                }
                else /* z = merge */
                {
                    // C[i] = [M .* C]  U  Z[i], z = "merge"
                    C_row.clear();
                    masked_merge(C_row, M[i], true, C[i], Z_row);
                    C.setRow(i, C_row);
                }
            }

            GRB_LOG_VERBOSE("C: " << C);
        }

    } // backend
} // GraphBLAS
