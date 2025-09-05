cmake_minimum_required(VERSION 3.20)
project(glRobotInterfaceAPI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Recommend high warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_executable(glRobotInterface
    src/main.cpp

    src/core/mesh.cpp
    src/core/model.cpp
    src/core/urdf.cpp
    src/core/mesh.h
    src/core/model.h
    src/core/urdf.h
    src/core/camera.h

    src/interface/interface.cpp
    src/interface/interface.h
)

target_include_directories(glRobotInterface
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
)

# On MSVC, prefer main CRT entry point
if(MSVC)
    target_compile_definitions(glRobotInterface PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()


# Include dirs for libraries that need them (glm is header-only)
target_link_libraries(glRobotInterface PRIVATE
    glfw
    glad::glad
    glm::glm-header-only
    assimp::assimp
)

# Copy entire assets folder
add_custom_command(TARGET glRobotInterface POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets/models"
        "$<TARGET_FILE_DIR:glRobotInterface>/models"
)

# Copy urdf folder
add_custom_command(TARGET glRobotInterface POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets/urdf"
        "$<TARGET_FILE_DIR:glRobotInterface>/urdf"
)

# Copy shaders folder
add_custom_command(TARGET glRobotInterface POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/shaders"
        "$<TARGET_FILE_DIR:glRobotInterface>/shaders"
)



if(WIN32) # Creates a shortcut in the source dir
    add_custom_command(TARGET glRobotInterface POST_BUILD
        COMMAND powershell -Command "$s=(New-Object -COM WScript.Shell).CreateShortcut('${CMAKE_SOURCE_DIR}/glRobotInterface.lnk'); $s.TargetPath='$<TARGET_FILE:glRobotInterface>'; $s.WorkingDirectory='$<TARGET_FILE_DIR:glRobotInterface>'; $s.Save()"
    )
elseif(UNIX) # Creates a shell script in the source dir (Since some might need to run this from linux or mac)
    add_custom_command(TARGET glRobotInterface POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "#!/bin/sh\n\"$<TARGET_FILE:glRobotInterface>\" \"\$@\"" > "${CMAKE_SOURCE_DIR}/runRobotInterface.sh"
        COMMAND ${CMAKE_COMMAND} -E chmod +x "${CMAKE_SOURCE_DIR}/runRobotInterface.sh"
    )
endif()