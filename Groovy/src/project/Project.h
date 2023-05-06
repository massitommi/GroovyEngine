#pragma once

#include "core/core.h"

class Project
{
public:
	Project();
	Project(const std::string& name, const std::string& assetPath);

	std::string GetName() const { return mName; }
	std::string GetAssetPath() const { return mAssetPath; }

	static void SetMain(Project* proj) { sMain = proj; }
	static Project* GetMain() { return sMain; }

private:
	std::string mName;
	std::string mAssetPath;

private:
	static Project* sMain;
};