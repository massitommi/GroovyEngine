#include "engine/engine.h"
#include "platform/platform.h"
#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "application.h"
#include "assets/asset_manager.h"
#include "project/project.h"
#include "classes/class_db.h"
#include "renderer/renderer.h"
#include "gameframework/scene.h"
#include "runtime/object_allocator.h"

void OnWndResizeCallback(uint32 width, uint32 height)
{
	if (!width || !height)
		return;
	if (gScreenFrameBuffer->GetSpecs().width != width || gScreenFrameBuffer->GetSpecs().height != height)
	{
		gScreenFrameBuffer->Resize(width, height);
		gScreenFrameBuffer->Bind();
	}
}

class Application* GetApplication();

int32 GroovyEntryPoint(const char* args)
{
	if (!args[0])
	{
		SysMessageBox::Show_Error("Project not found!", "Project not found!");
		return -1;
	}

	gProj.BuildPaths(args);

	for (GroovyClass* c : ENGINE_CLASSES)
		gClassDB.Register(c);

#if !BUILD_MONOLITHIC

	// load game dll and fill GAME_CLASSES
	std::string gameDllPath = (gProj.GetProjectFilePath().parent_path() / "bin" / "Game.dll").string();
	
	void* gameDll = Lib::LoadDll(gameDllPath);
	checkslowf(gameDll, "Game dll not found, path: %s", gameDllPath.c_str());
	
	void* gameClassesList = Lib::GetSymbol(gameDll, "GAME_CLASSES_LIST");
	checkslowf(gameClassesList, "Game classes list not found in game dll");

	std::vector<GroovyClass*>* gameClassesListVec = (std::vector<GroovyClass*>*)gameClassesList;
	GAME_CLASSES = *gameClassesListVec;

#endif

	for (GroovyClass* c : GAME_CLASSES)
		gClassDB.Register(c);
	
	gClassDB.BuildCDOs();

	// windowing system
	WindowProps wndProps =
	{
		"Groovy",			// title
		1600, 900,			// resolution
		false				// fullscreen
	};

	Window::InitSystem();
	Window wnd(wndProps);
	gWindow = &wnd;
	wnd.Spawn();
	wnd.Show();

	// startup renderering
	RendererAPISpec rendererAPISpec;
	rendererAPISpec.refreshrate = 0;	// max monitor refreshrate
	rendererAPISpec.vsync = 1;			// v-sync enabled

	RendererAPI::Create(RENDERER_API_D3D11, rendererAPISpec, &wnd);

	FrameBufferSpec screenBufferSpec;
	screenBufferSpec.swapchainTarget = true;
	screenBufferSpec.width = wnd.GetProps().width;
	screenBufferSpec.height = wnd.GetProps().height;
	screenBufferSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	screenBufferSpec.hasDepthAttachment = true;

	gScreenFrameBuffer = FrameBuffer::Create(screenBufferSpec);


	wnd.SubmitToWndResizeCallback(OnWndResizeCallback);

	AssetManager::Init();

	gProj.Load(); // we need to initalize the assetManager in order to deserialize the startup scene

	Application* app = GetApplication();

	app->Init();

	gScreenFrameBuffer->Bind();
	Renderer::Init();

	Input::Init();

	TickTimer::Init();

	while (gEngineShouldRun)
	{
		wnd.ProcessEvents();

		double currentTime = TickTimer::GetTimeSeconds();
		gDeltaTime = currentTime - gTime;
		gTime = currentTime;

		app->Update((float)gDeltaTime);

		Input::Clear();

		gScreenFrameBuffer->ClearColorAttachment(0, gScreenClearColor);
		gScreenFrameBuffer->ClearDepthAttachment();

		app->Render();

		RendererAPI::Get().Present();
	}

	Input::Shutdown();

	Renderer::Shutdown();

	app->Shutdown();

	gProj.Save();

	AssetManager::Shutdown();

	gClassDB.DestroyCDOs();
	
	delete gScreenFrameBuffer;
	RendererAPI::Destroy();

	delete app;

#if !BUILD_MONOLITHIC

	Lib::UnloadDll(gameDll);

#endif

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