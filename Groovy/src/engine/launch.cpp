#include "core/core.h"
#include "platform/platform.h"
#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "application.h"
#include "assets/asset_manager.h"
#include "project/project.h"
#include "classes/class_db.h"

bool gEngineShouldRun = true;
Window* gWindow = nullptr;
FrameBuffer* gScreenFrameBuffer = nullptr;
Project gProj;
ClassDB gClassDB;
ClearColor gScreenClearColor = { 0.9f, 0.7f, 0.7f, 1.0f };

extern std::vector<GroovyClass*> ENGINE_CLASSES;
//extern std::vector<GroovyClass*> GAME_CLASSES;

void OnWndResizeCallback(uint32 width, uint32 height)
{
	if (!width || !height)
		return;
	if (gScreenFrameBuffer->GetSpecs().width != width || gScreenFrameBuffer->GetSpecs().height != height)
	{
		gScreenFrameBuffer->Resize(width, height);
#if BUILD_SHIPPING
		gScreenFrameBuffer->Bind();
#endif
	}
}

int32 GroovyEntryPoint(const char* args)
{
	gProj.projFile = args;
	if (gProj.projFile.empty())
	{
		SysMessageBox::Show_Error("Error", "No project selected!");
		return -1;
	}

	Buffer projFile;
	FileSystem::ReadFileBinary(gProj.projFile.string(), projFile);
	gProj.name = std::string((char*)projFile.data(), projFile.size());
	gProj.assetRegistry = (gProj.projFile.parent_path() / "assets" / "assetregistry").string();
	gProj.assets = gProj.assetRegistry.parent_path().string();

	projFile.free();

	WindowProps wndProps =
	{
		gProj.name,			// wndTitle			
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

	RendererAPI::Create(RENDERER_API_D3D11, &wnd);

	FrameBufferSpec screenBufferSpec;
	screenBufferSpec.swapchainTarget = true;
	screenBufferSpec.width = wnd.GetProps().width;
	screenBufferSpec.height = wnd.GetProps().height;
	screenBufferSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	screenBufferSpec.hasDepthAttachment = true;

	gScreenFrameBuffer = FrameBuffer::Create(screenBufferSpec);

	wnd.SubmitToWndResizeCallback(OnWndResizeCallback);

	for (GroovyClass* c : ENGINE_CLASSES)
		gClassDB.Register(c);
	/*for (GroovyClass* c : GAME_CLASSES)
		gClassDB.Register(c);*/

	AssetManager::Init();
	Application::Init();

	gScreenFrameBuffer->Bind();

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

	delete gScreenFrameBuffer;
	Application::Shutdown();
	delete &RendererAPI::Get();

	return 0;
}