#include "engine/application.h"
#include "gameframework/scene.h"
#include "project/project.h"
#include "assets/asset_manager.h"
#include "platform/messagebox.h"
#include "renderer/scene_renderer.h"
#include "platform/window.h"
#include "gameframework/components/cameracomponent.h"

extern bool gEngineShouldRun;
extern GroovyProject gProj;
extern Window* gWindow;

static Scene* sScene = nullptr;
static float sAspectRatio = 0.0f;

void OnWndResize(uint32 width, uint32 height)
{
	sAspectRatio = (float)width / (float)height;
}

void Application::Init()
{
	sAspectRatio = (float)gWindow->GetProps().width / (float)gWindow->GetProps().height;
	gWindow->SubmitToWndResizeCallback(OnWndResize);
	gWindow->EnableCursor(false);
	gWindow->SetMaxSize();
	gWindow->SetFullscreen(true);

	sScene = gProj.GetStartupScene();

	if (!sScene)
	{
		auto scenes = AssetManager::GetAssets(ASSET_TYPE_SCENE);
		if (scenes.size())
		{
			sScene = (Scene*)scenes[0].instance;
		}
	}

	if (!sScene)
	{
		SysMessageBox::Show_Error("No startup scene!", "No startup scene!");
		gEngineShouldRun = false;
		return;
	}

	sScene->Load();
	sScene->BeginPlay();
}

void Application::Update(float deltaTime)
{
	sScene->Tick(deltaTime);
}

void Application::Render()
{
	if (sScene->mCamera && sAspectRatio > 0.0f)
	{
		SceneRenderer::BeginScene(sScene->mCamera, sAspectRatio);
		SceneRenderer::RenderScene(sScene);
	}
}

void Application::Shutdown()
{
	if (sScene)
		sScene->Unload();
}

void Application::Travel(Scene* scene)
{
	sScene->Unload();

	sScene = scene;

	if (scene)
	{
		scene->Load();
		scene->BeginPlay();
	}
	else
	{
		gEngineShouldRun = false;
	}
}