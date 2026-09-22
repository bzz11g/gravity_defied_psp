cmake_minimum_required(VERSION 3.24)
project(gravity_defied_cpp C CXX)

execute_process(
    COMMAND psp-config --psp-prefix
    OUTPUT_VARIABLE PSP_PREFIX_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

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
    src/psp/glib2d.c
    src/psp/intra/intraFont.c
    src/psp/intra/libccc.c
    src/psp/pspSetup.cpp
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

add_executable(GravityDefied ${SOURCES})

target_compile_features(GravityDefied PRIVATE cxx_std_17)
target_compile_options(GravityDefied PRIVATE -Wall -Wextra -fno-exceptions -fno-rtti)

target_include_directories(GravityDefied PRIVATE
                           src
                           src/psp
                           src/psp/intra
                           ${PSP_PREFIX_DIR}/include/freetype2
                           $ENV{PSPDEV}/psp/include/freetype2)

set_target_properties(GravityDefied PROPERTIES LINK_FLAGS_RELEASE -s)

target_link_libraries(GravityDefied PRIVATE
                      png jpeg bz2 pspgu pspgum pspdisplay pspge pspctrl psppower psprtc pspvram
                      freetype z m stdc++
                      Assets::Assets)

# Copy assets to the build directory
file(COPY assets/levels.mrg DESTINATION ${CMAKE_BINARY_DIR})

if(PSP OR DEFINED ENV{PSPDEV})
    create_pbp_file(
        TARGET GravityDefied
        TITLE "Gravity Defied: Trial Racing"
        ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/ICON0.png"
        PREVIEW_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/PIC0.png"
        BACKGROUND_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/PIC1.png"
    )
endif()
