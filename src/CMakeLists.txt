set(EXECUTABLE_NAME "e")

include_directories(${CMAKE_SOURCE_DIR}/include $ENV{CPLUS_INCLUDE_PATH})


add_executable(${EXECUTABLE_NAME} ./main.cpp)
add_library(Ned6502 STATIC ./Ned6502.cpp)
add_library(NedBus ./NedBus.cpp)

add_library(NedNes STATIC ./NedNes.cpp)
target_link_libraries(NedNes PRIVATE Ned6502 PRIVATE NedBus)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE NedNes )

target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
