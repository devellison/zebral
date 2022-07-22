cmake_minimum_required(VERSION 3.20)

if(MSVC)
  add_compile_options(/W4 /WX /Zc:preprocessor)
  set(EXTRA_LIBS Strmiids.lib Mfplat.lib Mf.lib delayimp.lib)
  # add_link_options(/DELAYLOAD:Mf.lib)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-c11-extensions)
  set(EXTRA_LIBS fmt::fmt)
endif()

if(OpenMP_CXX_FOUND)
  set(EXTRA_LIBS ${EXTRA_LIBS} OpenMP::OpenMP_CXX)
endif()

# Basic unit tests
add_executable(common_tests common_tests.cpp)
target_link_libraries(
  common_tests gtest_main zebralcommon::zebralcommon ${EXTRA_LIBS}
)
gtest_add_tests(TARGET common_tests)

# Copy our test files into the binary dir
file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.xml")

file(COPY ${TEST_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
