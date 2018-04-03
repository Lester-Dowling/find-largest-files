cmake_minimum_required (VERSION 3.9)
project(find-largest-files VERSION 0.0.2 LANGUAGES CXX)

### #####################
### The target executable
### #####################
file(GLOB_RECURSE GLOB_PROJECT_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*)
add_executable(${PROJECT_NAME}  ${GLOB_PROJECT_SRC_FILES})

# Include top-level the project src tree:
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

### ##################
### Project properties
### ##################
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

### #########################
### Configuration header file
### #########################
set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file (
  "${PROJECT_SOURCE_DIR}/config.hpp.in"
  "${PROJECT_BINARY_DIR}/config.hpp"
  )

### ########################
### Preprocessor output dump
### ########################
if (CMAKE_PREP_OUTPUT)
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-E>)
  message(STATUS "PREPROCESSOR OUTPUT ONLY")
endif()

### ##################
### Build Style Report
### ##################
if (CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "RELEASE BUILD STYLE")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "DEBUG BUILD STYLE")
else()
  message(FATAL_ERROR "The CMAKE_BUILD_TYPE variable is not defined.  Choose Release or Debug.")
endif()

foreach(_filename ${GLOB_PROJECT_SRC_FILES})
  message(STATUS "Src filename: ${_filename}")
endforeach()

### ####################
### Library Dependencies
### ####################

###
### Threads
###
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}  Threads::Threads)

###
### Boost
###
set(Boost_USE_STATIC_LIBS       OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(BOOST_ALL_DYN_LINK           ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE
  BOOST_ALL_DYN_LINK	        # Boost library is dynamically linked.
  BOOST_SYSTEM_NO_LIB           # Boost library does not use the Boost auto-link facility.
  BOOST_SYSTEM_NO_DEPRECATED    # Deprecated features are excluded.
  )
find_package(Boost REQUIRED  COMPONENTS  system  filesystem  )
target_link_libraries(${PROJECT_NAME}  ${Boost_LIBRARIES})

# Possible Boost COMPONENTS for find_package:
# atomic chrono container date_time exception filesystem graph iostreams locale
# log log_setup program_options random regex serialization signals system thread
# timer unit_test_framework wserialization

###
### Copy required Boost DLLs next to the newly built exe.  Requires the variable
### RUNTIME_OUTPUT_DIRECTORY to have been already defined.
###
if (WIN32)
  set(Boost_Do_Next_Copy                OFF)
  foreach(Boost_Lib ${Boost_LIBRARIES})
    MESSAGE( STATUS "Boost_Lib:  " ${Boost_Lib})
    if (Boost_Do_Next_Copy)
      set(Boost_Do_Next_Copy OFF)
      get_filename_component(Boost_Lib_Basename ${Boost_Lib} NAME_WE)
      get_filename_component(Boost_Lib_Directory ${Boost_Lib} DIRECTORY)
      set(Boost_DLL_path "${Boost_Lib_Directory}/${Boost_Lib_Basename}.dll")
      file(TO_CMAKE_PATH ${Boost_DLL_path} Boost_DLL)
      MESSAGE( STATUS "Boost_DLL:           " ${Boost_DLL})
      add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${Boost_DLL}"
        ${RUNTIME_OUTPUT_DIRECTORY})
    endif()
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
      if (Boost_Lib MATCHES "debug")
        set(Boost_Do_Next_Copy ON)
      endif()
    endif()
    if (CMAKE_BUILD_TYPE MATCHES "Release")
      if (Boost_Lib MATCHES "optimized")
        set(Boost_Do_Next_Copy ON)
      endif()
    endif()
  endforeach(Boost_Lib)
endif()
