#include "engine/engine.h"
#include "platform/platform.h"
#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "application.h"
#include "assets/asset_manager.h"
#include "engine/project.h"
#include "classes/class_db.h"
#include "renderer/renderer.h"
#include "gameframework/scene.h"
#include "runtime/object_allocator.h"
#include "audio/audio.h"

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

int32 GroovyEntryPoint(const char* args)
{
	Application::PreInit();

	if (!args[0])
	{
		SysMessageBox::Show_Error("Can't launch without a project!", "Can't launch without a project!");
		return -1;
	}

	gProj.BuildPaths(args);

	if (!FileSystem::FileExists(gProj.GetProjectFilePath().string()))
	{
		SysMessageBox::Show_Error("Project not found!", "Can't find " + std::string(args));
		return -1;
	}

	for (GroovyClass* c : ENGINE_CLASSES)
		gClassDB.Register(c);

#if !BUILD_MONOLITHIC

	// load game dll and fill GAME_CLASSES
	std::string gameDllPath = (gProj.GetProjectFilePath().parent_path() / "bin" / LINKER_OUTPUT_DIR / gProj.GetProjectName() / (gProj.GetProjectName() + ".dll")).string();
	
	void* gameDll = Lib::LoadDll(gameDllPath);

	if (!gameDll)
	{
		SysMessageBox::Show_Warning("Game dll not found!", "Unable to load game code: " + gameDllPath + ", starting without game code...");
	}
	else
	{
		void* gameClassesList = Lib::GetSymbol(gameDll, "GAME_CLASSES_LIST");
		checkslowf(gameClassesList, "Game classes list not found in game dll");

		std::vector<GroovyClass*>* gameClassesListVec = (std::vector<GroovyClass*>*)gameClassesList;
		GAME_CLASSES = *gameClassesListVec;
	}

#else

	extern std::vector<GroovyClass*> GAME_CLASSES_LIST;
	GAME_CLASSES = GAME_CLASSES_LIST;

#endif

	for (GroovyClass* c : GAME_CLASSES)
		gClassDB.Register(c);
	
	gClassDB.BuildCDOs();

	// windowing system
	WindowProps wndProps =
	{
		gProj.GetProjectName(),	// title
		1600, 900,				// resolution
		false					// fullscreen
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

	Audio::Init();

	AssetManager::Init();

	gProj.Load(); // we need to initalize the assetManager in order to deserialize the startup scene

	Application::Init();

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

		Audio::Update();

		Application::Update((float)gDeltaTime);

		Input::Clear();

		gScreenFrameBuffer->ClearColorAttachment(0, gScreenClearColor);
		gScreenFrameBuffer->ClearDepthAttachment();

		Application::Render();

		RendererAPI::Get().Present();
	}

	Input::Shutdown();
	Audio::Shutdown();
	Renderer::Shutdown();
	Application::Shutdown();

	gProj.Save();

	AssetManager::Shutdown();

	gClassDB.DestroyCDOs();
	
	delete gScreenFrameBuffer;
	RendererAPI::Destroy();

#if !BUILD_MONOLITHIC

	if(gameDll)
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