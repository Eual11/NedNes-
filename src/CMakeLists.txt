set(EXECUTABLE_NAME "e")

include_directories(${CMAKE_SOURCE_DIR}/include $ENV{CPLUS_INCLUDE_PATH})


add_executable(${EXECUTABLE_NAME} ./main.cpp)

target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
