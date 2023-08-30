#pragma once

#include "engine/engine.h"

namespace editorGui
{
	bool AssetRef(const char* label, EAssetType type, void* data, bool allowNull, GroovyClass* classFilter = nullptr);
	bool Transform(const char* label, void* data);
	bool PropertyInput(const std::string& label, EPropertyType type, void* data, bool readonly, float lblColWidth, uint64 param1 = 0, uint64 param2 = 0);
	bool Property(const GroovyProperty& prop, void* propData);
	bool PropertiesSingleClass(GroovyObject* obj, GroovyClass* gClass, const std::vector<GroovyProperty>& props);
	bool PropertiesAllClasses(GroovyObject* obj);
}

class ActorBlueprint;

namespace editor
{
	void Init();
	void Update(float deltaTime);
	void Render();
	void Shutdown();

	Scene* GetEditScene();
	void FlagEditScenePendingSave();

	void OnBPUpdated(ActorBlueprint* bp);

	const char* AssetTypeToStr(EAssetType type);
}

#define EDITOR_SETTINGS_FILE "settings/settings"

GROOVY_CLASS_DECL(EditorSettings)
class EditorSettings : public GroovyObject
{
	GROOVY_CLASS_BODY(EditorSettings, GroovyObject)
public:
	float mCameraFOV = 60.0f;
	float mCameraMoveSpeed = 8.5f;
	float mCameraRotationSpeed = 0.2f;

	float mContentBrowserIconSize = 140.0f;
	bool mClassBrowserShowEngineClasses = false;
	bool mClassBrowserShowGameClasses = true;

	void Load();
	void Save();
};