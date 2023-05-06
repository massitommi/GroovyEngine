#include "core/core.h"
#include "platform/platform.h"
#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "application.h"
#include "assets/asset_manager.h"
#include "project/Project.h"

bool gEngineShouldRun = true;
FrameBuffer* gScreenFrameBuffer = nullptr;

ClearColor screenClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };

Project mainProj;

void OnWndResizeCallback(uint32 width, uint32 height)
{
	if (!width || !height)
		return;
	if (gScreenFrameBuffer->GetSpecs().width != width || gScreenFrameBuffer->GetSpecs().height != height)
	{
		gScreenFrameBuffer->Resize(width, height);
	}
}

int32 GroovyEntryPoint(const char* args)
{
	WindowProps wndProps =
	{
		"Groovy window!",	// title
		1600, 900,			// resolution
		0,					// refreshrate (0 = max)
		false,				// fullscreen
		false				// v-sync
	};

	Window::InitSystem();
	Window wnd(wndProps);
	Window::SetMainWindow(&wnd);
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

	std::string projPath = args;
	if (projPath.empty())
	{
		ExtensionFilter projExtFilter = { "Groovy project", { "*.groovyproj" } };
		projPath = FileDialog::OpenFileDialog("Select a groovy project", { projExtFilter });
		if (projPath == "")
			return -1;
	}

	Buffer tmp;
	FileSystem::ReadFileBinary(projPath, tmp);
	std::string assetPath;
	assetPath.resize(tmp.size());
	memcpy(assetPath.data(), tmp.data(), tmp.size());

	std::string projName = FileSystem::GetFilenameNoExt(projPath);
	std::string projAbsoluteAssetPath = FileSystem::GetParentFolder(projPath) + assetPath + FileSystem::DIR_SEPARATOR;

	mainProj = Project(projName, projAbsoluteAssetPath);
	Project::SetMain(&mainProj);

	AssetManager::Init();
	Application::Init();

	while (gEngineShouldRun)
	{
		wnd.ProcessEvents();

		Application::Update(1.0f / 60.0f);

		gScreenFrameBuffer->ClearColorAttachment(0, screenClearColor);
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