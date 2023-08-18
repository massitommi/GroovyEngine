# GroovyEngine
Game engine with editor.

# Cool stuff
After playing around with Unreal I was fascinated by their reflection and serialization system.
I wanted to recreate that system and so I did.
- You can create your own classes (Actors, ActorComponents) and reflect properties in the editor with a simple macro (arrays and dynamic arrays supported!)
- You can then create blueprints and all the serialization is done automagically just like in Unreal.

# Game engine stuff
- Rendering: the renderer is really bare bone, just meshes with albedo textures. No lighting, no shadows, no fancy rendering technique. 
- Assets: you can easily import textures and meshes with a drag and drop, they will be converted in a custom binary format and ready to use.
- Missing stuff: Audio and Physics

# How to build
The engine is structured to support multiple platforms but currently only Windows is implemented.
To build the Visual Studio 2022 solution just run "GenWinProjects.bat", open the .sln file, build and run!
Important note: If you change your build configuration from Editor to Shipping/DebugGame and vice versa you'll have to also change the startup project.

# Prerequisites
- DirectX stuff

# Libs used
- Dear ImGui
- stbi
- tinyobj
