#pragma once
#include <memory>
#include <vector>
#include <string>
#include "DXCore.h"
#include "DXBuffer.h"
#include "DXTexture.h"
#include "DXShader.h"


struct PipelineState
{
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState = nullptr;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	std::shared_ptr<DXShader> vs = nullptr;	
	std::shared_ptr<DXShader> hs = nullptr;
	std::shared_ptr<DXShader> ds = nullptr;
	std::shared_ptr<DXShader> gs = nullptr;
	std::shared_ptr<DXShader> ps = nullptr;
};

struct RenderPass
{
	std::shared_ptr<PipelineState> pipeline = nullptr;
	std::vector<DXTexture> textureOutputs = {};			// RTV
	std::vector<D3D11_RECT> scissorRects = {};
	std::vector<DXTexture> textureOutputsUAV = {};		// UAV
};


/*

Works as an abstract 'Device' for low-level graphics resource creations AND as a 'Device Context' for low level binds
- DXTexture
- DXBuffer
- etc.

*/
class DXDevice
{
private:
	DXCore m_core;



	/*
	
	NOTE TO SELF:

	- Check out "nullBlob" in Wicked Engine DX11 for Unbinding purposes
	- Make function BindConstantBuffer that simply takes in an enum for shader stage and uses a switch to determine --> bind(vs, slot, res) (Again, check Wicked Engine for reference!)
	- Add explicit "= delete" for classes we know that are NOT allowed to be copied?
	- No great idea on how the resource manager should work for now.. Add, remove.. but what kind of identifier? string? number? what kind of datastructure? map? unordered_map?
		--> Depends on functionality and what it needs to be used for

	- This layer helps us keep a shadow-state for binding purposes so re-binds of the same resource do not occur!
	--> For example, by using this class when binding, we can identify nullptrs and avoid any Graphics API calls.
	
	*/

public:
	DXDevice(HWND& hwnd, UINT clientWidth, UINT clientHeight);
	~DXDevice();

	/*
	
	std::shared_ptr<DXShader>				createShader(id, shaderstage);

	// These two below are most important as they are closely tied to specific game assets (VBs, IBs and Material for mesh) --> May require std::hash string to std::size_t in GfxResourceDevice
	std::shared_ptr<DXBuffer>				createBuffer(id, desc)
	std::shared_ptr<DXTexture>				createTexture(id, desc);				// Dont forget about mip levels! (e.g Generate Mips)

	ComPtr<ID3D11InputLayout>				createInputLayout(desc);				// We can let this live in PipelineState

	ComPtr<ID3D11BlendState>				createBlendState(desc);					--> If previous objects has been created with same state -> Previous instance returned by D3D11 instead of duplicate! (remarks on MSDN)
	ComPtr<ID3D11RasterizerState>			createRasterizerState(desc);			--> Same as above (remarks on MSDN)
	ComPtr<ID3D11DepthStencilState>			createDepthStencilState(desc);			--> Same as above
	ComPtr<ID3D11SamplerState>				createSamplerState(desc);				--> Same as above
	// Duplicate resources given back by D3D11 requires no repository as it simply gives us a reference to an existing resource we have.
	// We can create above 4 resources without having to worry about duplicate resources.



	std::shared_ptr<DXPipelineState>		createPipelineState(pipelineStateDesc);
	DXPipelineStateDesc
	{
		il*,		IA					
		dss*,		OM					Toggle depth/stencil, depth op
		bs*,		OM					...
		rs*,		RS					FillMode (Wireframe), Toggle scissors, AA line, MS, 
		it*,		IA
		vs*, hs*, ds*, gs*, ps*			IL and IT closely related to VS/GS
	}

	std::shared_ptr<DXRenderPass>			createRenderPass(renderPassDesc)
	DXRenderPassDesc
	{
		pipelineState
		vector<rtvs>*		OM			Output Textures
		vector<viewports>*	OM			Part to draw to in said Output Textures
		scissors*			RS
		vector<uavs>*		OM
	}

	*/


	/*
	
	// Graphics resource upload functions
	Map/Unmap
	UpdateSubresource


	// Shader binds
	void bindConstantBuffer(shaderstage, slot, resource)
	void bindSampler(shaderstage, slot, resource)
	void bindTexture(shaderstage, slot, type, resource)



	// Changes for every new geometry to be drawn
	void bindInputBuffers(vb, ib)

	// Fixed Pipeline Stuff (that changes less often --> these changes are more dependent on technique passes rather than different draw calls)
	// For example, 'input layout' and 'input topology' may change once we want to use a "line renderer" instead of a traditional "triangle renderer"
	// Resources bound below are expected to live in a PipelineState

	// Resources inside a "Pipeline State"
	void bindInputLayout()			// IA		--> Draw bound
	void bindInputTopology()		// IA		--> Draw bound
	void bindDepthStencilState();	// OM
	void bindBlendState();			// OM
	void bindRasterizerState();		// RS

	// Resources inside a "Render Pass" --> Uses a Pipeline State?
	void bindRTV();					// OM
	void bindRTVUAV();				// OM
	void bindViewports();			// RS
	void bindScissors();			// RS


	// Utility
	void clearRTV();
	void clearDSV();
	void generateMips();


	*/


	/*
	Defaults to clearing to black
	*/
	void clearScreen();

	/*
	Currently no vsync options
	*/
	void present();

};

