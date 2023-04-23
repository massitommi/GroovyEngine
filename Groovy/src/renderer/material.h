#pragma once

#include "core/core.h"
#include "api/shader.h"

struct MaterialInput
{

};

class Material
{
public:
	Material();

private:
	Shader* mShader;
};

class MaterialInstance
{

};