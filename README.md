# GroovyEngine
Game engine with editor.
![](https://github.com/massitommi/GroovyEngine/blob/master/groovy.gif)

# Cool Stuff
- Native C++ scripting, reflection and serialization system. (Inspired by Unreal)
- Drag & Drop to import assets
- Blueprints / Prefabs

# Missing stuff
- Physics
- UI

# How to build
The engine is structured to support multiple platforms but currently only Windows is implemented.
To build the Visual Studio 2022 solution just run "GenWinProjects.bat".
Launching the engine from your IDE will load the DemoProject (if you want to see demo game-code running, you should also build this after the engine, .bat is inside /DemoProject). 
You can create new projects with the ProjectCreator tool, then launch the engine with the bat files.

# How to create your own Groovy class (Actor, ActorComponent, etc...)

GroovyClass is the base class that supports reflection and serialization. Actor and ActorComponent inherit from GroovyClass.
In order to make your class "Groovy" you have to use a couple of macros.
To make the engine aware of the existance of that class add it to the list inside game_classes.cpp

Take a look at the DemoProject for examples.

# Prerequisites
- DirectX stuff

# Libs used
- Dear ImGui
- stbi
- tinyobj
- FMOD
