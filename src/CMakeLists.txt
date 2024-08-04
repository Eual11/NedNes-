set(EXECUTABLE_NAME "e")

include_directories(${CMAKE_SOURCE_DIR}/include $ENV{CPLUS_INCLUDE_PATH})


add_executable(${EXECUTABLE_NAME} ./main.cpp)
add_library(Ned6502 STATIC ./Ned6502.cpp)
add_library(Ned2C02 STATIC ./Ned2CO2.cpp)
add_library(NedBus STATIC ./NedBus.cpp)
add_library(NedCartridge STATIC ./NedCartridge.cpp)
add_library(NedMapper STATIC ./NedMapper.cpp)
add_library(Mapper000 STATIC ./Mapper000.cpp)


target_link_libraries(Mapper000 PRIVATE NedMapper )
target_link_libraries(NedCartridge PRIVATE Mapper000 PRIVATE NedMapper )
target_link_libraries(${EXECUTABLE_NAME} PRIVATE NedCartridge )

target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
