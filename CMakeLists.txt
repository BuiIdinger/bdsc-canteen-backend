cmake_minimum_required(VERSION 3.10)

# Allow targets to be modified from outside 
cmake_policy(SET CMP0079 NEW)

# Set the project name
project(
  bdsc-canteen-backend
  VERSION 1.0.0 
  DESCRIPTION "BDSC Canteen backend." 
  LANGUAGES CXX
)

# Set C++ standards
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type
set(CMAKE_BUILD_TYPE Debug)

# Set variables
set(BC_PROJECT_NAME bdsc-canteen-backend)
set(BC_SRC_DIRECTORY ${CMAKE_SOURCE_DIR}/src)

# Add sub projects
add_subdirectory(src)

# Include headers in the include directory
target_include_directories(${BC_PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src/include)

# Find & link BWSS
find_package(bwss REQUIRED)
target_link_libraries(${BC_PROJECT_NAME} PUBLIC bwss::bwss)
target_include_directories(${BC_PROJECT_NAME} PUBLIC ${BWSS_INCLUDE_DIR})

# Find & link OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(${BC_PROJECT_NAME} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(${BC_PROJECT_NAME} PUBLIC ${OPENSSL_INCLUDE_DIR})

# Find & link Boost JSON
find_package(Boost CONFIG REQUIRED COMPONENTS json)
target_link_libraries(${BC_PROJECT_NAME} PUBLIC Boost::json)
target_include_directories(${BC_PROJECT_NAME} PUBLIC ${BOOST_INCLUDE_DIR})

# Find & link Cassandra library
# Link libraries
target_link_libraries(${BC_PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/externel/libcassandra.dylib)