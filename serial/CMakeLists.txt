# ZebralSerial library Simple serial comms
cmake_minimum_required(VERSION 3.20)

project(
  zebralserial
  VERSION ${ZEBRAL_VERSION}
  LANGUAGES CXX
)

if(MSVC)
  add_compile_options(/W4 /WX /Zc:preprocessor)
  set(EXTRA_LIBS Strmiids.lib Mfplat.lib Mf.lib)
  # add_link_options(/DELAYLOAD:MF.lib)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-c11-extensions)
  set(EXTRA_LIBS fmt::fmt)
endif()

# Add serial library
set(SRC src/serial_line_channel.cpp src/serial_line_channel_linux.cpp
        src/serial_line_channel_winrt.cpp src/serial_info.cpp
)

set(INC inc/serial_line_channel.hpp inc/serial_info.hpp)

add_library(${PROJECT_NAME} ${SRC} ${INC})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(
  ${PROJECT_NAME} PUBLIC inc ${fmt_INCLUDE_DIRS} zebralcam::zebralcam
)
target_link_libraries(
  ${PROJECT_NAME} PUBLIC ${EXTRA_LIBS} Threads::Threads zebralcam::zebralcam
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_subdirectory(test)

# Binary and includes...
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${INC} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
