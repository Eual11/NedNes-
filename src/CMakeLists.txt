
set(EXECUTABLE_NAME "NedNes")
include_directories(${CMAKE_SOURCE_DIR}/include $ENV{CPLUS_INCLUDE_PATH})
set(NED_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(SDL2 ${NED_LIB_DIR}/SDL2.lib)
message("SDL2 is ${SDL2}")
set(SDL2_Main ${NED_LIB_DIR}/SDL2main.lib)
set(SDL2_ttf ${NED_LIB_DIR}/SDL2_ttf.lib)
set(SDL2_image ${NED_LIB_DIR}/SDL2_image.lib)

if(WIN32)
  add_executable(${EXECUTABLE_NAME} ./Manager.cpp icon.rc)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
else()
  add_executable(${EXECUTABLE_NAME} ./Manager.cpp)
endif()


if(WIN32)
    add_definitions(-DSDL_STATIC)  # This tells SDL2 to link statically

    
        
    # Optionally, disable the dynamic CRT
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
endif()
add_library(Ned6502 STATIC ./Ned6502.cpp)
add_library(Ned2C02 STATIC ./Ned2CO2.cpp)
add_library(NedBus STATIC ./NedBus.cpp)
add_library(Ned2A03 STATIC ./Ned2A03.cpp)
add_library(NedCartridge STATIC ./NedCartridge.cpp)
add_library(NedMapper STATIC ./NedMapper.cpp)
add_library(Mapper000 STATIC ./Mapper000.cpp)
add_library(Mapper001 STATIC ./Mapper001.cpp)
add_library(Mapper002 STATIC ./Mapper002.cpp)
add_library(Mapper003 STATIC ./Mapper003.cpp)
add_library(Mapper004 STATIC ./Mapper004.cpp)
add_library(RenderUtils ./RenderUtils.cpp)
add_library(JoyPad ./NedJoypad.cpp)
add_library(NedNesEmulator STATIC ./NedNes.cpp)
add_library(NedManager ./NedManager.cpp)

target_link_libraries(RenderUtils PRIVATE ${SDL2} PRIVATE ${SDL2_Main} PRIVATE ${SDL2_ttf})
target_link_libraries(Mapper000 PRIVATE NedMapper )
target_link_libraries(Mapper001 PRIVATE NedMapper )
target_link_libraries(Mapper002 PRIVATE NedMapper )
target_link_libraries(Mapper003 PRIVATE NedMapper )
target_link_libraries(Mapper004 PRIVATE NedMapper )
target_link_libraries(NedCartridge PRIVATE Mapper000 PRIVATE Mapper001 PRIVATE Mapper002 PRIVATE Mapper003 PRIVATE Mapper004 PRIVATE NedMapper )
target_link_libraries(NedNesEmulator PRIVATE NedBus PRIVATE Ned6502  PRIVATE Ned2A03 PRIVATE Ned2C02 PRIVATE Ned2A03 PRIVATE NedCartridge PRIVATE JoyPad PRIVATE ${SDL2} PRIVATE ${SDL2_Main} PRIVATE ${SDL2_ttf} PRIVATE RenderUtils  )
target_link_libraries(NedManager PRIVATE NedNesEmulator  PRIVATE ${SDL2} PRIVATE ${SDL2_Main} PRIVATE ${SDL2_ttf} PRIVATE ${SDL2_image} PRIVATE RenderUtils  )
target_link_libraries(${EXECUTABLE_NAME} PRIVATE NedManager  )

target_compile_features(${EXECUTABLE_NAME} PRIVATE cxx_std_17)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
