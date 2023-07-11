#include "core/core.h"
#include "platform/platform.h"
#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "application.h"
#include "assets/asset_manager.h"
#include "project/project.h"
#include "classes/class_db.h"
#include "renderer/renderer.h"
#include "gameframework/scene.h"
#include "gameframework/game_module.h"
#include "world.h"
#include "runtime/object_allocator.h"

bool gEngineShouldRun = true;
Window* gWindow = nullptr;
FrameBuffer* gScreenFrameBuffer = nullptr;
GroovyProject gProj;
ClassDB gClassDB;
ClearColor gScreenClearColor = { 0.9f, 0.7f, 0.7f, 1.0f };

extern std::vector<GroovyClass*> ENGINE_CLASSES;
extern std::vector<GroovyClass*> GAME_CLASSES;

void OnWndResizeCallback(uint32 width, uint32 height)
{
	if (!width || !height)
		return;
	if (gScreenFrameBuffer->GetSpecs().width != width || gScreenFrameBuffer->GetSpecs().height != height)
	{
		gScreenFrameBuffer->Resize(width, height);
#if !WITH_EDITOR
		gScreenFrameBuffer->Bind();
#endif
	}
}

int32 GroovyEntryPoint(const char* args)
{
	// register classes
	for (GroovyClass* c : ENGINE_CLASSES)
		gClassDB.Register(c);
	for (GroovyClass* c : GAME_CLASSES)
		gClassDB.Register(c);
	
	gClassDB.BuildCDOs();

	// windowing system
	WindowProps wndProps =
	{
		"Groovy",			// title
		1600, 900,			// resolution
		0,					// refreshrate (0 = max)
		false,				// fullscreen
		false				// v-sync
	};

	Window::InitSystem();
	Window wnd(wndProps);
	gWindow = &wnd;
	wnd.Spawn();
	wnd.Show();

	// startup renderering
	RendererAPI::Create(RENDERER_API_D3D11, &wnd);

	FrameBufferSpec screenBufferSpec;
	screenBufferSpec.swapchainTarget = true;
	screenBufferSpec.width = wnd.GetProps().width;
	screenBufferSpec.height = wnd.GetProps().height;
	screenBufferSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	screenBufferSpec.hasDepthAttachment = true;

	gScreenFrameBuffer = FrameBuffer::Create(screenBufferSpec);

	wnd.SubmitToWndResizeCallback(OnWndResizeCallback);

	// load project and settings
	gProj.BuildPaths(args);

	AssetManager::Init();
	Application::Init();
	GameModule::Startup();

	gScreenFrameBuffer->Bind();
	Renderer::Init();

	World::Travel(gProj.GetStartupScene());

	while (gEngineShouldRun)
	{
		wnd.ProcessEvents();

		Application::Update(1.0f / 60.0f);

		gScreenFrameBuffer->ClearColorAttachment(0, gScreenClearColor);
		gScreenFrameBuffer->ClearDepthAttachment();

		Application::Render();

#if WITH_EDITOR
		gScreenFrameBuffer->Bind();
		Application::Render2EditorOnly();
#endif
		RendererAPI::Get().Present(0);
	}

	World::End();
	Renderer::Shutdown();

	GameModule::Shutdown();
	Application::Shutdown();

	gProj.Save();

	AssetManager::Shutdown();

	gClassDB.DestroyCDOs();
	
	delete gScreenFrameBuffer;
	RendererAPI::Destroy();

#if BUILD_DEBUG
	if (ObjectAllocator::Debug_GetLiveObjectsCount())
	{
		SysMessageBox::Show_Warning
		(
			"Dear engine programmer",
			"Some groovy objects are still alive after shutdown, how is that?"
		);
	}
#endif
	
	return 0;
}