#include "GamePlayScene.h"
#include "SceneManager.h"
#include "DebugWindow.h"
#include <Novice.h>
#include <cstdio>

GamePlayScene::GamePlayScene(SceneManager& mgr, GameShared& shared)
	: manager_(mgr), shared_(shared) {

	shared_.pad.Update();
	shared_.MarkExplanationViewed();

	// テクスチャをロード
	grHandleBackground_ = Novice::LoadTexture("./Resources/images/explanation/background.png");
	grHandleFrame_ = Novice::LoadTexture("./Resources/images/explanation/frame.png");

	// 初期化
	Initialize();

	// デバッグウィンドウを作成
	debugWindow_ = std::make_unique<DebugWindow>();
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {
	fade_ = 0.0f;
	InitializeCamera();
	InitializePlayer();
	InitializeBackground();
}

void GamePlayScene::InitializeCamera() {
	camera_ = std::make_unique<Camera2D>(Vector2{ 640.0f, 360.0f }, Vector2{ 1280.0f, 720.0f });
	camera_->SetFollowSpeed(0.1f);
	camera_->SetDeadZone(150.0f, 100.0f);
	camera_->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
}

void GamePlayScene::InitializePlayer() {
	player_ = std::make_unique<Player>();
	player_->SetPosition({ 640.0f, 360.0f });
}

void GamePlayScene::InitializeBackground() {
	background_ = std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background_ver1.png"));
}

void GamePlayScene::Update(float dt, const char* keys, const char* pre) {
	shared_.pad.Update();

	// フェードイン
	if (fade_ < 1.0f) {
		fade_ += dt * 4.0f;
	}

	// シーンを閉じる
	bool openPause =
		shared_.pad.Trigger(Pad::Button::Start) ||
		shared_.pad.Trigger(Pad::Button::Y) ||
		shared_.pad.Trigger(Pad::Button::Back) ||
		(!pre[DIK_ESCAPE] && keys[DIK_ESCAPE]) ||
		(!pre[DIK_RETURN] && keys[DIK_RETURN]);

	if (openPause) {
		shared_.PlayBackSe();
		manager_.RequestPause();
		return;
	}

#ifdef _DEBUG

	// カメラデバッグモード
	if (camera_->GetIsDebugCamera() && camera_) {
		camera_->DebugMove(true, keys, pre);
	}
	else {
		camera_->DebugMove(false, keys, pre);
	}
#endif

	// プレイヤーを更新
	if (player_) {
		player_->Update(dt, keys, pre);
	}

	// カメラを更新
	if (camera_) {
		camera_->Update(dt);
	}
}

void GamePlayScene::Draw() {
	// 背景を描画
	background_->Draw(*camera_);

	// プレイヤーを描画（カメラ使用）
	if (player_ && camera_) {
		player_->Draw(*camera_);
	}

#ifdef _DEBUG
	// デバッグウィンドウを描画
	if (debugWindow_) {
		debugWindow_->DrawDebugGui();
		debugWindow_->DrawCameraDebugWindow(camera_.get());
		debugWindow_->DrawPlayerDebugWindow(player_.get());
	}
#endif
}