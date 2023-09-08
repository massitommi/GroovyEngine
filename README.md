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
- Audio: you can play audio clips, thats all.
- Missing stuff: Physics

# How to build
The engine is structured to support multiple platforms but currently only Windows is implemented.
To build the Visual Studio 2022 solution just run "GenWinProjects.bat", open the .sln file, and build!
Note: Make sure you build the right project with the right configuration (dont' build Sandbox with Editor_ configurations).

# How to run
The engine in order to run needs a project and the game code (If you build for shipping everything is statically linked, game code included; needs some setup).
If you just want to try out the engine you should build the DemoProject because your IDE will launch the engine with that one.
If for some strange, obscure, dumb reason you want to make a game with it, you can create your projects with the ProjectCreator tool and start the engine with the .bat files inside your project folder.

# Prerequisites
- DirectX stuff

# Libs used
- Dear ImGui
- stbi
- tinyobj
- FMOD
