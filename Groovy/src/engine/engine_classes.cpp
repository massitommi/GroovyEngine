#include "classes/class.h"

#include "renderer/material.h"
#include "renderer/mesh.h"

#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(TestClassBase)
class TestClassBase : public GroovyObject
{
	GROOVY_CLASS_BODY(TestClassBase, GroovyObject)

	std::string baseStr = "boh";
	Vec2 baseVec = { 2,48};
};

GROOVY_CLASS_IMPL(TestClassBase)
	GROOVY_REFLECT(baseStr)
	GROOVY_REFLECT(baseVec)
GROOVY_CLASS_END()

GROOVY_CLASS_DECL(TestClass)
class TestClass : public TestClassBase
{
	GROOVY_CLASS_BODY(TestClass, TestClassBase)

	std::vector<std::string> strings = { "ciao", "come", "va?" };
	std::vector<Vec3> vecs = { { 1,1,1 }, {2,2,2} };
	std::vector<int32> ints = { 4,5,6,7,8 };
	bool bBool = false;
	std::string str = "mhanz";
	Vec3 pos = { 5,6,7 };
	std::string strings_static[3] = { "uno", "due", "tre" };
	uint32 uints_static[4] = { 2,3,4,5 };
};

GROOVY_CLASS_IMPL(TestClass)
	GROOVY_REFLECT(strings)
	GROOVY_REFLECT(vecs)
	GROOVY_REFLECT(ints)
	GROOVY_REFLECT(bBool)
	GROOVY_REFLECT(str)
	GROOVY_REFLECT(pos)
	GROOVY_REFLECT(strings_static)
	GROOVY_REFLECT(uints_static)
GROOVY_CLASS_END()


CLASS_LIST_BEGIN(ENGINE_CLASSES)
{
	CLASS_LIST_ADD(GroovyObject),

	CLASS_LIST_ADD(TestClassBase),
	CLASS_LIST_ADD(TestClass),

	CLASS_LIST_ADD(MaterialAssetFile),
	CLASS_LIST_ADD(MeshAssetFile),

	CLASS_LIST_ADD(Actor),
	CLASS_LIST_ADD(ActorComponent)
}
CLASS_LIST_END()