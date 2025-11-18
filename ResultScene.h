#pragma once

#include "GameShared.h"
#include "TextRenderer.h"
#include "GameSceneBase.h"
#include "DrawComponent2D.h"
#include "ButtonManager.h"

class SceneManager;

class ResultScene : public GameSceneBase {
public:
	ResultScene(SceneManager& mgr, GameShared& shared);

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

private:
	void InitializeButtons();

	SceneManager& manager_;
	GameShared& shared_;

	// ボタン管理
	ButtonManager buttonManager_;
	int grHandleButton_ = 0;

	// フォント
	bool fontReady_ = false;
	FontAtlas font_;
	TextRenderer text_;

	// 背景・UI
	DrawComponent2D* drawCompBgDraw_;
	DrawComponent2D* drawCompClearLabel_;
};