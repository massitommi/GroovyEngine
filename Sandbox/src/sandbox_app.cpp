#include "sandbox_app.h"
#include "engine/engine.h"
#include "gameframework/scene.h"
#include "project/project.h"
#include "assets/asset_manager.h"
#include "platform/messagebox.h"
#include "renderer/scene_renderer.h"
#include "platform/window.h"
#include "gameframework/components/cameracomponent.h"
#include "platform/input.h"
#include "renderer/api/renderer_api.h"

Application* GetApplication() { return new SandboxApplication(); }

static Scene* sScene = nullptr;
static float sAspectRatio = 0.0f;

void OnWndResize(uint32 width, uint32 height)
{
	sAspectRatio = (float)width / (float)height;
}

void SandboxApplication::Init()
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

void SandboxApplication::Update(float deltaTime)
{
#if !BUILD_SHIPPING // debug stuff
	if (Input::IsKeyPressed(EKeyCode::F3))
	{
		RasterizerState rasterState = RendererAPI::Get().GetRasterizerState();

		if (rasterState.fillMode == RASTERIZER_FILL_MODE_SOLID)
		{
			rasterState.fillMode = RASTERIZER_FILL_MODE_WIREFRAME;
			rasterState.cullMode = RASTERIZER_CULL_MODE_NONE;
		}
		else
		{
			rasterState.fillMode = RASTERIZER_FILL_MODE_SOLID;
			rasterState.cullMode = RASTERIZER_CULL_MODE_BACK;
		}

		RendererAPI::Get().SetRasterizerState(rasterState);
	}
#endif

	if (Input::IsKeyPressed(EKeyCode::F11))
	{
		gWindow->SetFullscreen(!gWindow->GetProps().fullscreen);
	}

	sScene->Tick(deltaTime);
}

void SandboxApplication::Render()
{
	if (sScene->mCamera && sAspectRatio > 0.0f)
	{
		SceneRenderer::BeginScene(sScene->mCamera, sAspectRatio);
		SceneRenderer::RenderScene(sScene);
	}
}

void SandboxApplication::Shutdown()
{
	if (sScene)
		sScene->Unload();
}

void SandboxApplication::Travel(Scene* scene)
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