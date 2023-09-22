# GroovyEngine
Game engine with editor.

# Cool Stuff
- Native C++ scripting, reflection and serialization system. (Inspired by Unreal)
- Drag & Drop to import assets
- Blueprints / Prefabs

# Missing stuff
- Physics

# How to build
The engine is structured to support multiple platforms but currently only Windows is implemented.
To build the Visual Studio 2022 solution just run "GenWinProjects.bat".
Launching the engine from your IDE will load the DemoProject.
You can create new projects with the ProjectCreator tool, then launch the engine with the bat files.

# How to create your own Groovy class (Actor, ActorComponent, etc...)

GroovyClass is the base class that supports reflection and serialization. Actor and ActorComponent inherit from GroovyClass.

In order to make your class groovy you have to place a couple of macros in your class declaration...

Example_Actor.h
```C++
#include "gameframework/actor.h"

GROOVY_CLASS_DECL(ExampleActor)  // First macro with your class as parameter
class ExampleActor : public Actor
{
    GROOVY_CLASS_BODY(ExampleActor, Actor) // Second macro with your class and the Super class as parameters

    // stuff
    float Health;
    std::vector<std::string> Comments;
    Texture* CoolTexture;
};
```

And in your implementation...

Example_Actor.cpp
```C++
GROOVY_CLASS_IMPL(MyActor)
    // Reflect here your properties to make them visible inside the Editor and serialized automagically
    GROOVY_REFLECT(Health)
    GROOVY_REFLECT(Comments)
    GROOVY_REFLECT(CoolTexture)
GROOVY_CLASS_END()
```

To make the engine aware of the existance of that class add it to the list inside game_classes.cpp

game_classes.cpp
```C++
#include "game_api.h"
#include "classes/class.h"

#include "Example_Actor.h" // include class declaration

GAME_CLASS_LIST_BEGIN{
GAME_API CLASS_LIST_BEGIN(GAME_CLASSES_LIST)
{
	CLASS_LIST_ADD(MyActor)  // add class to the list
}
CLASS_LIST_END() }
```
Take a look at the DemoProject for examples.

# Prerequisites
- DirectX stuff

# Libs used
- Dear ImGui
- stbi
- tinyobj
- FMOD
