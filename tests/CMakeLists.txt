add_executable(arcade_rescue_tests
  test_main.cpp
  test_smoke.cpp)

target_link_libraries(arcade_rescue_tests PRIVATE arcade_core)
target_include_directories(arcade_rescue_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(arcade_rescue_tests PRIVATE ${ARCADE_RESCUE_WARNINGS})

add_test(NAME unit COMMAND arcade_rescue_tests)
