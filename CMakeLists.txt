# Configura ANTLR C++, el compilador semántico/TAC/MIPS, el CLI y sus pruebas.
cmake_minimum_required(VERSION 3.20)
project(compiscript_analyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ANTLR_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(DISABLE_WARNINGS ON CACHE BOOL "" FORCE)
if(ANTLR4_LOCAL_SOURCE)
  add_subdirectory("${ANTLR4_LOCAL_SOURCE}" "${CMAKE_BINARY_DIR}/antlr4-runtime")
else()
  include(FetchContent)
  FetchContent_Declare(
    antlr4_cpp
    URL https://www.antlr.org/download/antlr4-cpp-runtime-4.13.2-source.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(antlr4_cpp)
endif()

set(COMPISCRIPT_GENERATED
  backend/compiscript/generated/CompiscriptLexer.cpp
  backend/compiscript/generated/CompiscriptParser.cpp
  backend/compiscript/generated/CompiscriptBaseVisitor.cpp
  backend/compiscript/generated/CompiscriptVisitor.cpp
)

set(COMPISCRIPT_CORE
  backend/compiscript/src/diagnostics.cpp
  backend/compiscript/src/json.cpp
  backend/compiscript/src/mips_generator.cpp
  backend/compiscript/src/semantic_analyzer.cpp
  backend/compiscript/src/service.cpp
  backend/compiscript/src/symbol_table.cpp
  backend/compiscript/src/tac_generator.cpp
  backend/compiscript/src/token_classifier.cpp
  backend/compiscript/src/tree_serializer.cpp
  ${COMPISCRIPT_GENERATED}
)

add_library(compiscript_core STATIC ${COMPISCRIPT_CORE})
target_include_directories(compiscript_core PUBLIC
  backend/compiscript/include
  backend/compiscript/generated
)
target_link_libraries(compiscript_core PUBLIC antlr4_static)
target_compile_definitions(compiscript_core PUBLIC ANTLR4CPP_STATIC)

if(MSVC)
  target_compile_options(compiscript_core PRIVATE /W4)
else()
  target_compile_options(compiscript_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(compiscript_cli backend/compiscript/src/main.cpp)
target_link_libraries(compiscript_cli PRIVATE compiscript_core)
set_target_properties(compiscript_cli PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
)

enable_testing()
add_executable(compiscript_tests backend/tests/compiscript_tests.cpp)
target_link_libraries(compiscript_tests PRIVATE compiscript_core)
set_target_properties(compiscript_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
)
add_test(NAME compiscript_semantic_suite COMMAND compiscript_tests)

# La suite MIPS ejecuta el ensamblador en un simulador externo real.
find_package(Python3 COMPONENTS Interpreter QUIET)
find_program(COMPISCRIPT_JAVA java)
set(COMPISCRIPT_MARS_JAR "${CMAKE_SOURCE_DIR}/tools/Mars.jar" CACHE FILEPATH "JAR de MARS para pruebas MIPS")
if(Python3_Interpreter_FOUND AND COMPISCRIPT_JAVA AND EXISTS "${COMPISCRIPT_MARS_JAR}")
  add_test(NAME compiscript_mips_execution
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/backend/tests/mips_execution_tests.py"
      --cli $<TARGET_FILE:compiscript_cli> --mars "${COMPISCRIPT_MARS_JAR}" --java "${COMPISCRIPT_JAVA}")
  set_tests_properties(compiscript_mips_execution PROPERTIES TIMEOUT 180)
else()
  message(STATUS "Pruebas MIPS requieren Python, Java y tools/Mars.jar (python3 scripts/setup_mars.py).")
endif()
