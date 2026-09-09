cmake_minimum_required(VERSION 3.24)
project(gravity_defied_cpp C CXX)

unset(ENV{PKG_CONFIG_PATH})

if(WIN32)
    include(cmake/mingw-sdl2.cmake)
    download_mingw_sdl2(
        "2.26.1"    # SDL2
        "2.20.1"    # SDL2_ttf
        "2.6.2"     # SDL2_image
    )
endif()

find_package(PkgConfig REQUIRED)

pkg_search_module(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_search_module(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
pkg_search_module(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    src/utils/Time.cpp
    src/utils/EmbedFileStream.cpp
    src/main.cpp
    src/MathF16.cpp
    src/GameCanvas.cpp
    src/GamePhysics.cpp
    src/TimerOrMotoPartOrMenuElem.cpp
    src/class_10.cpp
    src/GameLevel.cpp
    src/LevelLoader.cpp
    src/Micro.cpp
    src/TextRender.cpp
    src/GameMenu.cpp
    src/SettingsStringRender.cpp
    src/MenuManager.cpp
    src/RecordManager.cpp
    src/Timer.cpp
    src/lcdui/CanvasImpl.cpp
    src/lcdui/Canvas.cpp
    src/lcdui/Graphics.cpp
    src/lcdui/Image.cpp
    src/lcdui/Font.cpp
    src/lcdui/Command.cpp
    src/lcdui/FontStorage.cpp
    src/rms/RecordEnumerationImpl.cpp
    src/rms/RecordStore.cpp
    src/rms/PSPSavedata.cpp
)

include(cmake/CMakeRC.cmake)

cmrc_add_resource_library(Assets
    NAMESPACE assets
    ALIAS Assets::Assets
    WHENCE assets
    assets/bluearm.png
    assets/helmet.png
    assets/sprites.png
    assets/blueleg.png
    assets/bluebody.png
    assets/logo.png
    assets/fender.png
    assets/levels.mrg
    assets/engine.png
    assets/splash.png
    assets/raster.png
    assets/FontSansSerif.ttf
)

# Compile resources when building for Windows.
if(WIN32)
    enable_language(RC)
    list(APPEND SOURCES winres/res.rc)
endif()

add_executable(GravityDefied ${SOURCES})

target_compile_features(GravityDefied PRIVATE cxx_std_17)
target_compile_options(GravityDefied PRIVATE -Wall -Wextra)

target_include_directories(GravityDefied PRIVATE
                           ${SDL2_INCLUDE_DIRS}
                           ${SDL2_TTF_INCLUDE_DIRS}
                           ${SDL2_IMAGE_INCLUDE_DIRS})

target_link_directories(GravityDefied PRIVATE
                        ${SDL2_LIBRARY_DIRS}
                        ${SDL2_TTF_LIBRARY_DIRS}
                        ${SDL2_IMAGE_LIBRARY_DIRS})

set_target_properties(GravityDefied PROPERTIES LINK_FLAGS_RELEASE -s)

if(WIN32)
    target_link_options(GravityDefied PRIVATE -static -mwindows)
    target_link_libraries(GravityDefied PRIVATE
                         ${SDL2_STATIC_LIBRARIES} 
                         ${SDL2_TTF_STATIC_LIBRARIES}
                         ${SDL2_IMAGE_STATIC_LIBRARIES} 
                         Assets::Assets)
else()
    target_link_libraries(GravityDefied PRIVATE
                          ${SDL2_LIBRARIES}
                          ${SDL2_TTF_LIBRARIES}
                          ${SDL2_IMAGE_LIBRARIES}
                          Assets::Assets)
endif()

# Copy assets to the build directory
file(COPY assets/levels.mrg DESTINATION ${CMAKE_BINARY_DIR})

if(PSP)
    target_link_libraries(GravityDefied PRIVATE
                          pspctrl pspgu pspgum pspdisplay psppower psprtc m stdc++)

    create_pbp_file(
        TARGET GravityDefied
        TITLE "Gravity Defied: Trial Racing"
        ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/ICON0.png"
        PREVIEW_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/PIC0.png"
        BACKGROUND_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/PIC1.png"
    )
endif()

if(PSP)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/embed_sfo.py "import sys\nwith open(sys.argv[1], 'rb') as f:\n    d = f.read()\nwith open(sys.argv[2], 'w') as f:\n    f.write('#pragma once\\n')\n    f.write('static const unsigned char default_sfo_data[] = {' + ','.join(hex(b) for b in d) + '};\\n')\n    f.write('static const unsigned int default_sfo_data_len = ' + str(len(d)) + ';\\n')\n")

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/rms/sfo_data.h
        COMMAND python3 ${CMAKE_CURRENT_BINARY_DIR}/embed_sfo.py ${CMAKE_CURRENT_SOURCE_DIR}/assets/PARAM.SFO ${CMAKE_CURRENT_SOURCE_DIR}/src/rms/sfo_data.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/PARAM.SFO
        COMMENT "Embedding PARAM.SFO into sfo_data.h"
    )

    # Make the target depend on the generated header
    set_source_files_properties(src/rms/PSPSavedata.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/rms/sfo_data.h)

    # We must trigger this custom command before building
    add_custom_target(GenerateSFOHeader ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/rms/sfo_data.h)
    add_dependencies(GravityDefied GenerateSFOHeader)
endif()
