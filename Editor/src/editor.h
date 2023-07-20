#pragma once

#include "core/core.h"
#include "classes/object.h"

namespace editorGui
{
	bool AssetRef(const char* label, EAssetType type, void* data, GroovyClass* classFilter = nullptr);
	bool Transform(const char* label, void* data);
	bool PropertyInput(const std::string& label, EPropertyType type, void* data, bool readonly, float lblColWidth, uint64 param1 = 0, uint64 param2 = 0);
	bool Property(const GroovyProperty& prop, void* propData);
	bool PropertiesSingleClass(GroovyObject* obj, GroovyClass* gClass, const std::vector<GroovyProperty>& props);
	bool PropertiesAllClasses(GroovyObject* obj);
}

namespace editor
{
	void Init();
	void Update(float deltaTime);
	void Render();
	void Shutdown();

	namespace utils
	{
		const char* AssetTypeToStr(EAssetType type);
	}
}

#define EDITOR_SETTINGS_FILE "settings/settings"

GROOVY_CLASS_DECL(EditorSettings)
class EditorSettings : public GroovyObject
{
	GROOVY_CLASS_BODY(EditorSettings, GroovyObject)
public:
	float mEditorCameraFOV = 60.0f;
	float mEditorCameraMoveSpeed = 1.0f;
	float mEditorCameraRotationSpeed = 0.3f;

	void Load();
	void Save();
};