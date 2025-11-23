#include "GamePlayScene.h"
#include "SceneManager.h"
#include "DebugWindow.h"
#include <Novice.h>
#include <cstdio>
#include "ParticleManager.h"

GamePlayScene::GamePlayScene(SceneManager& mgr, GameShared& shared)
	: manager_(mgr), shared_(&shared) {

	shared_->pad.Update();
	shared_->MarkExplanationViewed();

	// テクスチャをロード
	grHandleBackground_ = Novice::LoadTexture("./Resources/images/explanation/background.png");
	grHandleFrame_ = Novice::LoadTexture("./Resources/images/explanation/frame.png");

	// 初期化
	Initialize();

	float groundY = 0.0f; // 地面のY座標
	shared_->particleManager_->SetGroundLevel(groundY);

	// 雨の発生（画面上部から）
	shared_->particleManager_->Emit(ParticleType::Rain, { 640.0f, 800.0f });

	// デバッグウィンドウを作成
	debugWindow_ = std::make_unique<DebugWindow>();
}

GamePlayScene::~GamePlayScene() {
	shared_->particleManager_->StopAllContinuousEmit();
}

void GamePlayScene::Initialize() {
	fade_ = 0.0f;
	InitializeCamera();
	InitializePlayer();
	InitializeBackground();
}

void GamePlayScene::InitializeCamera() {
	bool isWorldYUp = true; // 上に行けばYが+ならtreu
	camera_ = std::make_unique<Camera2D>(Vector2{ 640.0f, 360.0f }, Vector2{ 1280.0f, 720.0f }, isWorldYUp);
	camera_->SetFollowSpeed(0.1f);
	camera_->SetDeadZone(150.0f, 100.0f);
	camera_->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
}

void GamePlayScene::InitializePlayer() {
	player_ = std::make_unique<Player>();
	player_->SetPosition({ 640.0f, 360.0f });
}

void GamePlayScene::InitializeBackground() {
	// 九マスの3x3で背景を構成
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background0_0.png")));
	background_[0]->SetPosition({ -1280.0f, 720.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background_black.png")));
	background_[1]->SetPosition({ 0.0f, 720.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background0_2.png")));
	background_[2]->SetPosition({ 1280.0f, 720.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background1_0.png")));
	background_[3]->SetPosition({ -1280.0f, 0.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background1_1.png")));
	background_[4]->SetPosition({ 0.0f, 0.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background1_2.png")));
	background_[5]->SetPosition({ 1280.0f, 0.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background2_0.png")));
	background_[6]->SetPosition({ -1280.0f, -720.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background2_1.png")));
	background_[7]->SetPosition({ 0.0f, -720.0f });
	background_.push_back(std::make_unique<Background>(Novice::LoadTexture("./Resources/images/gamePlay/background/background2_2.png")));
	background_[8]->SetPosition({ 1280.0f, -720.0f });

}

void GamePlayScene::Update(float dt, const char* keys, const char* pre) {
	shared_->pad.Update();

	if (fade_ < 1.0f) {
		fade_ += dt * 4.0f;
	}

	bool openPause =
		shared_->pad.Trigger(Pad::Button::Start) ||
		shared_->pad.Trigger(Pad::Button::Y) ||
		shared_->pad.Trigger(Pad::Button::Back) ||
		(!pre[DIK_ESCAPE] && keys[DIK_ESCAPE]) ||
		(!pre[DIK_RETURN] && keys[DIK_RETURN]);

	if (openPause) {
		shared_->PlayBackSe();
		manager_.RequestPause();
		return;
	}

#ifdef _DEBUG
	if (camera_->GetIsDebugCamera() && camera_) {
		camera_->DebugMove(true, keys, pre);
	}
	else {
		camera_->DebugMove(false, keys, pre);
	}
#endif

	if (player_) {
		player_->Update(dt, keys, pre, camera_->GetIsDebugCamera());
	}

	// パーティクルマネージャーの更新
	shared_->particleManager_->Update(dt);

	// テスト: スペースキーで爆発エフェクト
	if (keys[DIK_SPACE] && !pre[DIK_SPACE]) {
		Vector2 playerPos = player_->GetPosition();
		shared_->particleManager_->Emit(ParticleType::Explosion, playerPos);
	}

	// テスト: Jキーでデブリエフェクト
	if (keys[DIK_J] && !pre[DIK_J]) {
		Vector2 playerPos = player_->GetPosition();
		shared_->particleManager_->Emit(ParticleType::Debris, playerPos);
	}

	// テスト: Lキーでヒットエフェクト
	if (keys[DIK_L] && !pre[DIK_L]) {
		Vector2 playerPos = player_->GetPosition();
		shared_->particleManager_->Emit(ParticleType::Hit, playerPos);
	}

	// テスト: Rキーで雨ON/OFF
	if (keys[DIK_R] && !pre[DIK_R]) {
		static bool rainEnabled = true;
		rainEnabled = !rainEnabled;
		if (rainEnabled) {
			shared_->particleManager_->StartContinuousEmit(ParticleType::Rain, { 640.0f, 800.0f });
		}
		else {
			shared_->particleManager_->StopContinuousEmit(ParticleType::Rain);
		}
	}

	if (camera_) {
		camera_->Update(dt);
	}
}

void GamePlayScene::Draw() {
	// 背景を描画
	for (auto& background : background_) {
		background->Draw(*camera_);
	}

	// パーティクル描画（カメラを使用）
	shared_->particleManager_->Draw(*camera_);

	// プレイヤーを描画（カメラ使用）
	if (player_ && camera_) {
		player_->Draw(*camera_);
	}


#ifdef _DEBUG
	// デバッグウィンドウ
	shared_->particleManager_->DrawDebugWindow();
#endif

#ifdef _DEBUG
	// デバッグウィンドウを描画
	if (debugWindow_) {
		debugWindow_->DrawDebugGui();
		debugWindow_->DrawCameraDebugWindow(camera_.get());
		debugWindow_->DrawPlayerDebugWindow(player_.get());
	}
#endif
}