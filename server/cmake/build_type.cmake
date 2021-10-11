message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
if (DEFINED CMAKE_CONFIGURATION_TYPES)
    # Multi-configuration build (e.g. Xcode). Here
    # CMAKE_BUILD_TYPE doesn't matter
    message(STATUS "Available configurations: ${CMAKE_CONFIGURATION_TYPES}")
else ()
    # Single configuration generator (e.g. Unix Makefiles, Ninja)
    set(available_build_types Debug Release RelWithDebInfo MinSizeRel)
    if (NOT CMAKE_BUILD_TYPE)
        message(STATUS "CMAKE_BUILD_TYPE is not set. Setting default")
        message(STATUS "The available build types are: ${available_build_types}")
        set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE String
                "Options are ${available_build_types}"
                FORCE)
        # Provide drop down menu options in cmake-gui
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${available_build_types})
    endif ()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

    # Check the selected build type is valid
    list(FIND available_build_types "${CMAKE_BUILD_TYPE}" _build_type_index)
    if ("${_build_type_index}" EQUAL "-1")
        message(FATAL_ERROR "\"${CMAKE_BUILD_TYPE}\" is an invalid build type.\n"
                "Use one of the following build types ${available_build_types}")
    endif ()
endif ()