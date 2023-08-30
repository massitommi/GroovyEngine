#pragma once

#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(MeshComponent)
class CORE_API MeshComponent : public SceneComponent
{
	GROOVY_CLASS_BODY(MeshComponent, SceneComponent)

public:
	MeshComponent();

	inline Mesh* GetMesh() const { return mMesh; }
	void SetMesh(Mesh* mesh);

	virtual void Initialize() override;
	virtual void Uninitialize() override;

	inline const std::vector<Material*> GetMaterialOverrides() const { return mMaterialOverrides; }
	void SetMaterialOverride(uint32 index, Material* mat);

#if WITH_EDITOR
	void Editor_OnPropertyChanged(const GroovyProperty* prop) override;
#endif

public:
	bool mVisible;

private:
	Mesh* mMesh;
	std::vector<Material*> mMaterialOverrides;


	friend class SceneRenderer;
};