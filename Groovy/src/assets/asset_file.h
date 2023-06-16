#pragma once

#include "classes/object.h"

GROOVY_CLASS_DECL(AssetFile)
class AssetFile : public GroovyObject
{
	GROOVY_CLASS_BODY(AssetFile, GroovyObject)

public:
	// call after AssetManager::Init
	virtual void ResolveDependencies() {}

	virtual size_t GetFileOffset() const { return 0; }
};