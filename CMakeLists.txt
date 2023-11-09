cmake_minimum_required(VERSION 3.12)
project(SaneQL)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -fPIC")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Source files
set(SOURCES
    parser/ASTBase.cpp
    parser/SaneQLLexer.cpp
    infra/Schema.cpp
    semana/Functions.cpp
    semana/SemanticAnalysis.cpp
    algebra/Expression.cpp
    algebra/Operator.cpp
    sql/SQLWriter.cpp
    sql/SQLGenerator.cpp
    compiler/SaneQLCompiler.cpp
)

# Generated sources
set(GEN_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/parser/saneql_parser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/parser/AST.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/parser/AST.cpp)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${GEN_SOURCES})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/parser)

# Custom command for AST generation
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser/AST.hpp ${CMAKE_CURRENT_BINARY_DIR}/parser/AST.cpp
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/astgen --astbaseheader parser/ASTBase.hpp --astbaseclass saneql::ASTBase --astheader parser/AST.hpp --namespace saneql::ast astheader parser/astspec ${CMAKE_CURRENT_BINARY_DIR}/parser/AST.hpp
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/astgen --astbaseheader parser/ASTBase.hpp --astbaseclass saneql::ASTBase --astheader parser/AST.hpp --namespace saneql::ast astbody parser/astspec ${CMAKE_CURRENT_BINARY_DIR}/parser/AST.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser/astspec ${CMAKE_CURRENT_BINARY_DIR}/astgen
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Custom command for parser generation
find_program(BISON bison)
add_custom_command(
    OUTPUT ${GEN_SOURCES}
    COMMAND makeutil/patchbison ${BISON} ${CMAKE_CURRENT_BINARY_DIR}/parser/saneql.expanded.ypp ${GEN_SOURCES}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/parser/saneql.expanded.ypp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parser/saneql.expanded.ypp
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/astgen expandgrammar parser/astspec parser/saneql.ypp ${CMAKE_CURRENT_BINARY_DIR}/parser/saneql.expanded.ypp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser/astspec ${CMAKE_CURRENT_SOURCE_DIR}/parser/saneql.ypp ${CMAKE_CURRENT_BINARY_DIR}/astgen
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Executable
add_executable(saneql ${SOURCES} ${GEN_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_include_directories(saneql PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Shared Library
add_library(saneql_shared SHARED ${SOURCES} ${GEN_SOURCES})
target_include_directories(saneql_shared PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Static Library
add_library(saneql_static STATIC ${SOURCES} ${GEN_SOURCES})
target_include_directories(saneql_static PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# ASTGen executable
add_executable(astgen makeutil/astgen.cpp)

# --------------------------------------------------------------------------------
# Print Configuration
# --------------------------------------------------------------------------------

message(STATUS "[saneql] settings")
message(STATUS "     CMAKE_CXX_COMPILER         = ${CMAKE_CXX_COMPILER}")
message(STATUS "     CMAKE_C_COMPILER           = ${CMAKE_C_COMPILER}")
message(STATUS "     CMAKE_CXX_FLAGS            = ${CMAKE_CXX_FLAGS}")
message(STATUS "     CMAKE_CXX_FLAGS_RELEASE    = ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "     CMAKE_BUILD_TYPE           = ${CMAKE_BUILD_TYPE}")
message(STATUS "     CMAKE_CURRENT_SOURCE_DIR   = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "     CMAKE_CURRENT_BINARY_DIR   = ${CMAKE_CURRENT_BINARY_DIR}")
