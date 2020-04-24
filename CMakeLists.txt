cmake_minimum_required(VERSION 2.8)
PROJECT(barrier)
SET(CMAKE_CXX_STANDARD 11)


ADD_COMPILE_DEFINITIONS(WITH_CDS)
INCLUDE_DIRECTORIES("/home/mintyi/codework/model-checker/include")
LINK_DIRECTORIES("/home/mintyi/codework/model-checker")

ADD_EXECUTABLE(barrier main.cpp mybarrier.hpp)
TARGET_LINK_LIBRARIES(barrier "model")

#TARGET_LINK_LIBRARIES(barrier pthread)

