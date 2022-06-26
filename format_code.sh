#!/bin/bash
set -e
pushd "$(dirname "$0")" 
echo "Formatting Code...."
clang-format -i app/*.cpp
clang-format -i test/*.cpp
clang-format -i src/*.cpp
clang-format -i inc/*.hpp

clang-format -i camera/src/*.cpp
clang-format -i camera/inc/*.hpp
clang-format -i camera/test/*.cpp

clang-format -i common/src/*.cpp
clang-format -i common/inc/*.hpp
clang-format -i common/test/*.cpp

clang-format -i serial/src/*.cpp
clang-format -i serial/inc/*.hpp
clang-format -i serial/test/*.cpp

echo "Formatting CMake..."
cmake-format -i CMakeLists.txt
cmake-format -i app/CMakeLists.txt
cmake-format -i test/CMakeLists.txt

cmake-format -i common/CMakeLists.txt
cmake-format -i common/test/CMakeLists.txt

cmake-format -i camera/CMakeLists.txt
cmake-format -i camera/test/CMakeLists.txt

cmake-format -i serial/CMakeLists.txt
cmake-format -i serial/test/CMakeLists.txt

popd
