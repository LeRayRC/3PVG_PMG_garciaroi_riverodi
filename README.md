# 3PVG_PMG_garciaroi_riverodi

## 1 - Dependencies
- **Vulkan**: [download](https://vulkan.lunarg.com/)
- **Cmake**
- **Visual Studio 2022**: C++23
- **Python**: Conan1 library.

## 2 - Generate dependencies

```bash
cd tools
gendeps.bat
```
## 3 - Compile Shaders

```bash
cd tools
compileshaders.bat
```

## 4 - Generate sln solution
Project root

```bash
./tools/premake5.exe vs2022
```

## 5 - Open sln solution at build folder





