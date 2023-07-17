#pragma once

#include "core/core.h"
#include "classes/class.h"

namespace editorGui
{
	bool AssetRef(const char* label, EAssetType type, void* data, GroovyClass* classFilter = nullptr);
	bool Transform(const char* label, void* data, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
	bool PropertyInput(const std::string& label, EPropertyType type, void* data, bool readonly, float lblColWidth, uint64 param1 = 0, uint64 param2 = 0);
	bool Property(const GroovyProperty& prop, void* propData);
	bool PropertiesSingleClass(GroovyObject* obj, GroovyClass* gClass, const std::vector<GroovyProperty>& props);
	bool PropertiesAllClasses(GroovyObject* obj);
}

namespace editor
{
	namespace internal
	{
		void Init();
		void Update(float deltaTime);
		void Render();
		void Shutdown();
	}

	namespace utils
	{
		const char* AssetTypeToStr(EAssetType type);
	}
}