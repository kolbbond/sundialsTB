# ---------------------------------------------------------------
# $Revision: 4483 $
# $Date: 2015-04-22 16:30:17 -0700 (Wed, 22 Apr 2015) $
# ---------------------------------------------------------------
# Programmer:  Radu Serban @ LLNL
# ---------------------------------------------------------------
# LLNS Copyright Start
# Copyright (c) 2014, Lawrence Livermore National Security
# This work was performed under the auspices of the U.S. Department 
# of Energy by Lawrence Livermore National Laboratory in part under 
# Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
# Produced at the Lawrence Livermore National Laboratory.
# All rights reserved.
# For details, see the LICENSE file.
# LLNS Copyright End
# ---------------------------------------------------------------
# CMakeLists.txt file for IDAS serial examples
#
# LEBanks 3/25/2013
#    Added SUPERLUMT logic.


# Add variable IDAS_examples with the names of the serial IDAS examples

SET(IDAS_examples
  idasAkzoNob_ASAi_dns
  idasAkzoNob_dns
  idasFoodWeb_bnd
  idasHeat2D_bnd
  idasHeat2D_kry
  idasHessian_ASA_FSA
  idasKrylovDemo_ls
  idasRoberts_ASAi_dns
  idasRoberts_dns
  "idasRoberts_FSA_dns\;-sensi stg t"
  idasSlCrank_dns
  idasSlCrank_FSA_dns
  )

# Add variable IDAS_examples_BL with the names of the serial IDAS examples
# that use Lapack

SET(IDAS_examples_BL
  )
  
SET(IDAS_examples_SUPERLUMT
  idasRoberts_sps
  )

SET(IDAS_examples_KLU
  idasRoberts_klu
  )

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE

IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(IDAS_LIB sundials_idas_static)
  SET(NVECS_LIB sundials_nvecserial_static)
ELSE(LINK_LIBRARY_TYPE MATCHES "static")
  SET(IDAS_LIB sundials_idas_shared)
  SET(NVECS_LIB sundials_nvecserial_shared)
ENDIF(LINK_LIBRARY_TYPE MATCHES "static")

# Set-up linker flags and link libraries

SET(SUNDIALS_LIBS ${IDAS_LIB} ${NVECS_LIB} ${EXTRA_LINK_LIBS})
IF(LAPACK_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${LAPACK_LIBRARIES})
ENDIF(LAPACK_FOUND)

IF(SUPERLUMT_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${SUPERLUMT_LIBRARIES})
ENDIF(SUPERLUMT_FOUND)

IF(KLU_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${KLU_LIBRARIES})
ENDIF(KLU_FOUND)

# Add the build and install targets for each IDAS example

# SGS is there a way to query CMAKE variable to get added executables 
# rather than keeping our own list?
SET(ADDED_EXECUTABLES "")
FOREACH(example_tuple ${IDAS_examples})
  LIST(GET example_tuple 0 example)

  # Only need to add the executable once
  LIST(FIND ADDED_EXECUTABLES ${example} index)
  IF(index EQUAL -1)
    LIST(APPEND ADDED_EXECUTABLES ${example})
    ADD_EXECUTABLE(${example} ${example}.c)
    SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
  ENDIF(index EQUAL -1)

  LIST(LENGTH example_tuple n)
  IF(n EQUAL 2)
    LIST(GET example_tuple 1 test_args)
    STRING(REGEX REPLACE " " "_" test_name ${example}_${test_args})
    SUNDIALS_ADD_TEST(${test_name} ${example} TEST_ARGS ${test_args})
  ELSE()
    SUNDIALS_ADD_TEST(${example} ${example})
  ENDIF()

  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/idas/serial)
  ENDIF(EXAMPLES_INSTALL)
ENDFOREACH(example_tuple ${IDAS_examples})

# If Lapack support is enabled, add the build and install targets for
# the examples using Lapack
IF(LAPACK_FOUND)
  FOREACH(example ${IDAS_examples_BL})
    ADD_EXECUTABLE(${example} ${example}.c)
    SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
    SUNDIALS_ADD_TEST(${example} ${example})
    TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
    IF(EXAMPLES_INSTALL)
      INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/idas/serial)
    ENDIF(EXAMPLES_INSTALL)
  ENDFOREACH(example ${IDAS_examples_BL})
ENDIF(LAPACK_FOUND)

# If SUPERLUMT support is enabled, add the build and install targets for
# the examples using SUPERLUMT
IF(SUPERLUMT_FOUND)
  FOREACH(example ${IDAS_examples_SUPERLUMT})
    ADD_EXECUTABLE(${example} ${example}.c)
    SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
    SUNDIALS_ADD_TEST(${example} ${example})
    TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
    IF(EXAMPLES_INSTALL)
      INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/idas/serial)
    ENDIF(EXAMPLES_INSTALL)
  ENDFOREACH(example ${IDAS_examples_SUPERLUMT})
ENDIF(SUPERLUMT_FOUND)

# If KLU support is enabled, add the build and install targets for
# the examples using KLU
IF(KLU_FOUND)
  FOREACH(example ${IDAS_examples_KLU})
    ADD_EXECUTABLE(${example} ${example}.c)
    SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
    SUNDIALS_ADD_TEST(${example} ${example})
    TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
    IF(EXAMPLES_INSTALL)
      INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/idas/serial)
    ENDIF(EXAMPLES_INSTALL)
  ENDFOREACH(example ${IDAS_examples_KLU})
ENDIF(KLU_FOUND)

IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/idas/serial)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER "IDAS")
  SET(SOLVER_LIB "sundials_idas")
  FOREACH(example_tuple ${IDAS_examples})
  	list(GET example_tuple 0 example)
  	LIST2STRING(example EXAMPLES)
  ENDFOREACH(example_tuple ${IDAS_examples})

  IF(LAPACK_FOUND)
    LIST2STRING(IDAS_examples_BL EXAMPLES_BL)
  ELSE(LAPACK_FOUND)
    SET(EXAMPLES_BL "")
  ENDIF(LAPACK_FOUND)

  IF(KLU_FOUND)
    LIST2STRING(IDAS_examples_KLU EXAMPLES)
  ENDIF(KLU_FOUND)

  IF(SUPERLUMT_FOUND)
    LIST2STRING(IDAS_examples_SUPERLUMT EXAMPLES)
  ENDIF(SUPERLUMT_FOUND)

  STRING (REPLACE ";" " " TMP_STR ${EXAMPLES})
  SET(EXAMPLES ${TMP_STR})

  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_serial_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/idas/serial/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/idas/serial/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/idas/serial 
    )

  # On UNIX-type platforms, we also  generate and install a makefile for 
  # building the examples. This makefile can then be used as a template 
  # for the user's own programs.

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_serial_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/idas/serial/Makefile_ex
      @ONLY
      )
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/idas/serial/Makefile_ex 
      DESTINATION ${EXAMPLES_INSTALL_PATH}/idas/serial 
      RENAME Makefile
      )
  ENDIF(UNIX)

ENDIF(EXAMPLES_INSTALL)
