CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(wdk)

message(STATUS "
        ~~ Window Development Kit ~~

    \\\\o Brought to you by Ensisoft o//
        http://www.ensisoft.com
    Copyright (c) 2016 Sami Väisänen
              Ensisoft

https://github.com/ensisoft/wdk

")


# Only enable release and debug builds
IF(CMAKE_CONFIGURATION_TYPES)
  SET(CMAKE_CONFIGURATION_TYPES Debug Release)
  SET(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
    "Reset the configurations to what we need"
    FORCE)
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
    MESSAGE("Defaulting to Debug build...")
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_DEBUG_POSTFIX   "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX ""  CACHE STRING "add a postfix, usually empty on windows")

IF(CMAKE_BUILD_TYPE MATCHES "Release")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ENDIF()


# Solution
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

IF (WIN32)
    ADD_LIBRARY(wdk_system STATIC
        wdk/window_listener.cpp
        wdk/keys.cpp
        wdk/win32/pixmap.cpp
        wdk/win32/system.cpp
        wdk/win32/window.cpp)

    ADD_LIBRARY(wdk_desktop_gl STATIC
        wdk/opengl/WGL/config.cpp
        wdk/opengl/WGL/context.cpp
        wdk/opengl/WGL/surface.cpp
        wdk/opengl/WGL/fakecontext.cpp)

    # In order to build the mobile opengl library (using EGL)
    # you'll need to have an implementation of libEGL and libGLESv2.
    # These aren't available on Windows by default.
    #
    # There are several options available to for this:
    # - Imagination has Power VR SDK that can be downloaded from imgtech.com
    # - Google has GLES2/3 implementation called libANGLE
    # - Other companies might have similar SDKs
    #
    # For the time being if you're not specifying a specific SDK folder for GLES2
    # we're going to default to a Power VR SDK prebuilt libraris within this
    # git repository.
    IF (NOT GLES2_SDK_INCLUDE)
        SET(GLES2_SDK_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/third_party/PowerVR_SDK/SDK_2016_R1.2/Builds/Include")
        MESSAGE("Using Imagination PowerVR SDK headers")
    ENDIF()

    IF (NOT GLES2_SDK_LIBS)
        SET(ARCH_PATH "x86_32")
        #IF (${CMAKE_SYSTEM_PROCESSOR} MATCHES AMD64)
        IF(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
            SET(ARCH_PATH "x86_64")
        ENDIF()
        SET(GLES2_SDK_LIBS "${CMAKE_CURRENT_LIST_DIR}/third_party/PowerVR_SDK/SDK_2016_R1.2/Builds/Windows/${ARCH_PATH}/Lib/")
    ENDIF()

    INCLUDE_DIRECTORIES(BEFORE ${GLES2_SDK_INCLUDE})
    LINK_DIRECTORIES(${GLES2_SDK_LIBS})

    MESSAGE("GLES2 Include ${GLES2_SDK_INCLUDE}")
    MESSAGE("GLES2 Libs    ${GLES2_SDK_LIBS}")

    ADD_LIBRARY(wdk_mobile_gl STATIC
        wdk/opengl/EGL/config.cpp
        wdk/opengl/EGL/context.cpp
        wdk/opengl/EGL/egldisplay.cpp
        wdk/opengl/EGL/surface.cpp)
    TARGET_COMPILE_DEFINITIONS(wdk_mobile_gl PRIVATE "WDK_MOBILE")
    TARGET_LINK_LIBRARIES(wdk_mobile_gl PUBLIC
        libGLESv2 libEGL wdk_system)

ELSEIF(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    ADD_LIBRARY(wdk_system STATIC
        wdk/window_listener.cpp
        wdk/keys.cpp
        wdk/X11/keysym2ucs.cpp
        wdk/X11/types.cpp
        wdk/X11/pixmap.cpp
        wdk/X11/system.cpp
        wdk/X11/window.cpp)
    TARGET_LINK_LIBRARIES(wdk_system PUBLIC
        X11 xcb Xau Xdmcp Xxf86vm Xext Xrandr)

    ADD_LIBRARY(wdk_desktop_gl STATIC
        wdk/opengl/GLX/config.cpp
        wdk/opengl/GLX/context.cpp
        wdk/opengl/GLX/surface.cpp)
    TARGET_LINK_LIBRARIES(wdk_desktop_gl PUBLIC
        GL wdk_system)

    # Nothing special to be done before building EGL specific code
    ADD_LIBRARY(wdk_mobile_gl STATIC
        wdk/opengl/EGL/config.cpp
        wdk/opengl/EGL/context.cpp
        wdk/opengl/EGL/egldisplay.cpp
        wdk/opengl/EGL/surface.cpp)
    TARGET_COMPILE_DEFINITIONS(wdk_mobile_gl PRIVATE "WDK_MOBILE")
    TARGET_LINK_LIBRARIES(wdk_mobile_gl PUBLIC
        GLESv2 EGL wdk_system)

ENDIF()


# Build the sample applications

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_LIST_DIR})

# Open a window and print events to the console.
ADD_EXECUTABLE(SimpleEventSample sample/events.cpp)
TARGET_LINK_LIBRARIES(SimpleEventSample wdk_system)


# Open a window and draw using "big" desktop GL
ADD_EXECUTABLE(DesktopGLSample sample/triangle.cpp)
TARGET_LINK_LIBRARIES(DesktopGLSample
    wdk_system wdk_desktop_gl)


# Open a window and draw using "small" GL
ADD_EXECUTABLE(MobileGLSample sample/triangle.cpp)
TARGET_COMPILE_DEFINITIONS(MobileGLSample PRIVATE "SAMPLE_GLES")
TARGET_LINK_LIBRARIES(MobileGLSample
    wdk_system wdk_mobile_gl)

# Build unit tests
ADD_EXECUTABLE(UnitTestSystem wdk/unit_test/unit_test_wdk.cpp)
TARGET_LINK_LIBRARIES(UnitTestSystem wdk_system)

