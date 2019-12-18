add_executable(VideoPlayer
    main.cpp

    pch.h
    pch.cpp
)

get_target_property(VideoPlayers VideoPlayer SOURCES)
epi_add_precompiled_header(VideoPlayer pch.h SOURCE_CXX pch.cpp)
epi_pack_sources(${VideoPlayer_SRCs})

target_link_libraries(VideoPlayer
    PUBLIC
        SDL2::SDL2
        EpiCore
        EpiMultiMedia
)

add_custom_command(TARGET VideoPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:VideoPlayer>
)

get_target_property(EpiMultiMedia_DLLs EpiMultiMedia EPI_DLL_LIBS)
foreach(dll ${EpiMultiMedia_DLLs})
    add_custom_command(TARGET VideoPlayer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dll} $<TARGET_FILE_DIR:VideoPlayer>
    )
endforeach()

target_include_directories(VideoPlayer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

set_target_properties(VideoPlayer
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)
