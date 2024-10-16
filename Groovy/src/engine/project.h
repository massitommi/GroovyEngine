#pragma once

#include "classes/object.h"
#include <filesystem>

GROOVY_CLASS_DECL(GroovyProject)
class CORE_API GroovyProject : public GroovyObject
{
	GROOVY_CLASS_BODY(GroovyProject, GroovyObject)

public:
	void BuildPaths(const char* projectFilePath);

private:
	// project settings
	Scene* mStartupScene;

	// paths (retrived at runtime)
	std::string mProjName;
	std::filesystem::path mProjFilePath;
	std::filesystem::path mAssetRegistryPath;
	std::filesystem::path mAssetsPath;

public:
	inline Scene* GetStartupScene() const { return mStartupScene; }

	inline void SetStartupScene(Scene* scene) { mStartupScene = scene; }

	inline const std::string& GetProjectName() const { return mProjName; }
	inline const std::filesystem::path& GetProjectFilePath() const { return mProjFilePath; }
	inline const std::filesystem::path& GetAssetRegistryPath() const { return mAssetRegistryPath; }
	inline const std::filesystem::path& GetAssetsPath() const { return mAssetsPath; }

	void Load();
	void Save();
};