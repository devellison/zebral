# Zebral Common library
#
# General framework code and utilities
cmake_minimum_required(VERSION 3.20)

project(
  zebralcommon
  VERSION ${ZEBRAL_VERSION}
  LANGUAGES CXX
)

if(MSVC)
  add_compile_options(/W4 /WX /Zc:preprocessor)
  set(EXTRA_LIBS Strmiids.lib Mfplat.lib Mf.lib)
  # add_link_options(/DELAYLOAD:MF.lib)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-c11-extensions)
  set(EXTRA_LIBS fmt::fmt LibXml2::LibXml2)
  set(EXTRA_INC ${fmt_INCLUDE_DIRS} ${LibXml2_INCLUDE_DIRS})
endif()

if(OpenMP_CXX_FOUND)
  set(EXTRA_LIBS ${EXTRA_LIBS} OpenMP::OpenMP_CXX)
endif()

set(SRC
    src/find_files.cpp
    src/platform.cpp
    src/errors.cpp
    src/log.cpp
    src/store_error.cpp
    src/system_utils.cpp
    src/string_utils.cpp
    src/xml_factory_winrt.cpp
    src/xml_factory_linux.cpp
)

set(INC
    inc/find_files.hpp
    inc/platform.hpp
    inc/errors.hpp
    inc/log.hpp
    inc/store_error.hpp
    inc/system_utils.hpp
    inc/string_utils.hpp
    inc/args.hpp
    inc/config.hpp
    inc/xml_factory.hpp
)

add_library(${PROJECT_NAME} STATIC  ${SRC} ${INC})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC inc ${EXTRA_INC})
target_link_libraries(${PROJECT_NAME} PUBLIC ${EXTRA_LIBS} Threads::Threads)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

add_subdirectory(test)

# Binary and includes...
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${INC} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
