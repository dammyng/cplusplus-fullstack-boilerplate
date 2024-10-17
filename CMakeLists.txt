cmake_minimum_required(VERSION 3.10)
project(CapReturn)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost libraries
find_package(Boost 1.74 REQUIRED COMPONENTS system program_options)

# Find PostgreSQL and libpqxx
find_package(PostgreSQL REQUIRED)
find_library(PQXX_LIB pqxx REQUIRED)
find_library(PQ_LIB pq REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)


# Include directories
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.cpp")

include(FetchContent)

FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG        v0.6.0 
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "disable building examples" FORCE)
FetchContent_MakeAvailable(jwt-cpp)

add_executable(cap_returns ${SOURCES})

# Link libraries
target_link_libraries(cap_returns
    Boost::system
    Boost::program_options
    ${PQXX_LIB}
    ${PQ_LIB}
    pthread
    nlohmann_json::nlohmann_json
    jwt-cpp::jwt-cpp 
)