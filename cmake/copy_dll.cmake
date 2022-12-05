if(MSVC)
    set_property(GLOBAL PROPERTY RaysterizerDlls)
endif()

function(append_dlls)
    if(MSVC)
        set_property(GLOBAL APPEND PROPERTY RaysterizerDlls ${ARGV})
    endif()
endfunction()

function(copy_dlls target)
    if(MSVC)
        get_property(RaysterizerDlls GLOBAL PROPERTY RaysterizerDlls)
        foreach(dll IN LISTS RaysterizerDll)
            add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${dll} $(OutDir))
        endforeach()
    endif()
endfunction()