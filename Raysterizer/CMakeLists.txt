set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RAYSTERIZER_NAME "Raysterizer")
set(RAYSTERIZER_DIR "Raysterizer")

IF(MSVC)
    SET(CMAKE_CXX_FLAGS "/EHsc")
ENDIF(MSVC)

message(STATUS "\tConfiguring test ${BoldBlue}${RAYSTERIZER_NAME}${ColorReset} in ${Yellow}${RAYSTERIZER_DIR}${ColorReset}")

# source
#file(GLOB SOURCES "${RAYSTERIZER_DIR}/include/*.h")
#file(GLOB HEADERS "${RAYSTERIZER_DIR}/src/*.cpp")
include_directories(${RAYSTERIZER_DIR})
file(GLOB_RECURSE RAYSTERIZER_SOURCE ${RAYSTERIZER_DIR}/*.cpp ${RAYSTERIZER_DIR}/*.h ${RAYSTERIZER_DIR}/*.def)

add_library(${RAYSTERIZER_NAME} SHARED ${RAYSTERIZER_SOURCE} ${COMMON_HEADERS} ${COMMON_SOURCES})

target_compile_options(${RAYSTERIZER_NAME} PRIVATE "$<$<CONFIG:Release>:/Zi>")
target_link_options(${RAYSTERIZER_NAME} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(${RAYSTERIZER_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(${RAYSTERIZER_NAME} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")

target_precompile_headers(${RAYSTERIZER_NAME} PUBLIC ${RAYSTERIZER_DIR}/pch.h)

target_include_directories(${RAYSTERIZER_NAME} PRIVATE
	RaysterizerEngine

    ${spirv_tools_SOURCE_DIR}/include
    ${spirv_tools_SOURCE_DIR}
)

file(GLOB LIBS "${RAYSTERIZER_DIR}/*.lib")
target_link_libraries(${RAYSTERIZER_NAME}
    ${LIBS}

    RaysterizerEngine
    #assimp
)
        
#set_target_properties(${RAYSTERIZER_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${RAYSTERIZER_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${RAYSTERIZER_NAME} PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(${RAYSTERIZER_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)
set_target_properties(${RAYSTERIZER_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/../")
set_target_properties(${RAYSTERIZER_NAME} PROPERTIES FOLDER ${RAYSTERIZER_DIR})

copy_dlls(${RAYSTERIZER_DIR})

if(MSVC)
    add_definitions(/MP)

    set_target_properties(${RAYSTERIZER_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
    
    add_custom_command(TARGET ${RAYSTERIZER_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${RAYSTERIZER_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/"opengl32.dll"
    )
endif()

add_subdirectory(tests)
