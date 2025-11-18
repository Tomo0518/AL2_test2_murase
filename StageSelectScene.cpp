#include "StageSelectScene.h"
#include "SceneManager.h"
#include "SceneType.h"

StageSelectScene::StageSelectScene(SceneManager& manager, GameShared& shared)
	: manager_(manager), shared_(shared) {

	// フォント読み込み
	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}

	// ボタン初期化
	InitializeButtons();

	// 描画コンポーネント初期化
	InitializeDrawComponents();

	shared_.PlayExclusive_(BgmKind::Title);

	inputDelayTimer_ = kInputDelayFrames_;
}

void StageSelectScene::Update(float dt, const char* keys, const char* pre) {

	// 入力受付の遅延処理
	if (inputDelayTimer_ > 0) {
		--inputDelayTimer_;
		if (inputDelayTimer_ == 0) {
			inputEnabled_ = true;
		}
	}

	if (inputEnabled_) {
		// ボタンマネージャーの更新
		buttonManager_.Update(dt, keys, pre, shared_.pad);

		// Escapeキーで戻る
		if (!pre[DIK_ESCAPE] && keys[DIK_ESCAPE]) {
			shared_.PlayBackSe();
			manager_.RequestBackToTitle();
		}
	}
}

void StageSelectScene::Draw() {
	// 背景描画
	drawCompBackground_.DrawAnimationScreen();

	// ボタン描画
	buttonManager_.Draw(grHandleButton_, &font_, &text_);
}

void StageSelectScene::InitializeButtons() {
	// ボタン用の白いテクスチャ
	grHandleButton_ = shared_.texWhite;

	// ボタンの位置とサイズ
	const float centerX = 640.0f;
	const float startY = 200.0f;
	const Vector2 buttonSize = { 400.0f, 80.0f };

	// ステージ1へ遷移する関数
	auto goToStage1 = [&]() {
		shared_.lastPlayedStageIndex = 0;
		manager_.RequestStage(1);
		};

	// ボタンを追加
	buttonManager_.AddButton(
		Vector2{ centerX, startY },
		buttonSize,
		"STAGE 1",
		goToStage1
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

void StageSelectScene::InitializeDrawComponents() {
	// 背景テクスチャ
	drawCompBackground_ = DrawComponent2D(
		{ kWindowWidth / 2.0f, kWindowHeight / 2.0f }, // 描画する中心座標
		1280.0f, 720.0f, // スクリーンに表示する横、縦幅
		Novice::LoadTexture("./Resources/images/stageSelect/background_ver1.png"), // グラフィックハンドル
		1280, 720,  // 画像の1フレームの横幅、縦幅
		1,		    // フレーム数
		1,			// 一行の分割数(基本フレーム数と同じでいい)
		0.3f,		// アニメーションの再生速度
		true		// ループ再生するかどうか
	);

	drawCompBackground_.StopAnimation();
	drawCompBackground_.PlayAnimation();
}

void StageSelectScene::UpdateDrawComponents(float dt) {
	drawCompBackground_.UpdateAnimation(dt);
}