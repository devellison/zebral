# Zebral library

## WORK IN PROGRESS
The Zebral Library is currently under active development.

The initial focus is ZebralCam, to provide crossplatform access to web cameras with easy access to controls and hardware identification.  It works now (Windows/Ubuntu), but is still under active development.

So far, there are 3 libraries:
 - zebralcommon: Base errors, logging, etc. used in Zebral libraries
 - zebralcam: Provides crossplatform cameras with control options and hardware identification.
 - zebralserial: Will provide some basic serial communications classes

Zebral is being written as a part of the Lightbox project, but it's intended to stand
alone as well once it's done.

You should be able to build this directory solo, or integrate it easily with other projects.
Check out the [Lightbox Project](https://github.com/devellison/lightbox) for an example.

## Dependencies
NOTE: ZebralCam and other libraries don't currently depend on OpenCV and can be linked against and used without it,
but some of the tests and utilities DO require it.

- Install dependencies:
  - As it's actively being developed, might check the [workflows](https://github.com/devellison/zebral/tree/main/.github/workflows) for latest requirements for building.
  - Windows:
    - Visual Studio 2019 (CMake 3.20+)  (Others may work - 2022 in CI)
    - Set up OpenCV 4.5.5, including videoio, highgui, and core 
      (current binary release didn't seem to have these, but choco did)
    - Doxygen, graphviz, cmake-format (pip install), clang-format
    - libCurl (vcpkg install curl)
  - Linux:
    - Currently using Clang-12 (CMake 3.23)
    - Using OpenCV 4.2 from Ubuntu's distribution
    - libfmt (on Linux) should be built and installed
    - Google test is downloaded by CMake currently
  - Build tools and deps for both:    
    - CMake v3.20+ (you can get it from kitware for ubuntu)
    - OpenMP (optional, not using it much yet)
    - Doxygen, graphviz, cpack, cmake-format (pip install), clang-format
    - Python 3.10.5+ and PyBind11 for python bindings
    - Numpy and OpenCV-python (pip installs) are required for the python tests
