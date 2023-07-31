# GroovyEngine
Game engine with editor, currently in development, important stuff missing like physics, audio etc...

# How to build
The engine was built from the ground up with cross platform support in mind, but it currently runs only on Windows.
To build the Visual Studio 2022 solution just run "GenWinProjects.bat", open the .sln file, build and run!
Important note: if you change your build configuration from Editor to Shipping/DebugGame and vice versa you'll have to also change the startup project.

# Prerequisites
- Make sure to install all DirectX stuff or it will crash

# Cool stuff
- C++ reflection and serialization (inspired by Unreal)

# Libs used
- Dear ImGui
- stbi
- tinyobj
