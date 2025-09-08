# glRobotInterfaceAPI

![C++](https://img.shields.io/badge/C++-17-blue)
![CMake](https://img.shields.io/badge/CMake-3.20+-orange)

glRobotInterface is a C++ API for visualizing (FUTURE: and controlling) robots defined via URDF files and ran in the real-world. 
It renders URDF Scenes using OpenGL on a separate thread and provides an API to link real-time encoder values to the model joints to move in real-time.

<img width="1194" height="688" alt="image" src="https://github.com/user-attachments/assets/04729483-250f-45fe-952f-731f6e753886" />

---

## Quick Start

### Clone the repository
```bash
git clone https://github.com/{user}/glRobotInterfaceAPI.git
cd glRobotInterfaceAPI
```
### Build with CMake
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Debug
```
### Run
* Windows: Double-click `glRobotInterface.bat` in the project root or run `build/Debug/glRobotInterface.exe`.
* Linux/macOS: Run `./glRobotInterface.sh` or `./buildglRobotInterface`

---

## Features
* Load custom URDF files and associated STL meshes
* Free-cam with pan and orbit mouse control (similar to Blender or Autodesk)
* Link encoder values to joints to view a live simulation
  * Only real use right now is for maybe remote monitoring... more to be added in future

---

## Future Improvements
* Better API Packaging
  * Currently, the API is demo'd in `main.cpp` and I haven't fully flushed it out for real use.
* Support multiple robots loaded simultaneously
* Better textures and URDF basic geometry support
* Better shader effects/looks (lighting, wireframe, materials)
* GUI for interactive parameter adjustment

  
