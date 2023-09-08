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
#include "runtime/object_allocator.h"
#include "audio/audio.h"

bool gEngineShouldRun = true;
Window* gWindow = nullptr;
FrameBuffer* gScreenFrameBuffer = nullptr;
GroovyProject gProj;
ClassDB gClassDB;
ClearColor gScreenClearColor = { 0.9f, 0.7f, 0.7f, 1.0f };
double gTime = 0.0f;
double gDeltaTime = 0.0f;
extern std::vector<GroovyClass*> ENGINE_CLASSES;
extern std::vector<GroovyClass*> GAME_CLASSES;

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
	if (!args[0])
	{
		CORE_ASSERT(0, "No project supplied for engine launch");
		return -1;
	}

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


	// load project and settings

	gProj.BuildPaths(args);

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

#if BUILD_DEBUG
	if (ObjectAllocator::Debug_GetLiveObjectsCount())
	{
		SysMessageBox::Show_Warning
		(
			"Dear engine programmer",
			"Some groovy objects are still alive after shutdown, how is that?"
		);
	}

	if (Audio::GetClipsCount())
	{
		SysMessageBox::Show_Warning
		(
			"Dear engine programmer",
			"Some audio clips are still in ram after shutdown, how is that?"
		);
	}
#endif
	
	return 0;
}