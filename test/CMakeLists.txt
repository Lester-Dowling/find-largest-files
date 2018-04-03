cmake_minimum_required(VERSION 3.9)


### ################################
### Automatic invocation of Qt Tools
### ################################
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


### #####################
### The target executable
### #####################

file(GLOB GLOB_SRC_TEST_FILES *.cpp)
file(GLOB_RECURSE GLOB_SRC_PROJECT_FILES ${PROJECT_SOURCE_DIR}/src/*)
add_executable(Test pch.hpp ${GLOB_SRC_PROJECT_FILES} ${GLOB_SRC_TEST_FILES} )
message(STATUS "Test src files are: ${GLOB_SRC_PROJECT_FILES} ${GLOB_SRC_TEST_FILES}")

# Add the project tree:
target_include_directories(Test PRIVATE ${PROJECT_BINARY_DIR})
target_include_directories(Test PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Activate unit testing code:
target_compile_definitions(Test PRIVATE RUN_UNIT_TESTS)

### ################
### Individual Tests
### ################
# The following augmentation was copied from:
#     https://eb2.co/blog/2015/06/driving-boost.test-with-cmake/
#
# On Windows 10, run the tests by:
#     cd Release/
#     ctest -C Release --output-on-failure
#
# !!! Assumes there is only at most one BOOST_AUTO_TEST_SUITE in each cpp file.
#
foreach(SRC_FILE_NAME ${GLOB_SRC_TEST_FILES} ${GLOB_SRC_PROJECT_FILES})
  file(READ "${SRC_FILE_NAME}" SRC_FILE_CONTENTS)
  string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\)"
	FOUND_TESTS ${SRC_FILE_CONTENTS})
  set(TEST_SUITE_NAME "")
  string(REGEX MATCHALL "BOOST_AUTO_TEST_SUITE\\( *([A-Za-z_0-9]+) *\\)"
	FOUND_SUITES ${SRC_FILE_CONTENTS})
  list(LENGTH FOUND_SUITES FOUND_SUITES_COUNT)
  if (FOUND_SUITES_COUNT GREATER 0)
	list(GET FOUND_SUITES 0 TEST_SUITE_LINE)
	string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1"
	  TEST_SUITE_NAME ${TEST_SUITE_LINE})
  endif()

  foreach(HIT ${FOUND_TESTS})
	string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1" TEST_NAME ${HIT})
	if ("${TEST_SUITE_NAME}" STREQUAL "")
	  add_test(
		NAME "${PROJECT_NAME}.${TEST_NAME}"
		COMMAND Test --run_test=${TEST_NAME} --catch_system_error=yes)
	else()
	  add_test(
		NAME "${PROJECT_NAME}.${TEST_NAME}"
		COMMAND Test --run_test=${TEST_SUITE_NAME}/${TEST_NAME} --catch_system_error=yes)
	endif()
  endforeach()
endforeach()

### ##################
### Build Style Report
### ##################
if (CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "TEST RELEASE BUILD STYLE")
elseif (CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "TEST DEBUG BUILD STYLE")
else()
  message(FATAL_ERROR "The CMAKE_BUILD_TYPE variable is not defined.  Choose Release or Debug.")
endif()

### ##################
### Project properties
### ##################
set_property(TARGET Test PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET Test PROPERTY CXX_STANDARD 14)
set_property(TARGET Test PROPERTY CXX_STANDARD_REQUIRED ON)

### ########################
### Preprocessor output dump
### ########################
if (CMAKE_PREP_OUTPUT)
  target_compile_options(Test PRIVATE $<$<CXX_COMPILER_ID:GNU>:-E>)
endif()

### ######################
### Local User Directories
### ######################

# Locate the user's home directory:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
  get_filename_component(HOME $ENV{USERPROFILE} ABSOLUTE)
else()
  get_filename_component(HOME $ENV{HOME} ABSOLUTE)
endif()

# Locate shared source code:
get_filename_component(SHARED "${HOME}/studies/Shared" ABSOLUTE)

# Locate the download directory for Open Source Software (OSS)
get_filename_component(OSS "${HOME}/oss" ABSOLUTE)

### ###################
### Pre-compiled Header
### ###################
if (NOT CMAKE_CXX_COMPILER_ID MATCHES Clang)
  include(${OSS}/cmake-precompiled-header/PrecompiledHeader.cmake)
  if (MSVC)
    target_sources(Test PRIVATE pch.cpp)
    add_precompiled_header(Test pch.hpp SOURCE_CXX pch.cpp FORCEINCLUDE)
  else()
    add_precompiled_header(Test pch.hpp)
  endif()
endif()

### ####################
### Library Dependencies
### ####################

# Custom CMake modules for this project:
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

###
### Threads
###
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(Test  Threads::Threads)

###
### Logging
###
find_package(PkgConfig REQUIRED)
pkg_check_modules(LOG4CPP REQUIRED log4cpp)
target_link_libraries(Test ${LOG4CPP_LIBRARIES})
target_include_directories(Test PRIVATE ${LOG4CPP_INCLUDE_DIRS})
target_compile_options(Test PRIVATE ${LOG4CPP_CFLAGS_OTHER})
target_compile_definitions(Test PRIVATE LJD_LOGGING)

###
### Boost
###
set(Boost_USE_STATIC_LIBS       OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(BOOST_ALL_DYN_LINK           ON)
target_compile_definitions(Test PRIVATE
  BOOST_ALL_DYN_LINK	# Boost library is dynamically linked.
  BOOST_SYSTEM_NO_LIB	# Boost library does not use the Boost auto-link facility.
  BOOST_SYSTEM_NO_DEPRECATED    # Deprecated features are excluded.
  )
find_package(Boost REQUIRED  COMPONENTS
  system
  filesystem
  iostreams
  unit_test_framework
  )
target_include_directories(Test PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(Test  ${Boost_LIBRARIES})

# Possible Boost COMPONENTS for find_package:
# atomic chrono container date_time exception filesystem graph iostreams locale
# log log_setup program_options random regex serialization signals system thread
# timer unit_test_framework wserialization
