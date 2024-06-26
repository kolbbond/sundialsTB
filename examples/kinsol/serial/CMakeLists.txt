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
# CMakeLists.txt file for KINSOL serial examples


# Add variable KINSOL_examples with the names of the serial KINSOL examples

SET(KINSOL_examples
  kinFerTron_dns
  kinFoodWeb_kry
  kinKrylovDemo_ls
  kinLaplace_bnd
  kinLaplace_picard_bnd
  kinRoberts_fp
  kinRoboKin_dns
  )

# Add variable KINSOL_examples_BL with the names of the serial KINSOL examples
# that use Lapack

SET(KINSOL_examples_BL
  )

SET(KINSOL_examples_SUPERLUMT
  kinRoboKin_slu
  )

# Specify libraries to link against (through the target that was used to 
# generate them) based on the value of the variable LINK_LIBRARY_TYPE

IF(LINK_LIBRARY_TYPE MATCHES "static")
  SET(KINSOL_LIB sundials_kinsol_static)
  SET(NVECS_LIB sundials_nvecserial_static)
ELSE(LINK_LIBRARY_TYPE MATCHES "static")
  SET(KINSOL_LIB sundials_kinsol_shared)
  SET(NVECS_LIB sundials_nvecserial_shared)
ENDIF(LINK_LIBRARY_TYPE MATCHES "static")

# Set-up linker flags and link libraries

SET(SUNDIALS_LIBS ${KINSOL_LIB} ${NVECS_LIB} ${EXTRA_LINK_LIBS})
IF(LAPACK_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${LAPACK_LIBRARIES})
ENDIF(LAPACK_FOUND)

IF(SUPERLUMT_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${SUPERLUMT_LIBRARIES})
ENDIF(SUPERLUMT_FOUND)

IF(KLU_FOUND)
  LIST(APPEND SUNDIALS_LIBS ${KLU_LIBRARIES})
ENDIF(KLU_FOUND)

# Add the build and install targets for each KINSOL example

FOREACH(example ${KINSOL_examples})
  ADD_EXECUTABLE(${example} ${example}.c)
  SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
  SUNDIALS_ADD_TEST(${example} ${example})
  TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
  IF(EXAMPLES_INSTALL)
    INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/kinsol/serial)
  ENDIF(EXAMPLES_INSTALL)
ENDFOREACH(example ${KINSOL_examples})

# If Lapack support is enabled, add the build and install targets for
# the examples using Lapack

IF(LAPACK_FOUND)
  FOREACH(example ${KINSOL_examples_BL})
    ADD_EXECUTABLE(${example} ${example}.c)    
    SET_TARGET_PROPERTIES(${example} PROPERTIES FOLDER "Examples")
    SUNDIALS_ADD_TEST(${example} ${example})
    TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
    IF(EXAMPLES_INSTALL)
      INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/kinsol/serial)
    ENDIF(EXAMPLES_INSTALL)
  ENDFOREACH(example ${KINSOL_examples_BL})
ENDIF(LAPACK_FOUND)

# If SUPERLUMT support is enabled, add the build and install targets for
# the examples using SUPERLUMT
IF(SUPERLUMT_FOUND)
  FOREACH(example ${KINSOL_examples_SUPERLUMT})
    ADD_EXECUTABLE(${example} ${example}.c)
    SUNDIALS_ADD_TEST(${example} ${example})
    TARGET_LINK_LIBRARIES(${example} ${SUNDIALS_LIBS})
    IF(EXAMPLES_INSTALL)
      INSTALL(FILES ${example}.c ${example}.out DESTINATION ${EXAMPLES_INSTALL_PATH}/kinsol/serial)
    ENDIF(EXAMPLES_INSTALL)
  ENDFOREACH(example ${KINSOL_examples_SUPERLUMT})
ENDIF(SUPERLUMT_FOUND)


IF(EXAMPLES_INSTALL)

  # Install the README file
  INSTALL(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/kinsol/serial)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  SET(SOLVER "KINSOL")
  SET(SOLVER_LIB "sundials_kinsol")
  LIST2STRING(KINSOL_examples EXAMPLES)
  IF(LAPACK_FOUND)
    LIST2STRING(KINSOL_examples_BL EXAMPLES_BL)
  ELSE(LAPACK_FOUND)
    SET(EXAMPLES_BL "")
  ENDIF(LAPACK_FOUND)

  IF(SUPERLUMT_FOUND)
    LIST2STRING(KINSOL_examples_SUPERLUMT EXAMPLES)
  ENDIF(SUPERLUMT_FOUND)

  STRING (REPLACE ";" " " TMP_STR ${EXAMPLES})
  SET(EXAMPLES ${TMP_STR})

  # Regardless of the platform we're on, we will generate and install 
  # CMakeLists.txt file for building the examples. This file  can then 
  # be used as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_serial_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/kinsol/serial/CMakeLists.txt
      @ONLY
      )

  # install CMakelists.txt
  INSTALL(
    FILES ${PROJECT_BINARY_DIR}/examples/kinsol/serial/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/kinsol/serial 
    )

  # On UNIX-type platforms, we also  generate and install a makefile for 
  # building the examples. This makefile can then be used as a template 
  # for the user's own programs.

  IF(UNIX)
    # generate Makefile and place it in the binary dir
    CONFIGURE_FILE(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_serial_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/kinsol/serial/Makefile_ex
      @ONLY
      )    
    # install the configured Makefile_ex as Makefile
    INSTALL(
      FILES ${PROJECT_BINARY_DIR}/examples/kinsol/serial/Makefile_ex 
      DESTINATION ${EXAMPLES_INSTALL_PATH}/kinsol/serial 
      RENAME Makefile
      )
  ENDIF(UNIX)


ENDIF(EXAMPLES_INSTALL)
