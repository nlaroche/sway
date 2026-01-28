cmake_minimum_required(VERSION 3.22)
project(Sway VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SWAY_DEV_MODE "Enable development mode (hot reload from Vite)" OFF)
option(BEATCONNECT_ENABLE_ACTIVATION "Enable BeatConnect activation system" OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "BeatConnect"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Beat
    PLUGIN_CODE Sway
    FORMATS VST3 Standalone
    PRODUCT_NAME "Sway"
    NEEDS_WEBVIEW2 TRUE
)

target_sources(${PROJECT_NAME}
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/ParameterIDs.h
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        $<IF:$<BOOL:${SWAY_DEV_MODE}>,SWAY_DEV_MODE=1,SWAY_DEV_MODE=0>
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_USE_WIN_WEBVIEW2=1)
endif()

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
    target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_USE_WKWEBVIEW=1)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(EXISTS "${CMAKE_SOURCE_DIR}/resources/project_data.json")
    juce_add_binary_data(${PROJECT_NAME}_ProjectData
        HEADER_NAME "ProjectData.h"
        NAMESPACE ProjectData
        SOURCES resources/project_data.json
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_ProjectData)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_PROJECT_DATA=1)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_PROJECT_DATA=0)
endif()

if(BEATCONNECT_ENABLE_ACTIVATION AND EXISTS "${CMAKE_SOURCE_DIR}/beatconnect-sdk/sdk/activation/CMakeLists.txt")
    add_subdirectory(beatconnect-sdk/sdk/activation)
    target_link_libraries(${PROJECT_NAME} PRIVATE beatconnect_activation)
    target_compile_definitions(${PROJECT_NAME} PUBLIC BEATCONNECT_ACTIVATION_ENABLED=1)
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC BEATCONNECT_ACTIVATION_ENABLED=0)
endif()

if(NOT SWAY_DEV_MODE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/Resources/WebUI/index.html")
        juce_add_binary_data(${PROJECT_NAME}_WebUI
            HEADER_NAME "WebUIData.h"
            NAMESPACE WebUIData
            SOURCES
                Resources/WebUI/index.html
        )
        file(GLOB_RECURSE WEB_ASSETS "${CMAKE_SOURCE_DIR}/Resources/WebUI/assets/*")
        if(WEB_ASSETS)
            target_sources(${PROJECT_NAME}_WebUI PRIVATE ${WEB_ASSETS})
        endif()
        target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_WebUI)
        target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_WEB_UI_DATA=1)
    else()
        target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_WEB_UI_DATA=0)
    endif()
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_WEB_UI_DATA=0)
endif()
