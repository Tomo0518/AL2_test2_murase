#include "TitleScene.h"
#include "GameShared.h"
#include "Easing.h"
#include <Novice.h>
#include <cstring>
#include <algorithm>
#include "WindowSize.h"
#include "SceneManager.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

TitleScene::TitleScene(SceneManager& manager, GameShared& shared)
	:manager_(manager),
	shared_(shared) {

	// フォント読み込み
	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}

	// ボタン初期化
	InitializeButtons();

	InitializeDrawComponents();

	shared_.PlayExclusive_(BgmKind::Title);
}

void TitleScene::InitializeDrawComponents() {
	// 背景テクスチャ
	grHandleBackground_ = Novice::LoadTexture("./Resources/images/title/background_ver1.png");
	// ロゴテクスチャ
	grHandleLogo_ = Novice::LoadTexture("./Resources/images/title/logo_ver1.png");
	int logoWidth = 0;
	int logoHeight = 0;

	Novice::GetTextureSize(grHandleLogo_, &logoWidth, &logoHeight);

	drawCompBackground_ = DrawComponent2D(
		{ kWindowWidth / 2.0f, kWindowHeight / 2.0f }, // 描画する中心座標
		1280.0f, 720.0f, // スクリーンに表示する横、縦幅
		Novice::LoadTexture("./Resources/images/title/background_ver1.png"), // グラフィックハンドル
		1280, 720,  // 画像の1フレームの横幅、縦幅
		1,		    // フレーム数
		1,			// 一行の分割数(基本フレーム数と同じでいい)
		0.3f,		// アニメーションの再生速度
		true		// ループ再生するかどうか
	);

	drawCompLogo_ = DrawComponent2D(
		{ kWindowWidth / 2.0f,  kWindowHeight / 2.0f }, // 描画する中心座標
		(float)logoWidth, (float)logoHeight, // スクリーンに表示する横、縦幅
		grHandleLogo_, // グラフィックハンドル
		logoWidth, logoHeight,  // 画像の1フレームの横幅、縦幅
		1,		    // フレーム数
		1,			// 一行の分割数(基本フレーム数と同じでいい)
		0.3f,		// アニメーションの再生速度
		true		// ループ再生するかどうか
	);

	drawCompBackground_.StopAnimation();
	drawCompBackground_.PlayAnimation();

	// ロゴ拡大縮小アニメーション開始
	drawCompLogo_.StopAnimation();
	drawCompLogo_.PlayAnimation();

	drawCompLogo_.StartScaleContinuous(0.9f, 1.1f, 0.5f);
}

void TitleScene::Update(float dt, const char* keys, const char* pre) {
	UpdateDrawComponents(dt);
	buttonManager_.Update(dt, keys, pre, shared_.pad);
}

void TitleScene::Draw() {

	// 背景描画
	drawCompBackground_.DrawAnimationScreen();

	// ロゴ描画
	drawCompLogo_.DrawAnimationScreen();

	buttonManager_.Draw(
		grHandleButton_,
		&font_,
		&text_
	);
}

void TitleScene::InitializeButtons() {

	// ボタン用の白いテクスチャ
	grHandleButton_ = shared_.texWhite;

	// ボタンの位置とサイズ
	const float centerX = 1080.0f;
	const float startY = 500.0f;
	const float buttonSpacing = 80.0f;
	const Vector2 buttonSize = { 270.0f, 60.0f };

	auto goToStageSelect = [&]() {
		manager_.RequestStageSelect();
		};

	auto goToSettings = [&]() {
		manager_.RequestOpenSettings();
		};

	auto quitGame = [&]() {
		manager_.RequestQuit();
		};

	// 3つのボタンを追加
	buttonManager_.AddButton({ centerX,startY }, buttonSize, "Play", goToStageSelect);

	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing },
		buttonSize,
		"SETTING",
		goToSettings
	);

	buttonManager_.AddButton(
		Vector2{ centerX, startY + buttonSpacing * 2 },
		buttonSize,
		"QUIT",
		quitGame
	);

	// SE設定
	buttonManager_.SetOnSelectSound([&]() {
		shared_.PlaySelectSe();
		});

	buttonManager_.SetOnDecideSound([&]() {
		shared_.PlayDecideSe();
		});

	// 初期選択をリセット
	buttonManager_.SetFirstFrame(true);
}

void TitleScene::UpdateDrawComponents(float dt) {
	drawCompBackground_.UpdateAnimation(dt);
	drawCompLogo_.UpdateAnimation(dt);
	drawCompLogo_.UpdateEffects(dt);
}