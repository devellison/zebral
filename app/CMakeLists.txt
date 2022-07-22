cmake_minimum_required(VERSION 3.20)

project(
  zebral_camera_util
  VERSION ${ZEBRAL_VERSION}
  LANGUAGES CXX
)

if(MSVC)
  add_compile_options(/W4 /WX /Zc:preprocessor)
  set(EXTRA_LIBS Strmiids.lib Mfplat.lib Mf.lib)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-c11-extensions)
  set(EXTRA_LIBS fmt::fmt)
endif()

# Add camera utility
set(ZEB_SRC zebral_camera_util.cpp)
set(ZEB_INC)

add_executable(${PROJECT_NAME} ${ZEB_SRC} ${ZEB_INC})
target_include_directories(
  ${PROJECT_NAME} PUBLIC zebralcam::zebralcam zebralcommon::zebralcommon
                         ${OpenCV_INCLUDE_DIRS} ${fmt_INCLUDE_DIRS}
)
target_compile_definitions(
  ${PROJECT_NAME} PUBLIC ZEBRAL_VERSION="${ZEBRAL_VERSION}"
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} zebralcam::zebralcam ${OpenCV_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(zebral_camera_specs zebral_camera_specs.cpp)
target_include_directories(
  zebral_camera_specs PUBLIC zebralcam::zebralcam zebralcommon::zebralcommon
                             ${fmt_INCLUDE_DIRS}
)
target_compile_features(zebral_camera_specs PUBLIC cxx_std_20)
target_link_libraries(zebral_camera_specs zebralcam::zebralcam)
install(TARGETS zebral_camera_specs DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(zbafile2c zebral_file2c.cpp)
target_include_directories(
  zbafile2c PUBLIC zebralcommon::zebralcommon ${fmt_INCLUDE_DIRS}
)
target_compile_features(zbafile2c PUBLIC cxx_std_20)
target_link_libraries(zbafile2c zebralcommon::zebralcommon)
install(TARGETS zbafile2c DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(zbaterm zebral_terminal.cpp)
target_include_directories(
  zbaterm PUBLIC zebralcommon::zebralcommon zebralserial::zebralserial
                 ${fmt_INCLUDE_DIRS}
)
target_compile_features(zbaterm PUBLIC cxx_std_20)
target_link_libraries(
  zbaterm zebralcommon::zebralcommon zebralserial::zebralserial
)
install(TARGETS zbaterm DESTINATION ${CMAKE_INSTALL_BINDIR})
