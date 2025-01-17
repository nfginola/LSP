#pragma once
#include "../Scene.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/2D/ISprite.h"
#include "../SceneManager.h"

class StageSelect : public Scene
{
private:
	std::shared_ptr<ISprite> m_title;
	std::shared_ptr<ISprite> m_backButton;
	std::shared_ptr<ISprite> m_backButtonDown;
	std::shared_ptr<ISprite> m_backText;
	std::shared_ptr<ISprite> m_stage1Button;
	std::shared_ptr<ISprite> m_stage1ButtonDown;
	std::shared_ptr<ISprite> m_stage1ButtonText;
	std::shared_ptr<ISprite> m_stage2Button;
	std::shared_ptr<ISprite> m_stage2ButtonDown;
	std::shared_ptr<ISprite> m_stage2ButtonText;

	bool m_firstSetup = true;

public:
	StageSelect(Graphics* gph, SceneManager* sm);
	~StageSelect() = default;

	void setUpScene() override;
};

