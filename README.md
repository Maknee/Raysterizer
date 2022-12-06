# Raysterizer

<img src="images/dolphin.gif" width="100%" height="100%"/>

Framework that transforms existing opengl programs instead use raytracing.

- This project is very much in a work-in progress, expect many bugs and crashes

## Demo

[![](https://img.youtube.com/vi/iuHRDvmhX9Y/0.jpg)](https://youtu.be/iuHRDvmhX9Y)

## Explanation

[Blog](https://maknee.github.io/blog/2022/Raysterizer/#vulkan-ray-tracing)

## Supported Platforms

- Windows

## Running Examples

Use the correct configuration per example below

### LearnOpenGL (tutorials from https://learnopengl.com/)

Download the [latest release](https://github.com/Maknee/Raysterizer/releases/)

Download `learnopengl.zip` from [releases](https://github.com/Maknee/Raysterizer/releases/)

### Craft (Minecraft clone)

[Download craft](https://github.com/fogleman/Craft) and compile it following the instructions on the github

Download `runelite.zip` from [releases](https://github.com/Maknee/Raysterizer/releases/)

### Runescape

Runelite has to be compiled from source

Follow [installation video to compile runelite from source](https://www.youtube.com/watch?v=-eTTrlFoKPc)

Download `runelite.zip` from [releases](https://github.com/Maknee/Raysterizer/releases/)

Extract `runelite.zip` to the `runelite` directory

Run `update.cmd` to copy the update files to runelite

Compile and run runelite

Enable GPU plugin from runelite settings

### Dolphin

Download [modified version of dolphin](https://github.com/Maknee/RaysterizerDolphin)

```
git clone https://github.com/Maknee/RaysterizerDolphin
git submodule update --init
```

Compile `dolphin` by opening `Source/dolphin-emu.sln` and building under `Release`

Download `dolphin.zip` from [releases](https://github.com/Maknee/Raysterizer/releases/)

Extract `dolphin.zip` to the `dolphin\Binary\x64` directory

Edit the `dolphin` field in `raysterizer_config.json` to specify which game you wish to play (some games have edits to filter out some polygons). The games are found under `draw_calls.cpp` in the Raysterizer source. 

## Compilation

### Dependencies

- [Vulkan SDK](https://vulkan.lunarg.com/)
- [CMake](https://cmake.org/download/)

### MSVC Compliation

```
git clone https://github.com/Maknee/Raysterizer.git
cd Raysterizer
mkdir build
cd build
cmake ..
msbuild.exe Raysterizer.sln /p:Configuration=Release
or open Raysterizer.sln with Visual Studio and build Raysterizer

Then copy the contents of BuildDependencies/ to the build/bin/ directory
```

## Configuration

`TODO`

## License

GPL-3
