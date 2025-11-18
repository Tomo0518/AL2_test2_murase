#pragma once

#include "GameSceneBase.h"
#include "GameShared.h"
#include "Pad.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include "ButtonManager.h"

class SceneManager;

class StageSelectScene : public GameSceneBase {
public:
	StageSelectScene(SceneManager& manager, GameShared& shared);

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

private:
	SceneManager& manager_;
	GameShared& shared_;

	// ボタンマネージャー
	ButtonManager buttonManager_;
	void InitializeButtons(); // ボタン初期化

	// フォント
	FontAtlas font_;
	TextRenderer text_;
	bool fontReady_ = false;

	//=========================
	// 描画類
	//=========================
	// grHandleやコンポーネントの初期化
	void InitializeDrawComponents();
	void UpdateDrawComponents(float deltaTime);

	// ボタン用テクスチャ
	int grHandleButton_ = -1;

	// 背景テクスチャ
	int grHandleBackground_ = -1;
	DrawComponent2D drawCompBackground_;

	// 入力受付の遅延タイマー
	int inputDelayTimer_ = 0;
	const int kInputDelayFrames_ = 30;  // フレーム数ベースの遅延
	bool inputEnabled_ = false;
};