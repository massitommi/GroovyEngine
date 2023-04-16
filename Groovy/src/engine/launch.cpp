//ClearColor clearColor = { 0.8f, .2f, 0.3f, 1.0f };
//ClearColor clearGameColor = { 1.0f, .45f, 0.83f, 1.0f };

//struct Vertex
//{
//	Vec4 location;
//	Vec4 color;
//	Vec2 textCoords;
//};

//struct MeshData
//{
//	String name;
//	size_t polyCount;
//	std::vector<Vertex> vertices;
//	std::vector<uint32> indices;
//};
//
//std::vector<MeshData> loadObj(const char* path)
//{
//	tinyobj::attrib_t attrib;
//	std::vector<tinyobj::shape_t> shapes;
//	std::vector<tinyobj::material_t> materials;
//	std::string warn, err;
//
//	tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &warn, path);
//
//	std::vector<MeshData> submeshes;
//
//	for (const auto& shape : shapes)
//	{
//		MeshData md;
//		md.name = shape.name.c_str();
//		md.polyCount = shape.mesh.indices.size() / 3; // is this correct ?
//
//		for (const auto& index : shape.mesh.indices)
//		{
//			Vertex vertex = {};
//
//			vertex.location =
//			{
//				attrib.vertices[3 * index.vertex_index + 0],
//				attrib.vertices[3 * index.vertex_index + 1],
//				attrib.vertices[3 * index.vertex_index + 2],
//				1.0f
//			};
//
//			if (index.texcoord_index != -1)
//			{
//				vertex.textCoords =
//				{
//					attrib.texcoords[2 * index.texcoord_index + 0],
//					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
//				};
//			}
//
//			vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };
//
//			md.vertices.push_back(vertex);
//			md.indices.push_back(md.indices.size());
//		}
//
//		submeshes.push_back(md);
//	}
//
//	return submeshes;
//}

#include "core/core.h"
#include "platform/platform.h"
#include "renderer/renderer_api.h"
#include "renderer/framebuffer.h"
#include "engine.h"

bool gEngineShouldRun = true;

ClearColor screenClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };

FrameBuffer* screenFrameBuffer = nullptr;

void OnWndResizeCallback(uint32 width, uint32 height)
{
	if (!width || !height)
		return;
	if (screenFrameBuffer->GetSpecs().width != width || screenFrameBuffer->GetSpecs().height != height)
	{
		screenFrameBuffer->Resize(width, height);
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
	wnd.Show();

	RendererAPI::Create(RENDERER_API_D3D11, &wnd);

	FrameBufferSpec screenBufferSpec;
	screenBufferSpec.swapchainTarget = true;
	screenBufferSpec.width = wnd.GetProps().width;
	screenBufferSpec.height = wnd.GetProps().height;
	screenBufferSpec.colorAttachments = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA };
	screenBufferSpec.hasDepthAttachment = true;

	screenFrameBuffer = FrameBuffer::Create(screenBufferSpec);

	wnd.SubmitToWndResizeCallback(OnWndResizeCallback);

	// shaders
	/*Buffer vertexFile, pixelFile;
	FileSystem::ReadFileBinary("assets/shaders/vertex.hlsl", vertexFile);
	FileSystem::ReadFileBinary("assets/shaders/pixel.hlsl", pixelFile);

	ShaderSrc vertexSrc = { vertexFile.data(), vertexFile.size() };
	ShaderSrc pixelSrc = { pixelFile.data(), pixelFile.size() };

	ShaderAttribute a_Location;
	a_Location.alignedOffset = offsetof(Vertex, location);
	a_Location.name = "POSITION";
	a_Location.size = sizeof(Vec4);
	a_Location.type = SHADER_ATTRIBUTE_TYPE_FLOAT4;

	ShaderAttribute a_Color;
	a_Color.alignedOffset = offsetof(Vertex, color);
	a_Color.name = "COLOR";
	a_Color.size = sizeof(Vec4);
	a_Color.type = SHADER_ATTRIBUTE_TYPE_FLOAT4;

	ShaderAttribute a_TexCoords;
	a_TexCoords.alignedOffset = offsetof(Vertex, textCoords);
	a_TexCoords.name = "TEXTCOORDS";
	a_TexCoords.size = sizeof(Vec2);
	a_TexCoords.type = SHADER_ATTRIBUTE_TYPE_FLOAT2;

	ShaderConstBuffer mvpBuffer;
	mvpBuffer.bufferTarget = CONSTBUFFER_TARGET_VERTEX;
	mvpBuffer.name = "mvp";
	mvpBuffer.size = sizeof(Mat4);

	Shader* shader = Shader::Create(vertexSrc, pixelSrc, { a_Location, a_Color, a_TexCoords }, { mvpBuffer });
	shader->Bind();*/

	/* textures
	Texture* testTexture;
	Texture* modelTexture;
	{
		 test texture
		int width, height, channels;
		void* mem = stbi_load("assets/ue4.jpg", &width, &height, &channels, 4);
		TextureSpecs specs;
		specs.width = width;
		specs.height = height;
		specs.format = TEXTURE_FORMAT_RGBA;
		testTexture = Texture::Create(specs, mem, width * height * 4);
		stbi_image_free(mem);
	}
	{
		 model texture
		int width, height, channels;
		void* mem = stbi_load("assets/negan/negan_texture.png", &width, &height, &channels, 4);
		TextureSpecs specs;
		specs.width = width;
		specs.height = height;
		specs.format = TEXTURE_FORMAT_RGBA;
		modelTexture = Texture::Create(specs, mem, width * height * 4);
		stbi_image_free(mem);
	}

	modelTexture->Bind(0);*/

	// model loading
	/*std::vector<MeshData> modelData = loadObj("assets/negan/negan_model.objfake");
	VertexBuffer* vertexBuffer = VertexBuffer::Create(modelData[0].vertices.size() * sizeof(Vertex), &modelData[0].vertices[0], sizeof(Vertex));
	IndexBuffer* indexBuffer = IndexBuffer::Create(modelData[0].indices.size() * sizeof(uint32), &modelData[0].indices[0]);
	vertexBuffer->Bind();
	indexBuffer->Bind();

	Vec3 camLocation = { 0,0,0 };
	Vec3 camRotation = { 0,0,0 };

	Vec3 objLocation = { 0, 0, 1 };
	Vec3 objRotation = { 0, 0, 0};
	Vec3 objScale = { 1, 1, 1};

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(wnd.GetNativeHandle());
	ImGui_ImplDX11_Init(d3dUtils::gDevice, d3dUtils::gContext);

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;*/

	Engine::Init();

	while (gEngineShouldRun)
	{
		wnd.ProcessEvents();

		Engine::Update(1.0f / 60.0f);

		screenFrameBuffer->ClearColorAttachment(0, screenClearColor);
		screenFrameBuffer->ClearDepthAttachment();

		Engine::Render();

		/*screenFrameBuffer->ClearColorAttachment(0, clearColor);
		screenFrameBuffer->ClearDepthAttachment();
		gameViewportFrameBuffer->ClearColorAttachment(0, clearGameColor);
		gameViewportFrameBuffer->ClearDepthAttachment();

		Mat4 newMVP = 
			math::GetModelMatrix(objLocation, objRotation, objScale)
			*
			math::GetViewMatrix(camLocation, camRotation)
			*
			math::GetPerspectiveMatrix(16.0f / 9.0f, 60.0f, 0.1f, 1000.0f);

		newMVP = math::GetMatrixTransposed(newMVP);

		shader->SetConstBuffer(0, &newMVP, sizeof(Mat4));

		gameViewportFrameBuffer->Bind();
		RendererAPI::Get().DrawIndexed(modelData[0].indices.size());
		screenFrameBuffer->Bind();

		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("game viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
		ImGui::Image((void*)gameViewportFrameBuffer->GetRendererID(0), { 500, 500 });
		ImGui::End();

		ImGui::Begin("texture demo", nullptr, ImGuiWindowFlags_NoScrollbar);
		ImGui::Image((void*)testTexture->GetRendererID(), { 300.0f, 300.0f });
		ImGui::End();

		ImGui::Begin("settings");
		ImGui::DragFloat3("model location", &objLocation.x);
		ImGui::DragFloat3("model rotation", &objRotation.x);
		ImGui::DragFloat3("model scale", &objScale.x);
		ImGui::End();

		ImGui::Render();
		
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());*/

#if 1
		screenFrameBuffer->Bind();
#endif
		Engine::Render2();
		RendererAPI::Get().Present(0);
	}

	Engine::Shutdown();

	return 0;
}