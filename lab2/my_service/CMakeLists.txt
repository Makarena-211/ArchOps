cmake_minimum_required(VERSION 3.12...3.31)
project(my_service CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(
  userver
  COMPONENTS
    core
    postgresql
  QUIET
)

if(NOT userver_FOUND)
  download_userver(TRY_DIR third_party/userver)
endif()

userver_setup_environment()

find_package(OpenSSL REQUIRED)

# --- Common sources ---
add_library(
  ${PROJECT_NAME}_objs OBJECT

  src/components/component_list.cpp
  src/components/auth_config.cpp

  src/auth/jwt.cpp
  src/auth/password.cpp

  src/handlers/health.cpp
  src/handlers/auth_register.cpp
  src/handlers/auth_login.cpp

  src/handlers/records_create.cpp
  src/handlers/records_put.cpp
  src/handlers/records_patch.cpp
  src/handlers/records_delete.cpp
  src/handlers/records_list.cpp

  src/handlers/patients_create.cpp
  src/handlers/patients_search.cpp
  src/handlers/patients_records_create.cpp
  src/handlers/patients_records_list.cpp
  src/handlers/records_get.cpp
  src/handlers/users_get_by_login.cpp
  src/handlers/users_search_by_name.cpp
)

target_include_directories(
  ${PROJECT_NAME}_objs
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
  ${PROJECT_NAME}_objs
  PUBLIC
    userver::core
    userver::postgresql
  PRIVATE
    OpenSSL::Crypto
)

# --- The Service ---
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# --- Unit Tests (опционально) ---
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/greeting_test.cpp")
  add_executable(${PROJECT_NAME}_unittest src/greeting_test.cpp)
  target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
  add_google_tests(${PROJECT_NAME}_unittest)
endif()

# --- Benchmarks (опционально) ---
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/greeting_benchmark.cpp")
  add_executable(${PROJECT_NAME}_benchmark src/greeting_benchmark.cpp)
  target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver::ubench)
  add_google_benchmark_tests(${PROJECT_NAME}_benchmark)
endif()

# Functional testing
userver_testsuite_add_simple()

# --- Install ---
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml
  ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json
)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT ${PROJECT_NAME}
)

install(
  FILES ${CONFIGS_FILES}
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
  COMPONENT ${PROJECT_NAME}
)