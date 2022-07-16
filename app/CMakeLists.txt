cmake_minimum_required(VERSION 3.20)

project(
  zebral_camera_util
  VERSION ${ZEBRAL_VERSION}
  LANGUAGES CXX
)

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
target_include_directories( zebral_camera_specs PUBLIC zebralcam::zebralcam zebralcommon::zebralcommon ${fmt_INCLUDE_DIRS})
target_compile_features(zebral_camera_specs PUBLIC cxx_std_20)
target_link_libraries(zebral_camera_specs zebralcam::zebralcam)
install(TARGETS zebral_camera_specs DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(zebral_file2c zebral_file2c.cpp)
target_include_directories( zebral_file2c PUBLIC zebralcommon::zebralcommon ${fmt_INCLUDE_DIRS})
target_compile_features(zebral_file2c PUBLIC cxx_std_20)
target_link_libraries(zebral_file2c zebralcommon::zebralcommon)
install(TARGETS zebral_file2c DESTINATION ${CMAKE_INSTALL_BINDIR})



