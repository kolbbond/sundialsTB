/*
 * -----------------------------------------------------------------
 * $Revision: 4378 $
 * $Date: 2015-02-19 10:55:14 -0800 (Thu, 19 Feb 2015) $
 * -----------------------------------------------------------------
 * Programmer(s): Alan Hindmarsh, Radu Serban, and Aaron Collier @ LLNL
 * -----------------------------------------------------------------
 * LLNS Copyright Start
 * Copyright (c) 2014, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department 
 * of Energy by Lawrence Livermore National Laboratory in part under 
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 * -----------------------------------------------------------------
 * This is the header file for the KINBBDPRE module, for a
 * band-block-diagonal preconditioner, i.e. a block-diagonal
 * matrix with banded blocks, for use with KINSol, KINSp*,
 * and the parallel implementaion of the NVECTOR module.
 *
 * Summary:
 *
 * These routines provide a preconditioner matrix for KINSol that
 * is block-diagonal with banded blocks. The blocking corresponds
 * to the distribution of the dependent variable vector u amongst
 * the processes. Each preconditioner block is generated from
 * the Jacobian of the local part (associated with the current
 * process) of a given function g(u) approximating f(u). The blocks
 * are generated by each process via a difference quotient scheme,
 * utilizing the assumed banded structure with given half-bandwidths,
 * mudq and mldq. However, the banded Jacobian block kept by the
 * scheme has half-bandwidths mukeep and mlkeep, which may be smaller.
 *
 * The user's calling program should have the following form:
 *
 *   #include <sundials/sundials_types.h>
 *   #include <sundials/sundials_math.h>
 *   #include <sundials/sundials_iterative.h>
 *   #include <nvector_parallel.h>
 *   #include <kinsol.h>
 *   #include <kinsol/kinsol_bbdpre.h>
 *   ...
 *   MPI_Init(&argc,&argv);
 *   ...
 *   tmpl = N_VNew_Parallel(...);
 *   ...
 *   kin_mem = KINCreate();
 *   flag = KINInit(kin_mem,...,tmpl);
 *   ...
 *   flag = KINSptfqmr(kin_mem,...);
 *         -or-
 *   flag = KINSpbcg(kin_mem,...);
 *         -or-
 *   flag = KINSpgmr(kin_mem,...);
 *   ...
 *   flag = KINBBDPrecInit(kin_mem,...);
 *   ...
 *   KINSol(kin_mem,...);
 *   ...
 *   KINFree(&kin_mem);
 *   ...
 *   N_VDestroy_Parallel(tmpl);
 *   ...
 *   MPI_Finalize();
 *
 * The user-supplied routines required are:
 *
 *  func    the function f(u) defining the system to be solved:
 *          f(u) = 0
 *
 *  glocal  the function defining the approximation g(u) to f(u)
 *
 *  gcomm   the function to do necessary communication for glocal
 *
 * Notes:
 *
 * 1) This header file (kinsol_bbdpre.h) is included by the user for
 *    the definition of the KBBDData data type and for needed
 *    function prototypes.
 *
 * 2) The KINBBDPrecInit call includes half-bandwiths mudq and mldq
 *    to be used in the difference quotient calculation of the
 *    approximate Jacobian. They need not be the true half-bandwidths
 *    of the Jacobian of the local block of g, when smaller values may
 *    provide greater efficiency. Also, the half-bandwidths mukeep and
 *    mlkeep of the retained banded approximate Jacobian block may be
 *    even smaller, to furhter reduce storage and computational costs.
 *    For all four half-bandwidths, the values need not be the same
 *    for every process.
 *
 * 3) The actual name of the user's f function is passed to
 *    KINInit, and the names of the user's glocal and gcomm
 *    functions are passed to KINBBDPrecInit.
 *
 * 4) Optional outputs specific to this module are available by
 *    way of the functions listed below. These include work space
 *    sizes and the cumulative number of glocal calls.
 * -----------------------------------------------------------------
 */

#ifndef _KINBBDPRE_H
#define _KINBBDPRE_H

#include <sundials/sundials_nvector.h>

#ifdef __cplusplus  /* wrapper to enable C++ usage */
extern "C" {
#endif

/* KINBBDPRE return values */

#define KINBBDPRE_SUCCESS          0
#define KINBBDPRE_PDATA_NULL     -11
#define KINBBDPRE_FUNC_UNRECVR   -12
/*
 * -----------------------------------------------------------------
 * Type : KINCommFn
 * -----------------------------------------------------------------
 * The user must supply a function of type KINCommFn which
 * performs all inter-process communication necessary to
 * evaluate the approximate system function described above.
 *
 * This function takes as input the local vector size Nlocal,
 * the solution vector u, and a pointer to the user-defined
 * data block user_data.
 *
 * The KINCommFn gcomm is expected to save communicated data in
 * space defined with the structure *user_data.
 *
 * Each call to the KINCommFn is preceded by a call to the system
 * function func at the current iterate uu. Thus functions of the
 * type KINCommFn can omit any communications done by f (func) if
 * relevant to the evaluation of the KINLocalFn function. If all
 * necessary communication was done in func, the user can pass
 * NULL for gcomm in the call to KINBBDPrecInit (see below).
 *
 * A KINCommFn function should return 0 if successful or
 * a non-zero value if an error occured.
 * -----------------------------------------------------------------
 */

typedef int (*KINCommFn)(long int Nlocal, N_Vector u, void *user_data);

/*
 * -----------------------------------------------------------------
 * Type : KINLocalFn
 * -----------------------------------------------------------------
 * The user must supply a function g(u) which approximates the
 * function f for the system f(u) = 0, and which is computed
 * locally (without inter-process communication). Note: The case
 * where g is mathematically identical to f is allowed.
 *
 * The implementation of this function must have type KINLocalFn
 * and take as input the local vector size Nlocal, the local
 * solution vector uu, the returned local g values vector, and a
 * pointer to the user-defined data block user_data. It is to
 * compute the local part of g(u) and store the result in the
 * vector gval. (Note: Memory for uu and gval is handled within the
 * preconditioner module.) It is expected that this routine will
 * save communicated data in work space defined by the user and
 * made available to the preconditioner function for the problem.
 *
 * A KINLocalFn function should return 0 if successful or
 * a non-zero value if an error occured.
 * -----------------------------------------------------------------
 */

typedef int (*KINLocalFn)(long int Nlocal, N_Vector uu,
                          N_Vector gval, void *user_data);

/*
 * -----------------------------------------------------------------
 * Function : KINBBDPrecInit
 * -----------------------------------------------------------------
 * KINBBDPrecInit allocates and initializes the BBD preconditioner.
 *
 * The parameters of KINBBDPrecInit are as follows:
 *
 * kinmem  is a pointer to the KINSol memory block.
 *
 * Nlocal  is the length of the local block of the vectors
 *         on the current process.
 *
 * mudq, mldq  are the upper and lower half-bandwidths to be used
 *             in the computation of the local Jacobian blocks.
 *
 * mukeep, mlkeep  are the upper and lower half-bandwidths of the
 *                 retained banded approximation to the local
 *                 Jacobian block.
 *
 * dq_rel_uu  is the relative error to be used in the difference
 *            quotient Jacobian calculation in the preconditioner.
 *            The default is sqrt(unit roundoff), obtained by
 *            passing 0.
 *
 * gloc    is the name of the user-supplied function g(u) that
 *         approximates f and whose local Jacobian blocks are
 *         to form the preconditioner.
 *
 * gcomm   is the name of the user-defined function that performs
 *         necessary inter-process communication for the
 *         execution of gloc.
 *
 * The return value of KINBBDPrecInit is one of:
 *   KINSPILS_SUCCESS if no errors occurred
 *   KINSPILS_MEM_NULL if the integrator memory is NULL
 *   KINSPILS_LMEM_NULL if the linear solver memory is NULL
 *   KINSPILS_ILL_INPUT if an input has an illegal value
 *   KINSPILS_MEM_FAIL if a memory allocation request failed
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int KINBBDPrecInit(void *kinmem, long int Nlocal, 
                                   long int mudq, long int mldq,
                                   long int mukeep, long int mlkeep,
                                   realtype dq_rel_uu, 
                                   KINLocalFn gloc, KINCommFn gcomm);

/*
 * -----------------------------------------------------------------
 * Function : KINBBDPrecGet*
 *
 * The return value of KINBBDPrecGet* is one of:
 *    KINBBDPRE_SUCCESS    if successful
 *    KINBBDPRE_PDATA_NULL if the p_data memory was NULL
 * -----------------------------------------------------------------
 */

SUNDIALS_EXPORT int KINBBDPrecGetWorkSpace(void *kinmem, long int *lenrwBBDP, long int *leniwBBDP);
SUNDIALS_EXPORT int KINBBDPrecGetNumGfnEvals(void *kinmem, long int *ngevalsBBDP);

#ifdef __cplusplus
}
#endif

#endif
