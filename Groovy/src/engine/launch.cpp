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

Project gProj;

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
	std::string assetRegistryPath;
	assetRegistryPath.resize(tmp.size());
	memcpy(assetRegistryPath.data(), tmp.data(), tmp.size());

#if PLATFORM_WIN32
	for (char& c : assetRegistryPath)
		if (c == '/')
			c = '\\';
#endif

	std::string projName = FileSystem::GetFilenameNoExt(projPath);
	std::string absoluteAssetRegistryPath = FileSystem::GetParentFolder(projPath) + assetRegistryPath;
	std::string absoluteAssetsPath = FileSystem::GetParentFolder(absoluteAssetRegistryPath);

	gProj.name = projName;
	gProj.registryPath = absoluteAssetRegistryPath;
	gProj.assetsPath = absoluteAssetsPath;

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
	AssetManager::Shutdown();
	Application::Shutdown();
	delete &RendererAPI::Get();

	return 0;
}