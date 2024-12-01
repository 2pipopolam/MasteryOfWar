cmake_minimum_required(VERSION 3.10)
project(GameServer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(SQLite3 REQUIRED)

# Find JsonCpp using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)

# Add source files
file(GLOB SOURCES 
    "src/*.cpp"
)

# Create executable
add_executable(GameServer ${SOURCES})

# Link libraries
target_link_libraries(GameServer
    PRIVATE
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${SQLite3_LIBRARIES}
    pthread
)

# Add compiler warnings
if(MSVC)
    target_compile_options(GameServer PRIVATE /W4)
else()
    target_compile_options(GameServer PRIVATE -Wall -Wextra)
endif()