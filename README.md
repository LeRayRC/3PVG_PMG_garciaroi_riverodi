# 3PVG_PMG_garciaroi_riverodi

## 1 - Dependencies
- **Vulkan**: [download](https://vulkan.lunarg.com/)
- **Cmake**
- **Visual Studio 2022**: C++23
- **Python**: Conan1 library.

## 2 - Download and compile OpenXR

```bash
./build_openxr.bat
```

## 3 - Generate dependencies

```bash
cd tools
gendeps.bat
```
## 4 - Compile Shaders

```bash
cd tools
compileshaders.bat
```

## 5 - Generate sln solution
Project root

```bash
./tools/premake5.exe vs2022 --build-engine --include-examples
```

## 6 - Open sln solution at build folder

```bash
cd build
Lava.sln
```





