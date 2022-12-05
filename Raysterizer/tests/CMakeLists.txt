function(add_test TEST_DIR TEST_NAME HAS_RESOURCES EXE)
        message(STATUS "\tConfiguring test ${BoldBlue}${TEST_NAME}${ColorReset} in ${Yellow}${TEST_DIR}${ColorReset}")

        #SET(original_build_type ${CMAKE_BUILD_TYPE})
        #set(CMAKE_BUILD_TYPE "Release")

        #set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
        add_depend(assimp https://github.com/assimp/assimp.git v5.0.1 TRUE)

        #set(CMAKE_BUILD_TYPE ${original_build_type})

        # source
        file(GLOB SOURCES "${TEST_DIR}/*.cpp")
        file(GLOB HEADERS "${TEST_DIR}/*.h")

	    if(${EXE})
            add_executable(${TEST_NAME} ${HEADERS} ${SOURCES} ${COMMON_HEADERS} ${COMMON_SOURCES})
        else()
            add_library(${TEST_NAME} SHARED ${HEADERS} ${SOURCES} ${COMMON_HEADERS} ${COMMON_SOURCES})
        endif()

        target_include_directories(${TEST_NAME} PRIVATE
	        Raysterizer

    	    ${assimp_SOURCE_DIR}
        )

        file(GLOB LIBS "${TEST_DIR}/*.lib")
        target_link_libraries(${TEST_NAME}
            ${LIBS}

            assimp
        )

        #target_link_libraries(${TEST_NAME} PUBLIC
        #    Raysterizer
        #)

        #set_target_properties(${TEST_NAME} PROPERTIES CXX_STANDARD 20)
        set_target_properties(${TEST_NAME} PROPERTIES CXX_STANDARD 17)
        set_target_properties(${TEST_NAME} PROPERTIES CXX_EXTENSIONS OFF)
        set_target_properties(${TEST_NAME} PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON)
        set_target_properties(${TEST_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/../")
        set_target_properties(${TEST_NAME} PROPERTIES FOLDER "Tests")

        copy_dlls(${TEST_DIR})

        if(MSVC)
                set_target_properties(${TEST_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
        endif()

        #copy resources to binary
	    if(${HAS_RESOURCES})
            file(COPY "${TEST_DIR}/resources" DESTINATION "${CMAKE_BINARY_DIR}/tests/")
        else()
            file(GLOB VERTEX_SHADERS "${TEST_DIR}/*.vs")
            file(COPY ${VERTEX_SHADERS} DESTINATION "${CMAKE_BINARY_DIR}/tests/")
            file(GLOB FRAGMENT_SHADERS "${TEST_DIR}/*.fs")
            file(COPY ${FRAGMENT_SHADERS} DESTINATION "${CMAKE_BINARY_DIR}/tests/")
        endif()

endfunction(add_test)

add_test(bare Bare FALSE TRUE)
add_test(simple_opengl Demo TRUE TRUE)
add_test(8.advanced_glsl_ubo 8.advanced_glsl_ubo FALSE TRUE)
#add_test(10.1.instancing_quads 10.1.instancing_quads FALSE)
add_test(10.3.asteroids_instanced 10.3.asteroids_instanced FALSE TRUE)
add_test(5.1.framebuffers 5.1.framebuffers FALSE TRUE)
add_test(nomad_inject nomad_inject FALSE FALSE)

