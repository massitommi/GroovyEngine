workspace "GroovyEngine"

    include "premake_core.lua"

    startproject "Editor"

    debugargs "../DemoProject/DemoProject.groovyproj"

include "Groovy"
include "Editor"
include "Sandbox"
include "ProjectCreator"