#include "Project.h"

Project* Project::sMain = nullptr;

Project::Project()
	: mName(), mAssetPath()
{}

Project::Project(const std::string& name, const std::string& assetPath)
	: mName(name), mAssetPath(assetPath)
{}
