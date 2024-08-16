cmake_minimum_required(VERSION 3.15)
project(ChaosAttractors VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML package, you might need to set CMAKE_PREFIX_PATH to /opt/homebrew/Cellar/sfml/2.6.1
find_package(SFML 2.6 COMPONENTS system window graphics audio network REQUIRED)

# Add source files
file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

# Link SFML libraries
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-window sfml-system sfml-audio sfml-network)

# On macOS, you might need to bundle the SFML frameworks within the app bundle
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_SKIP_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
        MACOSX_FRAMEWORK_IDENTIFIER com.example.ChaosAttractorMusicVisualizer
        RESOURCE "${CMAKE_SOURCE_DIR}/assets"
    )

    install(TARGETS ${PROJECT_NAME}
            BUNDLE DESTINATION "."
            RUNTIME DESTINATION bin)
endif()

# For Windows, ensure that the SFML DLLs are copied to the output directory
if(WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ".")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sfml-graphics>
        $<TARGET_FILE:sfml-window>
        $<TARGET_FILE:sfml-system>
        $<TARGET_FILE:sfml-audio>
        $<TARGET_FILE:sfml-network>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Install the executable
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
