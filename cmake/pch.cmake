# - Simple precompiler header support for gcc 4.x
# Sets up precompiled headers for your C/C++ project.
#
#=====
# Copyright 2009-2013 Igor Shevchenko <igor.shevchenko@gmail.com>
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written agreement
# is hereby granted.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#=====
#
# Basic usage:
#
# create_precompiled_header(LABEL, HEADER_NAME)
# - creates precompiled header target "LABEL" based on single header HEADER_NAME
#
# use_precompiled_header(TARGET, LABEL)
# - attaches precompiled header LABEL to any other target TARGET#
#

# Used to get gcc compile flags.
MACRO(PCH_GET_COMPILE_FLAGS _pch_compile_flags)
    STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
    SET(${_pch_compile_flags} ${${_flags_var_name}})

    GET_DIRECTORY_PROPERTY(DIRINC INCLUDE_DIRECTORIES)
    FOREACH(item ${DIRINC})
        LIST(APPEND ${_pch_compile_flags} "-I${item}")
    ENDFOREACH()

    GET_DIRECTORY_PROPERTY(_directory_flags COMPILE_DEFINITIONS)
    FOREACH(item ${_directory_flags})
        LIST(APPEND ${_pch_compile_flags} "-D${item}")
    ENDFOREACH()

    LIST(APPEND ${_pch_compile_flags} ${CMAKE_CXX_FLAGS})

    SEPARATE_ARGUMENTS(${_pch_compile_flags})
ENDMACRO()


#
# create precompiled header
#
#IN: PCH_NAME: precompiled header's name (e.g. "stable" for "common/stable.h"
#   header_fn: full path to the precompiled header file (e.g. "${CMAKE_SOURCE_DIR}/common/stable.h")
#
MACRO(CREATE_PRECOMPILED_HEADER PCH_NAME HEADER_NAME)
    SET(_pch_header_fn ${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_NAME})
    get_filename_component(_pch_header_name ${_pch_header_fn} NAME)
    get_filename_component(_pch_header_name_we ${_pch_header_fn} NAME_WE)

    SET(_pch_gch_dir ${CMAKE_BINARY_DIR}/pch/${_pch_header_name}.gch)
    SET(_pch_hxx_fn ${_pch_gch_dir}/${_pch_header_name_we}.hxx)
    SET(_pch_hxx_dep ${PCH_NAME}_hxx_dephelp)

    # create dirs
    MAKE_DIRECTORY(${CMAKE_BINARY_DIR}/pch)
    MAKE_DIRECTORY(${_pch_gch_dir})

    pch_get_compile_flags(_pch_compile_flags)

    # figure out header's dependencies
    execute_process(COMMAND perl ${CMAKE_SOURCE_DIR}/cmake/pch_get_deps.pl ${_pch_header_fn} ${CMAKE_CXX_COMPILER} "${_pch_compile_flags}" OUTPUT_VARIABLE _pch_header_deps)

    SET(_pch_compile_cmd ${CMAKE_CXX_COMPILER} ${_pch_compile_flags} -x c++-header -o ${_pch_hxx_fn} ${_pch_header_fn})
    add_custom_command(
        OUTPUT  ${_pch_hxx_fn}
        COMMAND ${_pch_compile_cmd}
        DEPENDS ${_pch_header_fn} ${_pch_header_deps}
    )

    # create dependency helper target
    add_custom_target(${_pch_hxx_dep} DEPENDS ${_pch_hxx_fn} ${_pch_header_deps})

    # set global macros which are later used in "USE_PRECOMPILED_HEADER(..)"
    SET(_pch_${PCH_NAME}_hxx_fn ${_pch_hxx_fn})
    SET(_pch_${PCH_NAME}_include ${CMAKE_BINARY_DIR}/pch/${_pch_header_name})
    SET(_pch_${PCH_NAME}_dep ${_pch_hxx_dep})
ENDMACRO()


#
# use pch for a target
#
FUNCTION(USE_PRECOMPILED_HEADER TARGET_NAME PCH_NAME)
    ADD_DEPENDENCIES(${TARGET_NAME} ${_pch_${PCH_NAME}_dep})

    #
    # add dependencies between source files defined for target and pch h++ file generated by gcc.
    # This makes sure source files are recompiled whenever h++ file is re-generated.
    #
    get_target_property(_cxx_files ${TARGET_NAME} SOURCES)
    FOREACH(item ${_cxx_files})
        IF(item MATCHES "\\.(cpp|cc|cxx)$")
            set_source_files_properties(${item} PROPERTIES OBJECT_DEPENDS ${_pch_${PCH_NAME}_hxx_fn})
        ELSE()
            #message(STATUS "item=${item} didn't match")
        ENDIF()
    ENDFOREACH()

    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-include ${_pch_${PCH_NAME}_include} -Winvalid-pch")
ENDFUNCTION()
