#include "pch.h"
#include "GfxResourceDevice.h"
#include "2D/SpriteTexture.h"
#include <functional>

GfxResourceDevice::GfxResourceDevice(std::shared_ptr<DXDevice> dev) :
	m_dxDev(dev)
{
	m_assimpLoader = std::make_unique<AssimpLoader>();

}

GfxResourceDevice::~GfxResourceDevice()
{

}

int getNumMipLevels(int width, int height)
{
	return static_cast<int>(1 + floor(log2(std::max(width, height))));
}

std::shared_ptr<DXTexture> GfxResourceDevice::loadTexture(std::string filepath)
{
	auto texData = loadFileToTexture(filepath);

	if (texData.width == 0 || texData.height == 0)
		return nullptr;

	DXTexture::Desc desc{
		DXTexture::Type::TEX2D,
	};
	int mips = getNumMipLevels(texData.width, texData.height);
	desc.desc2D = {
		static_cast<UINT>(texData.width),
		static_cast<UINT>(texData.height),
		static_cast<UINT>(mips),
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		{1, 0},
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		0,
		D3D11_RESOURCE_MISC_GENERATE_MIPS,
	};

	D3D11_SUBRESOURCE_DATA* initData = new D3D11_SUBRESOURCE_DATA[mips];
	int sysMemPitch = texData.width * sizeof(uint32_t);
	for(int i = 0; i < mips; i++)
	{
		initData[i].pSysMem = texData.data;
		initData[i].SysMemPitch = sysMemPitch;
		initData[i].SysMemSlicePitch = 0;
		sysMemPitch >>= 1;
	}

	auto tex = m_dxDev->createTexture(desc, initData);

	free(texData.data);
	delete[] initData;
	initData = nullptr;

	return tex;
}

unsigned int createRGB(int r, int g, int b)
{
	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

std::shared_ptr<DXTexture> GfxResourceDevice::createTextureFromColor(float color[3])
{
	// MIP Levels set to one for now!!!
	DXTexture::Desc desc{
		DXTexture::Type::TEX2D,
	};
	desc.desc2D = {
		1,
		1,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		{1, 0},
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		0,
	};

	// BGR!!
	uint32_t col = createRGB(color[2] * 255.f, color[1] * 255.f, color[0] * 255.f);
	D3D11_SUBRESOURCE_DATA initData = { &col, sizeof(uint32_t), 0 };

	auto tex = m_dxDev->createTexture(desc, &initData);
	return tex;
}

std::shared_ptr<DXTexture> GfxResourceDevice::loadTextureSprite(std::string filepath)
{
	auto texData = loadFileToTexture(filepath);

	if (texData.width == 0 || texData.height == 0)
		return nullptr;

	// MIP Levels set to one for now!!!
	DXTexture::Desc desc{
		DXTexture::Type::TEX2D,
	};
	desc.desc2D = {
		static_cast<UINT>(texData.width),
		static_cast<UINT>(texData.height),
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		{1, 0},
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		0,
	};

	D3D11_SUBRESOURCE_DATA initData{
		texData.data,
		texData.width * sizeof(uint32_t),
		0,
	};


	auto tex = m_dxDev->createTexture(desc, &initData);

	free(texData.data);

	return tex;
}

std::shared_ptr<Mesh> GfxResourceDevice::createMesh(const std::string& meshID, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
{
	if (m_meshRepo.exists(meshID))
	{
		return m_meshRepo.find(meshID);
	}

	D3D11_SUBRESOURCE_DATA subres;
	ZeroMemory(&subres, sizeof(D3D11_SUBRESOURCE_DATA));
	subres.pSysMem = vertices.data();
	auto vb = m_dxDev->createVertexBuffer(vertices.size(), sizeof(Vertex), false, false, false, &subres);

	subres.pSysMem = indices.data();
	auto ib = m_dxDev->createIndexBuffer(indices.size() * sizeof(uint32_t), false, &subres);

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vb, ib);

	m_meshRepo.add(meshID, mesh);

	return mesh;
}

std::shared_ptr<Model> GfxResourceDevice::assembleModel(const std::string& meshID, std::shared_ptr<Material> material)
{
	if (!m_meshRepo.exists(meshID)) assert(false);		// Mesh ID doesn't exist!
	auto mesh = m_meshRepo.find(meshID);				// Get mesh

	std::vector<SubsetMaterial> materials;
	
	SubsetMaterial mat;
	mat.vertexStart = 0;
	mat.vertexCount = mesh->getVB()->getElementCount();
	mat.indexStart = 0;
	mat.indexCount = mesh->getIB()->getElementCount();
	mat.material = material;
	materials.push_back(mat);

	std::shared_ptr<Model> modToAdd = std::make_shared<Model>(mesh, materials);

	return modToAdd;			// NO REPO FOR NOW FOR MODELS!

}

std::shared_ptr<Model> GfxResourceDevice::createModel(const std::string& modelDirectory, const std::string& modelFileName, GfxShader shader)
{

	EngineMeshData modelData;
	size_t modHash = std::hash<std::string>{}(modelDirectory + modelFileName);
	if (m_modelRepo.exists(modHash))
	{
		modelData = m_modelRepo.find(modHash);
	}
	else
	{
		modelData = m_assimpLoader->loadStaticModel(modelDirectory + modelFileName);
		m_modelRepo.add(modHash, modelData);
	}

	// Load model in one mesh
	auto mesh = createMesh(modelDirectory + modelFileName, modelData.vertices, modelData.indices);

	std::vector<SubsetMaterial> materials;

	for (auto& subsetInfo : modelData.subsetsInfo)
	{
		SubsetMaterial mat;
		mat.indexCount = subsetInfo.indexCount;
		mat.indexStart = subsetInfo.indexStart;
		mat.vertexCount = subsetInfo.vertexCount;
		mat.vertexStart = subsetInfo.vertexStart;
		if (subsetInfo.diffuseFilePath == "")
		{
			// Use our default texture
			std::string defaultDir = "Textures/Default/";
			//mat.material = createMaterial(shader, defaultDir + "diffuse.png", defaultDir + "specular.png", defaultDir + "normal.png");

			// Load 1x1 based on diffuse color
			mat.material = createMaterial(shader, subsetInfo.color, defaultDir + "specular.png", defaultDir + "normal.png");
		}
		else
		{
			mat.material = createMaterial(shader, modelDirectory + subsetInfo.diffuseFilePath, modelDirectory + subsetInfo.specularFilePath, modelDirectory + subsetInfo.normalFilePath);
		}
		materials.push_back(mat);
	}
	

	std::shared_ptr<Model> modToAdd = std::make_shared<Model>(mesh, materials);

	return modToAdd;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, const std::wstring& path, float x, float y)
{
	auto t = createTextElement(text, path);
	t->setPosition({x, y});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, float x, float y)
{
	auto t = std::make_shared<Text>();
	t->setText(text);
	t->setPosition({x, y});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, const std::wstring& path, ScreenPos screenPosX, float y)
{
	auto t = createTextElement(text, path);
	t->setPosition({getXFromScreenPos(screenPosX), y});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, ScreenPos screenPosX, float y)
{
	auto t = std::make_shared<Text>();
	t->setText(text);
	t->setPosition({getXFromScreenPos(screenPosX), y});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, const std::wstring& path, float x, ScreenPos screenPosY)
{
	auto t = createTextElement(text, path);
	t->setPosition({x, getYFromScreenPos(screenPosY)});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, float x, ScreenPos screenPosY)
{
	auto t = std::make_shared<Text>();
	t->setText(text);
	t->setPosition({x, getYFromScreenPos(screenPosY)});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, const std::wstring& path, ScreenPos screenPosX, ScreenPos screenPosY)
{
	auto t = createTextElement(text, path);
	t->setPosition({getXFromScreenPos(screenPosX), getYFromScreenPos(screenPosY)});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSprite(const std::string& text, ScreenPos screenPosX, ScreenPos screenPosY)
{
	auto t = std::make_shared<Text>();
	t->setText(text);
	t->setPosition({getXFromScreenPos(screenPosX), getYFromScreenPos(screenPosY)});
	t->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return t;
}

std::shared_ptr<ISprite> GfxResourceDevice::createSpriteTexture(const std::string& textureName, float positionX, float positionY, float scaleX, float scaleY, float rotation, float depth, std::function<void()> callback)
{
	auto tex = loadTextureSprite(textureName);
	auto sprite = std::make_shared<SpriteTexture>(tex, rotation, depth, callback);
	sprite->setPosition({ positionX, positionY });
	sprite->setScale({ scaleX, scaleY });
	sprite->setWindowSize({ (float)m_dxDev->getClientWidth(), (float)m_dxDev->getClientHeight() });
	return sprite;
}

std::pair<std::size_t, Material::ShaderSet> GfxResourceDevice::loadShader(GfxShader shader)
{
	std::string vsFileName, psFileName;

	switch (shader)
	{
	case GfxShader::DEFAULT:
		vsFileName = "DefaultVS.cso";
		psFileName = "DefaultPS.cso";
		break;
	default:
		assert(false);
	}

	std::string shdPathToHash = vsFileName + psFileName;
	size_t shdHash = std::hash<std::string>{}(shdPathToHash);

	Material::ShaderSet shaders;
	if (!m_shaderSetRepo.exists(shdHash))
	{
		// Here we should ASK DXDevice for the Shaders rather than create them! 
		// --> We assume that all shaders are loaded into the program at start time!

		auto vs = m_dxDev->createShader(vsFileName, DXShader::Type::VS);
		auto ps = m_dxDev->createShader(psFileName, DXShader::Type::PS);

		shaders = { vs, ps };
	}

	shaders = m_shaderSetRepo.add(shdHash, shaders);  // If hash already exists, the existing shader will be returned and old one will be discarded.

	return { shdHash, shaders };
}

std::pair<std::shared_ptr<DXBuffer>, std::shared_ptr<DXBuffer>> GfxResourceDevice::loadBuffers(GfxShader shader)
{
	unsigned int vsDataSize = 0;
	unsigned int psDataSize = 0;

	switch (shader)
	{
	case GfxShader::DEFAULT:
		vsDataSize = sizeof(DefaultShader_VSDATA);
		psDataSize = sizeof(DefaultShader_PSDATA);
		break;

	default:
		assert(false);
	}

	std::shared_ptr<DXBuffer> vsBuf = m_dxDev->createConstantBuffer(vsDataSize, true, true, nullptr);
	std::shared_ptr<DXBuffer> psBuf = m_dxDev->createConstantBuffer(psDataSize, true, true, nullptr);

	return { vsBuf, psBuf };
}

std::shared_ptr<Text> GfxResourceDevice::createTextElement(const std::string& text, const std::wstring& path)
{

	std::shared_ptr<DirectX::SpriteFont> sFont;
	if (m_fontRepo.exists(path))
	{
		sFont = m_fontRepo.find(path);
	}
	else
	{
		sFont = m_dxDev->createSpriteFont(path);
		m_fontRepo.add(path, sFont);
	}
	auto t = std::make_shared<Text>(sFont);
	t->setText(text);
	return t;
}

float GfxResourceDevice::getXFromScreenPos(ScreenPos pos)
{
	switch (pos)
	{
	case ScreenPos::MIDDLE:
		return m_dxDev->getBackbufferViewport()->Width / 2.f;
	}
}

float GfxResourceDevice::getYFromScreenPos(ScreenPos pos)
{
	switch (pos)
	{
	case ScreenPos::MIDDLE:
		return m_dxDev->getBackbufferViewport()->Height / 2.f;
	}
}

std::shared_ptr<Material> GfxResourceDevice::createMaterial(GfxShader shader, const std::string& difPath, const std::string& specPath, const std::string& normPath)
{
	// Load shaders
	auto hashAndShaders = loadShader(shader);

	// Load buffers
	auto vsPsBuffers = loadBuffers(shader);

	// Load textures
	Material::PhongMaps maps;

	// Check hash first before loading texture!
	std::string texPathToHash = difPath + specPath + normPath;
	size_t texHash = std::hash<std::string>{}(texPathToHash);

	std::shared_ptr<Material> matToAdd = nullptr;
	if (!m_materialRepo.exists(texHash))
	{
		maps.diffuse = loadTexture(difPath);
		maps.specular = loadTexture(specPath);
		maps.normal = loadTexture(normPath);

		matToAdd = std::make_shared<Material>(hashAndShaders.second, maps, texHash, hashAndShaders.first);
	}
	std::shared_ptr<Material> mat = m_materialRepo.add(texHash, matToAdd);			// If hash already exists, the existing mat will be returned and matToAdd will be discarded
	mat->setBuffers(vsPsBuffers.first, vsPsBuffers.second);

	return mat;
}

std::shared_ptr<Material> GfxResourceDevice::createMaterial(GfxShader shader, float difColor[3], const std::string& specPath, const std::string& normPath)
{
	// Load shaders
	auto hashAndShaders = loadShader(shader);

	// Load buffers
	auto vsPsBuffers = loadBuffers(shader);

	// Load textures
	Material::PhongMaps maps;

	// Check hash first before loading texture!
	std::string texPathToHash = std::to_string(difColor[0] + difColor[1] + difColor[2]) + specPath + normPath;
	size_t texHash = std::hash<std::string>{}(texPathToHash);

	std::shared_ptr<Material> matToAdd = nullptr;
	if (!m_materialRepo.exists(texHash))
	{
		maps.diffuse = createTextureFromColor(difColor);
		maps.specular = loadTexture(specPath);
		maps.normal = loadTexture(normPath);

		matToAdd = std::make_shared<Material>(hashAndShaders.second, maps, texHash, hashAndShaders.first);
	}
	std::shared_ptr<Material> mat = m_materialRepo.add(texHash, matToAdd);			// If hash already exists, the existing mat will be returned and matToAdd will be discarded
	mat->setBuffers(vsPsBuffers.first, vsPsBuffers.second);

	return mat;
}
