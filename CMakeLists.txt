# Configura ANTLR C++, el intérprete SQL (Proyecto 3), su CLI y sus pruebas.
# El compilador Compiscript del Proyecto 2 sigue disponible con
# -DBUILD_COMPISCRIPT=ON, pero ya no forma parte del pipeline por defecto.
cmake_minimum_required(VERSION 3.20)
project(sql_interpreter LANGUAGES CXX)

option(BUILD_COMPISCRIPT "Compilar también Compiscript (Proyecto 2)" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Con MinGW el runtime de GCC se enlaza estático: el .exe no depende de que
# el PATH tenga la misma libstdc++ con la que se compiló.
if(MINGW)
  add_link_options(-static -static-libgcc -static-libstdc++)
endif()

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

enable_testing()

# ─────────────── Intérprete SQL (Proyecto 3) ───────────────
set(SQL_GENERATED
  backend/sql/generated/SQLLexer.cpp
  backend/sql/generated/SQLParser.cpp
  backend/sql/generated/SQLBaseVisitor.cpp
  backend/sql/generated/SQLVisitor.cpp
)

set(SQL_CORE
  backend/sql/src/ast_builder.cpp
  backend/sql/src/ast_serializer.cpp
  backend/sql/src/catalog.cpp
  backend/sql/src/diagnostics.cpp
  backend/sql/src/executor.cpp
  backend/sql/src/file_transaction.cpp
  backend/sql/src/json.cpp
  backend/sql/src/semantic_analyzer.cpp
  backend/sql/src/service.cpp
  backend/sql/src/storage.cpp
  backend/sql/src/token_classifier.cpp
  backend/sql/src/tree_serializer.cpp
  backend/sql/src/value.cpp
  ${SQL_GENERATED}
)

add_library(sql_core STATIC ${SQL_CORE})
target_include_directories(sql_core PUBLIC
  backend/sql/include
  backend/sql/generated
)
target_link_libraries(sql_core PUBLIC antlr4_static)
target_compile_definitions(sql_core PUBLIC ANTLR4CPP_STATIC)

if(MSVC)
  target_compile_options(sql_core PRIVATE /W4)
else()
  target_compile_options(sql_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(sql_cli backend/sql/src/main.cpp)
target_link_libraries(sql_cli PRIVATE sql_core)
set_target_properties(sql_cli PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
)

add_executable(sql_tests backend/tests/sql_tests.cpp)
target_link_libraries(sql_tests PRIVATE sql_core)
set_target_properties(sql_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
)
add_test(NAME sql_interpreter_suite COMMAND sql_tests)

# ─────────────── Compiscript (Proyecto 2, opcional) ───────────────
if(BUILD_COMPISCRIPT)
  set(COMPISCRIPT_GENERATED
    backend/compiscript/generated/CompiscriptLexer.cpp
    backend/compiscript/generated/CompiscriptParser.cpp
    backend/compiscript/generated/CompiscriptBaseVisitor.cpp
    backend/compiscript/generated/CompiscriptVisitor.cpp
  )

  set(COMPISCRIPT_CORE
    backend/compiscript/src/diagnostics.cpp
    backend/compiscript/src/json.cpp
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

  add_executable(compiscript_tests backend/tests/compiscript_tests.cpp)
  target_link_libraries(compiscript_tests PRIVATE compiscript_core)
  set_target_properties(compiscript_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
  )
  add_test(NAME compiscript_semantic_suite COMMAND compiscript_tests)
endif()
