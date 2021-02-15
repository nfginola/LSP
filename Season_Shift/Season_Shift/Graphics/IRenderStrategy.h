#pragma once
#include "GfxRenderer.h"
#include "Model.h"
#include "DX/DXRenderPass.h"
#include "../Camera.h"

/*

Works as an interface to be able to select different rendering implementations (e.g forward or deferred)
- Render Strategies create and use specific graphics resources in order to implement techniques.
- The meat of graphics techniques lies in a Render Strategy

*/
class IRenderStrategy
{
protected:
	std::shared_ptr<GfxRenderer> m_renderer;
	UINT m_clientWidth;
	UINT m_clientHeight;

public:
	IRenderStrategy(std::shared_ptr<GfxRenderer> renderer);
	virtual ~IRenderStrategy();
	virtual void render(const std::vector<std::shared_ptr<Model>>& models, const std::shared_ptr<Camera>& mainCamera) = 0;
	virtual void setUp() = 0;
};

