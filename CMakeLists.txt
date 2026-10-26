cmake_minimum_required(VERSION 3.16)
project(AsyncTimerHostTests LANGUAGES CXX)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
