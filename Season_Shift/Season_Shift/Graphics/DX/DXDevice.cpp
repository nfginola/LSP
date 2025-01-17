#include "pch.h"
#include "DXDevice.h"

using Microsoft::WRL::ComPtr;

DXDevice::DXDevice(HWND& hwnd, UINT clientWidth, UINT clientHeight) :
	m_vOffsetBind(0),
	m_currTargetBind({ })
{
	m_core = std::make_shared<DXCore>(hwnd, clientWidth, clientHeight);

	DXTexture::Desc dsc = {};
	dsc.type = DXTexture::Type::TEX2D;
	m_backbufferText = std::make_shared<DXTexture>(m_core, m_core->getBackbufferTexture(), dsc);
	m_backbufferText->m_rtv = m_core->getBackbufferRTV();
	m_backbufferText->m_srv = m_core->getBackbufferSRV();

	int a = 5;
}

DXDevice::~DXDevice()
{
}

std::shared_ptr<DXShader> DXDevice::createShader(const std::string& fileName, DXShader::Type shaderType)
{
	std::string shaderData = loadCompiledShader(COMPILED_SHADERS_DIRECTORY + fileName);

	ComPtr<ID3D11VertexShader> vs = nullptr;
	ComPtr<ID3D11HullShader> hs = nullptr;
	ComPtr<ID3D11DomainShader> ds = nullptr;
	ComPtr<ID3D11GeometryShader> gs = nullptr;
	ComPtr<ID3D11PixelShader> ps = nullptr;
	ComPtr<ID3D11ComputeShader> cs = nullptr;

	std::any shader;
	switch (shaderType)
	{
	case DXShader::Type::VS:
		HRCHECK(m_core->getDevice()->CreateVertexShader(shaderData.data(), shaderData.size(), nullptr, vs.GetAddressOf()));
		shader = vs;
		break;
	case DXShader::Type::HS:
		HRCHECK(m_core->getDevice()->CreateHullShader(shaderData.data(), shaderData.size(), nullptr, hs.GetAddressOf()));
		shader = hs;
		break;
	case DXShader::Type::DS:
		HRCHECK(m_core->getDevice()->CreateDomainShader(shaderData.data(), shaderData.size(), nullptr, ds.GetAddressOf()));
		shader = ds;
		break;
	case DXShader::Type::GS:
		HRCHECK(m_core->getDevice()->CreateGeometryShader(shaderData.data(), shaderData.size(), nullptr, gs.GetAddressOf()));
		shader = gs;
		break;
	case DXShader::Type::PS:
		HRCHECK(m_core->getDevice()->CreatePixelShader(shaderData.data(), shaderData.size(), nullptr, ps.GetAddressOf()));
		shader = ps;
		break;
	case DXShader::Type::CS:
		HRCHECK(m_core->getDevice()->CreateComputeShader(shaderData.data(), shaderData.size(), nullptr, cs.GetAddressOf()));
		shader = cs;
		break;
	default:
		assert(false);
	}

	std::shared_ptr<DXShader> shd = std::make_shared<DXShader>(shader, shaderType, shaderData);
	return shd;

}

std::shared_ptr<DXBuffer> DXDevice::createVertexBuffer(unsigned int elementCount, unsigned int elementStride, bool dynamic, bool cpuUpdates, bool streamOut, D3D11_SUBRESOURCE_DATA* subres)
{
	D3D11_BUFFER_DESC desc = { };
	desc.ByteWidth = elementCount * elementStride;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	if (streamOut)
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	else
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	if (dynamic && cpuUpdates)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}
	else
	{
		if (subres == nullptr)	assert(false);	// Immutable but no initial data!?!?

		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}

	ComPtr<ID3D11Buffer> buf = nullptr;
	HRCHECK(m_core->getDevice()->CreateBuffer(&desc, subres, buf.GetAddressOf()));

	// smart memory complaining at this? lol
	std::shared_ptr<DXBuffer> toRet = std::make_shared<DXBuffer>(m_core, buf, desc, DXBuffer::Type::Vertex, elementCount, elementStride);
	return toRet;

}

std::shared_ptr<DXBuffer> DXDevice::createIndexBuffer(unsigned int size, bool dynamic, D3D11_SUBRESOURCE_DATA* subres)
{
	D3D11_BUFFER_DESC desc = { };
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	if (dynamic)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		if (subres == nullptr)	assert(false);	// Immutable but no initial data!?!?

		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}

	unsigned int elementCount = size / sizeof(std::uint32_t);

	ComPtr<ID3D11Buffer> buf = nullptr;
	HRCHECK(m_core->getDevice()->CreateBuffer(&desc, subres, buf.GetAddressOf()));

	std::shared_ptr<DXBuffer> toRet = std::make_shared<DXBuffer>(m_core, buf, desc, DXBuffer::Type::Index, elementCount, sizeof(std::uint32_t));
	return toRet;
}

//kanske �ndra argumenten till gpuWritable-------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
std::shared_ptr<DXBuffer> DXDevice::createConstantBuffer(unsigned int size, bool dynamic, bool updateOnCPU, D3D11_SUBRESOURCE_DATA* subres) 
{
	if (size % 16 != 0) assert(false);	// Not 16 byte aligned!

	D3D11_BUFFER_DESC desc = { };
	desc.ByteWidth = size;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	if (dynamic && updateOnCPU)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (dynamic && !updateOnCPU)	
	{
		desc.Usage = D3D11_USAGE_DEFAULT;	
		desc.CPUAccessFlags = 0;
	}
	else
	{
		if (subres == nullptr)	assert(false);	// Immutable but no initial data!?!?

		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}


	ComPtr<ID3D11Buffer> buf = nullptr;
	HRCHECK(m_core->getDevice()->CreateBuffer(&desc, subres, buf.GetAddressOf()));

	std::shared_ptr<DXBuffer> toRet = std::make_shared<DXBuffer>(m_core, buf, desc, DXBuffer::Type::Constant, 1, size);
	return toRet;
}

std::shared_ptr<DXBuffer> DXDevice::createStructuredBuffer(unsigned int count, unsigned int structSize, bool cpuWritable, bool gpuWritable, D3D11_SUBRESOURCE_DATA* subres)
{
	D3D11_BUFFER_DESC desc = { };
	desc.ByteWidth = count * structSize;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = structSize;

	if (!cpuWritable && !gpuWritable)
	{
		if (subres == nullptr)	assert(false);	// Immutable but no initial data!?!?

		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}
	else if (cpuWritable && !gpuWritable)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (!cpuWritable && gpuWritable)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}
	else // CPU and GPU writable.. can't do this simultaneously
	{
		assert(false);
	}

	//fix more fancy shit 
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.ElementWidth = structSize;
	srvDesc.Buffer.NumElements = count;

	ComPtr<ID3D11Buffer> buf = nullptr;
	HRCHECK(m_core->getDevice()->CreateBuffer(&desc, subres, buf.GetAddressOf()));

	std::shared_ptr<DXBuffer> toRet = std::make_shared<DXBuffer>(m_core, buf, desc, DXBuffer::Type::Structured, count, structSize);

	ComPtr<ID3D11ShaderResourceView> srv;
	m_core->getDevice()->CreateShaderResourceView(buf.Get(), &srvDesc, &srv);
	toRet->setSRV(srv);
	return toRet;
}

std::shared_ptr<DXBuffer> DXDevice::createAppendConsumeBuffer(unsigned int count, unsigned int structSize, bool cpuWritable, bool gpuWritable, D3D11_SUBRESOURCE_DATA* subres)
{
	D3D11_BUFFER_DESC desc = { };
	desc.ByteWidth = count * structSize;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = structSize;

	if (!cpuWritable && !gpuWritable)
	{
		assert(false);// suppoerted???
		if (subres == nullptr)	assert(false);	// Immutable but no initial data!?!?

		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.CPUAccessFlags = 0;
	}
	else if (cpuWritable && !gpuWritable)
	{
		assert(false);// suppoerted???
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (!cpuWritable && gpuWritable)
	{
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
	}
	else // CPU and GPU writable.. can't do this simultaneously
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.ElementWidth = structSize;
	srvDesc.Buffer.NumElements = count;

	//fix more fancy shit 
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = count;

	ComPtr<ID3D11Buffer> buf = nullptr;
	HRCHECK(m_core->getDevice()->CreateBuffer(&desc, subres, buf.GetAddressOf()));

	std::shared_ptr<DXBuffer> toRet = std::make_shared<DXBuffer>(m_core, buf, desc, DXBuffer::Type::Structured, count, structSize);

	ComPtr<ID3D11UnorderedAccessView> uav;
	m_core->getDevice()->CreateUnorderedAccessView(buf.Get(), &uavDesc, &uav);
	toRet->setUAV(uav);

	ComPtr<ID3D11ShaderResourceView> srv;
	m_core->getDevice()->CreateShaderResourceView(buf.Get(), &srvDesc, &srv);
	toRet->setSRV(srv);

	return toRet;
}

std::shared_ptr<DXBuffer> DXDevice::createIndirectArgumentBuffer(unsigned int vertexCountPerInstance , unsigned int instanceCount, unsigned int startVertexLocation, unsigned int startInstanceLocation)
{
	D3D11_BUFFER_DESC desc = { };
	desc.BindFlags = 0;
	desc.ByteWidth = 16;
	desc.CPUAccessFlags = 0;
	desc.StructureByteStride = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;

	struct IndirectDrawArg
	{
		UINT a, b, c, d;
	} args = { vertexCountPerInstance, instanceCount, startVertexLocation, startVertexLocation };

	D3D11_SUBRESOURCE_DATA srDataForArg = { 0 };
	srDataForArg.pSysMem = &args;

	ComPtr<ID3D11Buffer> buf = nullptr;
	HRCHECK(m_core->getDevice()->CreateBuffer(&desc, &srDataForArg, buf.GetAddressOf()));

	std::shared_ptr<DXBuffer> toRet = std::make_shared<DXBuffer>(m_core, buf, desc, DXBuffer::Type::Indirect_args, 4, 4);

	return toRet;
}

std::shared_ptr<DXTexture> DXDevice::createTexture(const DXTexture::Desc& desc, D3D11_SUBRESOURCE_DATA* subres)
{
	std::shared_ptr<DXTexture> tex = nullptr;


	if (desc.type == DXTexture::Type::TEX1D)
	{
		ComPtr<ID3D11Texture1D> t = nullptr;
		HRCHECK(m_core->getDevice()->CreateTexture1D(&desc.desc1D, subres, t.GetAddressOf()));
		tex = std::make_shared<DXTexture>(m_core, t, desc);

		if (desc.desc1D.BindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			ComPtr<ID3D11ShaderResourceView> srv;
			D3D11_SHADER_RESOURCE_VIEW_DESC mtSRV = { };

			mtSRV.Format = desc.desc1D.Format;
			mtSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			mtSRV.Texture1D.MostDetailedMip = 0;
			mtSRV.Texture1D.MipLevels = 1;

			HRCHECK(m_core->getDevice()->CreateShaderResourceView(t.Get(), &mtSRV, srv.GetAddressOf()));
			tex->setSRV(srv);
		}
	}
	else if (desc.type == DXTexture::Type::TEX2D)
	{
		ComPtr<ID3D11Texture2D> t = nullptr;
		HRCHECK(m_core->getDevice()->CreateTexture2D(&desc.desc2D, subres, t.GetAddressOf()));
		tex = std::make_shared<DXTexture>(m_core, t, desc);

		/*
		Temporary below! We should be able to branch to SRV, DSV, RTV and UAV
		*/

		// SRV
		if (desc.desc2D.BindFlags & D3D11_BIND_SHADER_RESOURCE)
		{
			ComPtr<ID3D11ShaderResourceView> srv;
			D3D11_SHADER_RESOURCE_VIEW_DESC mtSRV = { };
			mtSRV.Format = desc.desc2D.Format;

			if (desc.desc2D.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
			{
				mtSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				mtSRV.TextureCube.MipLevels = 1;
				mtSRV.TextureCube.MostDetailedMip = 0;
			}
			else if (desc.desc2D.BindFlags & D3D11_BIND_DEPTH_STENCIL)
			{
				mtSRV.Format = DXGI_FORMAT_R32_FLOAT;
				mtSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				mtSRV.Texture2D.MostDetailedMip = 0;
				mtSRV.Texture2D.MipLevels = 1;
			}
			else
			{
				mtSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				mtSRV.Texture2D.MostDetailedMip = 0;
				mtSRV.Texture2D.MipLevels = desc.desc2D.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS ? -1 : 1;
			}


			HRCHECK(m_core->getDevice()->CreateShaderResourceView(t.Get(), &mtSRV, srv.GetAddressOf()));

			if (desc.desc2D.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS)
			{
				m_core->getImmediateContext()->GenerateMips(srv.Get());
				tex->setSRV(srv);
				return tex; //return early to skip rendertarget creation for miped textures
			}

			tex->setSRV(srv);
		}
		if (desc.desc2D.BindFlags & D3D11_BIND_DEPTH_STENCIL)
		{
			ComPtr<ID3D11DepthStencilView> dsv;
			D3D11_DEPTH_STENCIL_VIEW_DESC mtDSV = { };
			mtDSV.Format = desc.desc2D.Format;

			if (desc.desc2D.BindFlags & D3D11_BIND_SHADER_RESOURCE)		// shadow map
				mtDSV.Format = DXGI_FORMAT_D32_FLOAT;

			mtDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			mtDSV.Texture2D.MipSlice = 0;

			HRCHECK(m_core->getDevice()->CreateDepthStencilView(t.Get(), &mtDSV, dsv.GetAddressOf()));

			tex->setDSV(dsv);
		}
		if (desc.desc2D.BindFlags & D3D11_BIND_RENDER_TARGET)
		{
			ComPtr<ID3D11RenderTargetView> rtv;
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = { };
			rtvDesc.Format = desc.desc2D.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			HRCHECK(m_core->getDevice()->CreateRenderTargetView(t.Get(), &rtvDesc, rtv.GetAddressOf()));

			tex->setRTV(rtv);
		}


	}
	// TEX3D
	else
	{
		ComPtr<ID3D11Texture3D> t = nullptr;
		HRCHECK(m_core->getDevice()->CreateTexture3D(&desc.desc3D, subres, t.GetAddressOf()));
		tex = std::make_shared<DXTexture>(m_core, t, desc);
	}


	return tex;
}

ComPtr<ID3D11InputLayout> DXDevice::createInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, const std::string& shaderData)
{
	ComPtr<ID3D11InputLayout> inputLayout = nullptr;

	HRCHECK(m_core->getDevice()->CreateInputLayout(elements.data(), static_cast<unsigned int>(elements.size()), shaderData.data(), static_cast<unsigned int>(shaderData.size()), inputLayout.GetAddressOf()));

	return inputLayout;
}

Microsoft::WRL::ComPtr<ID3D11SamplerState> DXDevice::createSamplerState(D3D11_SAMPLER_DESC desc)
{
	ComPtr<ID3D11SamplerState> sampler;

	HRCHECK(m_core->getDevice()->CreateSamplerState(&desc, sampler.GetAddressOf()));
	return sampler;
}

Microsoft::WRL::ComPtr<ID3D11RasterizerState> DXDevice::createRasterizerState(D3D11_RASTERIZER_DESC desc)
{
	ComPtr<ID3D11RasterizerState> rss;
	HRCHECK(m_core->getDevice()->CreateRasterizerState(&desc, rss.GetAddressOf()));
	return rss;
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DXDevice::createDepthStencilState(D3D11_DEPTH_STENCIL_DESC desc)
{
	ComPtr<ID3D11DepthStencilState> dss;
	HRCHECK(m_core->getDevice()->CreateDepthStencilState(&desc, dss.GetAddressOf()));
	return dss;
}

Microsoft::WRL::ComPtr<ID3D11BlendState> DXDevice::createBlendState(D3D11_BLEND_DESC desc)
{
	ComPtr<ID3D11BlendState> bs;
	HRCHECK(m_core->getDevice()->CreateBlendState(&desc, bs.GetAddressOf()));
	return bs;
}

/* 2D Helper Methods */

std::shared_ptr<DirectX::SpriteBatch> DXDevice::createSpriteBatch()
{
	return std::make_shared<DirectX::SpriteBatch>(m_core->getImmediateContext().Get());
}

std::shared_ptr<DirectX::SpriteFont> DXDevice::createSpriteFont(const std::wstring& fontPath)
{
	return std::make_shared<DirectX::SpriteFont>(m_core->getDevice().Get(), fontPath.c_str());
}

/* Bind methods */

void DXDevice::bindShader(const std::shared_ptr<DXShader>& shader, DXShader::Type stage)
{

	switch (stage)
	{
	case DXShader::Type::VS:
		if (shader == nullptr)
		{
			m_core->getImmediateContext()->VSSetShader(nullptr, 0, 0);
			return;
		}
		m_core->getImmediateContext()->VSSetShader(shader->getShader<ID3D11VertexShader>(), 0, 0);
		break;
	case DXShader::Type::HS:
		if (shader == nullptr)
		{
			m_core->getImmediateContext()->HSSetShader(nullptr, 0, 0);
			return;
		}
		m_core->getImmediateContext()->HSSetShader(shader->getShader<ID3D11HullShader>(), 0, 0);
		break;
	case DXShader::Type::DS:
		if (shader == nullptr)
		{
			m_core->getImmediateContext()->DSSetShader(nullptr, 0, 0);
			return;
		}
		m_core->getImmediateContext()->DSSetShader(shader->getShader<ID3D11DomainShader>(), 0, 0);
		break;
	case DXShader::Type::GS:
		if (shader == nullptr)
		{
			m_core->getImmediateContext()->GSSetShader(nullptr, 0, 0);
			return;
		}
		m_core->getImmediateContext()->GSSetShader(shader->getShader<ID3D11GeometryShader>(), 0, 0);
		break;
	case DXShader::Type::PS:
		if (shader == nullptr)
		{
			m_core->getImmediateContext()->PSSetShader(nullptr, 0, 0);
			return;
		}
		m_core->getImmediateContext()->PSSetShader(shader->getShader<ID3D11PixelShader>(), 0, 0);
		break;
	case DXShader::Type::CS:
		if (shader == nullptr)
		{
			m_core->getImmediateContext()->CSSetShader(nullptr, 0, 0);
			return;
		}
		m_core->getImmediateContext()->CSSetShader(shader->getShader<ID3D11ComputeShader>(), 0, 0);
		break;
	default:
		assert(false);
		break;
	}
}

void DXDevice::bindShaderConstantBuffer(DXShader::Type stage, unsigned int slot, const std::shared_ptr<DXBuffer>& res)
{
	if (!(res->getDesc().BindFlags & D3D11_BIND_CONSTANT_BUFFER)) assert(false);

	switch (stage)
	{
	case DXShader::Type::VS:
		m_core->getImmediateContext()->VSSetConstantBuffers(slot, 1, (ID3D11Buffer**)res->getResource().GetAddressOf());
		break;
	case DXShader::Type::HS:
		m_core->getImmediateContext()->HSSetConstantBuffers(slot, 1, (ID3D11Buffer**)res->getResource().GetAddressOf());
		break;
	case DXShader::Type::DS:
		m_core->getImmediateContext()->DSSetConstantBuffers(slot, 1, (ID3D11Buffer**)res->getResource().GetAddressOf());
		break;
	case DXShader::Type::GS:
		m_core->getImmediateContext()->GSSetConstantBuffers(slot, 1, (ID3D11Buffer**)res->getResource().GetAddressOf());
		break;
	case DXShader::Type::PS:
		m_core->getImmediateContext()->PSSetConstantBuffers(slot, 1, (ID3D11Buffer**)res->getResource().GetAddressOf());
		break;
	case DXShader::Type::CS:
		m_core->getImmediateContext()->CSSetConstantBuffers(slot, 1, (ID3D11Buffer**)res->getResource().GetAddressOf());
		break;
	default:
		assert(false);
		break;
	}
}

void DXDevice::bindShaderStructuredBuffer(DXShader::Type stage, unsigned int slot, const std::shared_ptr<DXBuffer>& res)
{
	if (res == nullptr) //unbind
	{
		ID3D11ShaderResourceView* srvNull = nullptr;
		switch (stage)
		{
		case DXShader::Type::VS:
			m_core->getImmediateContext()->VSSetShaderResources(slot, 1, &srvNull);
			break;
		case DXShader::Type::HS:
			m_core->getImmediateContext()->HSSetShaderResources(slot, 1, &srvNull);
			break;
		case DXShader::Type::DS:
			m_core->getImmediateContext()->DSSetShaderResources(slot, 1, &srvNull);
			break;
		case DXShader::Type::GS:
			m_core->getImmediateContext()->GSSetShaderResources(slot, 1, &srvNull);
			break;
		case DXShader::Type::PS:
			m_core->getImmediateContext()->PSSetShaderResources(slot, 1, &srvNull);
			break;
		case DXShader::Type::CS:
			m_core->getImmediateContext()->CSSetShaderResources(slot, 1, &srvNull);
			break;
		default:
			assert(false);
			break;
		}
		return;
	}

	if (!(res->getType() == DXBuffer::Type::Structured)) assert(false);

	switch (stage)
	{
	case DXShader::Type::VS:
		m_core->getImmediateContext()->VSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::HS:
		m_core->getImmediateContext()->HSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::DS:
		m_core->getImmediateContext()->DSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::GS:
		m_core->getImmediateContext()->GSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::PS:
		m_core->getImmediateContext()->PSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::CS:
		m_core->getImmediateContext()->CSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	default:
		assert(false);
		break;
	}
}

void DXDevice::bindAppendConsumeBuffer(DXShader::Type stage, unsigned int slot, unsigned int offset, const std::shared_ptr<DXBuffer>& res)
{
	if (res == nullptr) //unbind
	{
		ID3D11UnorderedAccessView* uavNull = nullptr;
		m_core->getImmediateContext()->CSSetUnorderedAccessViews(slot, 1, &uavNull, nullptr);
		return;
	}
	if (!(res->getType() == DXBuffer::Type::Structured)) assert(false);

	if (DXShader::Type::CS == stage)
	{
		m_core->getImmediateContext()->CSSetUnorderedAccessViews(slot, 1, res->getUAV().GetAddressOf(), &offset);
	}
	else
	{
		assert(false);
	}
}

void DXDevice::bindShaderSampler(DXShader::Type stage, unsigned int slot, const Microsoft::WRL::ComPtr<ID3D11SamplerState>& res)
{
	switch (stage)
	{
	case DXShader::Type::VS:
		m_core->getImmediateContext()->VSSetSamplers(slot, 1, res.GetAddressOf());
		break;
	case DXShader::Type::HS:
		m_core->getImmediateContext()->HSSetSamplers(slot, 1, res.GetAddressOf());
		break;
	case DXShader::Type::DS:
		m_core->getImmediateContext()->DSSetSamplers(slot, 1, res.GetAddressOf());
		break;
	case DXShader::Type::GS:
		m_core->getImmediateContext()->GSSetSamplers(slot, 1, res.GetAddressOf());
		break;
	case DXShader::Type::PS:
		m_core->getImmediateContext()->PSSetSamplers(slot, 1, res.GetAddressOf());
		break;
	case DXShader::Type::CS:
		m_core->getImmediateContext()->CSSetSamplers(slot, 1, res.GetAddressOf());
		break;
	default:
		assert(false);
		break;
	}
}

void DXDevice::bindShaderTexture(DXShader::Type stage, unsigned int slot, const std::shared_ptr<DXTexture> res)
{
	ID3D11ShaderResourceView* nullView[] = { nullptr };

	switch (stage)
	{
	case DXShader::Type::VS:
		if (!res)
		{
			m_core->getImmediateContext()->VSSetShaderResources(slot, 1, nullView);
			return;
		}
		m_core->getImmediateContext()->VSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::HS:
		if (!res)
		{
			m_core->getImmediateContext()->HSSetShaderResources(slot, 1, nullView);
			return;
		}
		m_core->getImmediateContext()->HSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::DS:
		if (!res)
		{
			m_core->getImmediateContext()->DSSetShaderResources(slot, 1, nullView);
			return;
		}
		m_core->getImmediateContext()->DSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::GS:
		if (!res)
		{
			m_core->getImmediateContext()->GSSetShaderResources(slot, 1, nullView);
			return;
		}
		m_core->getImmediateContext()->GSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::PS:
		if (!res)
		{
			m_core->getImmediateContext()->PSSetShaderResources(slot, 1, nullView);
			return;
		}
		m_core->getImmediateContext()->PSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	case DXShader::Type::CS:
		if (!res)
		{
			m_core->getImmediateContext()->CSSetShaderResources(slot, 1, nullView);
			return;
		}
		m_core->getImmediateContext()->CSSetShaderResources(slot, 1, res->getSRV().GetAddressOf());
		break;
	default:
		assert(false);
		break;
	}
}

void DXDevice::bindInputLayout(const Microsoft::WRL::ComPtr<ID3D11InputLayout>& il)
{
	if (il == nullptr)
	{
		m_core->getImmediateContext()->IASetInputLayout(nullptr);
	}

	m_core->getImmediateContext()->IASetInputLayout(il.Get());
}

void DXDevice::bindInputTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_core->getImmediateContext()->IASetPrimitiveTopology(topology);
}

void DXDevice::bindDepthStencilState(const Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& dss, unsigned int stencilRef)
{
	if (dss == nullptr)
	{
		m_core->getImmediateContext()->OMSetDepthStencilState(nullptr, 0);
		return;
	}
	m_core->getImmediateContext()->OMSetDepthStencilState(dss.Get(), stencilRef);

}

void DXDevice::bindBlendState(const Microsoft::WRL::ComPtr<ID3D11BlendState>& bs, std::array<float, 4> blendFac, unsigned int sampleMask)
{
	if (bs == nullptr)
	{
		m_core->getImmediateContext()->OMSetBlendState(nullptr, blendFac.data(), sampleMask);
		return;
	}

	m_core->getImmediateContext()->OMSetBlendState(bs.Get(), blendFac.data(), sampleMask);
}

void DXDevice::bindRasterizerState(const Microsoft::WRL::ComPtr<ID3D11RasterizerState>& rss)
{
	if (rss == nullptr)
	{
		m_core->getImmediateContext()->RSSetState(nullptr);
		return;
	}

	m_core->getImmediateContext()->RSSetState(rss.Get());
}

void DXDevice::bindRenderTargets(const std::vector<std::shared_ptr<DXTexture>>& targets, const std::shared_ptr<DXTexture>& depthTarget)
{
	if (targets.size() == 0 && depthTarget != nullptr) 
	{
		m_core->getImmediateContext()->OMSetRenderTargets(0, { nullptr }, depthTarget->getDSV().Get());		// depth only pass
		return;
	}
	else if (targets.size() == 0)
	{
		m_core->getImmediateContext()->OMSetRenderTargets(0, { nullptr }, nullptr);
		return;
	}

	// Make sure depth target is of correct type, or is null
	if (depthTarget)
		if (depthTarget->getDesc().type != DXTexture::Type::TEX2D || depthTarget->getDSV() == nullptr) assert(false);

	for (auto& target : targets)
	{
		if (target)
			if (target->getDesc().type != DXTexture::Type::TEX2D || target->getRTV() == nullptr) assert(false);
	}
	for (unsigned int i = 0; i < targets.size(); ++i)
	{
		m_currTargetBind[i] = targets[i] ? targets[i]->getRTV().Get() : nullptr;
	}

	auto dsv = depthTarget ? depthTarget->getDSV().Get() : nullptr;

	m_core->getImmediateContext()->OMSetRenderTargets(static_cast<unsigned int>(targets.size()), m_currTargetBind.data(), dsv);

}

void DXDevice::bindDrawBuffer(const std::shared_ptr<DXBuffer>& vb)
{
	if (vb->getType() != DXBuffer::Type::Vertex) assert(false);

	m_core->getImmediateContext()->IASetVertexBuffers(0, 1, (ID3D11Buffer* const*)(vb->getResource().GetAddressOf()), &vb->getElementStride(), &m_vOffsetBind);

}

void DXDevice::bindViewports(const std::vector<D3D11_VIEWPORT>& vps)
{
	m_core->getImmediateContext()->RSSetViewports(vps.size(), vps.data());
}

void DXDevice::copyStructureCount(const std::shared_ptr<DXBuffer>& constantBuffer, const std::shared_ptr<DXBuffer>& structuredBuffer)
{
	assert(structuredBuffer->getType() == DXBuffer::Type::Structured);
	//borde anv�nda static_cast f�r att det fick vi l�ra oss i c++ kursen, men vi fick inte l�ra oss varf�r s�...
	m_core->getImmediateContext()->CopyStructureCount((ID3D11Buffer*)constantBuffer->getResource().Get(), 0, structuredBuffer->getUAV().Get());
}

void DXDevice::draw(unsigned int vtxCount, unsigned int vbStartIdx)
{
	m_core->getImmediateContext()->Draw(vtxCount, vbStartIdx);
}

void DXDevice::drawIndexed(unsigned int idxCount, unsigned int ibStartIdx, unsigned int vbStartIdx)
{
	m_core->getImmediateContext()->DrawIndexed(idxCount, ibStartIdx, vbStartIdx);
}

void DXDevice::drawIndexedInstanced(unsigned int idxCountPerInst, unsigned int instCount, unsigned int ibStartIdx, unsigned int vbStartIdx, unsigned int instStartIdx)
{
	m_core->getImmediateContext()->DrawIndexedInstanced(idxCountPerInst, instCount, ibStartIdx, vbStartIdx, instStartIdx);
}

void DXDevice::drawInstancedIndirect(const std::shared_ptr<DXBuffer>& argumentBuffer, unsigned int alignedByteOffsetForArgs)
{
	m_core->getImmediateContext()->DrawInstancedIndirect(static_cast<ID3D11Buffer*>(argumentBuffer->getResource().Get()), alignedByteOffsetForArgs);
}

void DXDevice::dispatch(unsigned int threadGroupCountX, unsigned int threadGroupCountY, unsigned int threadGroupCountZ)
{
	m_core->getImmediateContext()->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void DXDevice::clearRenderTarget(const std::shared_ptr<DXTexture>& target, float color[4])
{
	if (target->getRTV() == nullptr) assert(false);

	m_core->getImmediateContext()->ClearRenderTargetView(target->getRTV().Get(), color);
}

void DXDevice::clearDepthTarget(const std::shared_ptr<DXTexture>& depthTarget, unsigned int clearFlag, float depth, float stencil)
{
	if (depthTarget == nullptr)
	{
		return;
	}

	m_core->getImmediateContext()->ClearDepthStencilView(depthTarget->getDSV().Get(), clearFlag, depth, stencil);
}

void DXDevice::onResize(UINT width, UINT height)
{

	m_core->getImmediateContext()->OMSetRenderTargets(0, nullptr, nullptr);
	m_core->getImmediateContext()->ClearState();
	m_core->getImmediateContext()->Flush();

	/*for (auto& rtv : m_currTargetBind)
	{
		if (rtv == nullptr) continue;
		rtv->Release();
	}*/
	
	m_backbufferText->m_resource.Reset();
	m_backbufferText->m_rtv.Reset();
	m_backbufferText->m_srv.Reset();

	m_core->onResize(width, height);


	



	//from constructor
	DXTexture::Desc dsc = {};
	dsc.type = DXTexture::Type::TEX2D;
	m_backbufferText = std::make_shared<DXTexture>(m_core, m_core->getBackbufferTexture(), dsc);
	m_backbufferText->m_rtv = m_core->getBackbufferRTV();
	m_backbufferText->m_srv = m_core->getBackbufferSRV();




}

void DXDevice::setFullScreen(bool fullScreen)
{
	m_core->setFullScreen(fullScreen);
}

bool DXDevice::getFullScreenState() const
{
	return m_core->getFullScreenState();
}

void DXDevice::bindDrawIndexedBuffer(const std::shared_ptr<DXBuffer>& vb, const std::shared_ptr<DXBuffer>& ib, unsigned int vbOffset, unsigned int ibOffset)
{

	if (vb == nullptr || ib == nullptr)
	{
		ID3D11Buffer* nullBufs[] = { nullptr, nullptr, nullptr };
		UINT stride = 0;
		UINT offset = 0;
		m_core->getImmediateContext()->IASetVertexBuffers(0, 3, nullBufs, &stride, &offset);
		m_core->getImmediateContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, ibOffset);
		return;
	}


	if (vb->getType() != DXBuffer::Type::Vertex || ib->getType() != DXBuffer::Type::Index) assert(false);
	m_vOffsetBind = vbOffset;

	m_core->getImmediateContext()->IASetVertexBuffers(0, 1, (ID3D11Buffer* const*)vb->getResource().GetAddressOf(), &vb->getElementStride(), &m_vOffsetBind);
	m_core->getImmediateContext()->IASetIndexBuffer((ID3D11Buffer*)ib->getResource().Get(), DXGI_FORMAT_R32_UINT, ibOffset);
}

void DXDevice::clearScreen()
{
	FLOAT color[] = { 0.0, 0.0, 0.0, 1.0 };
	m_core->getImmediateContext()->ClearRenderTargetView(m_core->getBackbufferRTV().Get(), color);
}

void DXDevice::present()
{
	m_core->getSwapChain()->Present(0, 0);
}

//void DXDevice::bindBackBufferAsTarget(const std::shared_ptr<DXTexture>& depthTarget)
//{
//
//	if (depthTarget != nullptr) 
//		m_core->getImmediateContext()->OMSetRenderTargets(1, m_core->getBackbufferRTV().GetAddressOf(), depthTarget->getDSV().Get());
//	else
//		m_core->getImmediateContext()->OMSetRenderTargets(1, m_core->getBackbufferRTV().GetAddressOf(), nullptr);
//
//	m_core->getImmediateContext()->RSSetViewports(1, m_core->getBackBufferViewport());
//
//
//}

const std::shared_ptr<DXTexture>& DXDevice::getBackbuffer()
{
	return m_backbufferText;
}

D3D11_VIEWPORT* DXDevice::getBackbufferViewport()
{
	return m_core->getBackBufferViewport();
}

//const std::shared_ptr<DXCore>& DXDevice::getCore()
//{
//	return m_core;
//}

UINT DXDevice::getClientWidth()
{
	return m_core->getClientWidth();
}

UINT DXDevice::getClientHeight()
{
	return m_core->getClientHeight();
}