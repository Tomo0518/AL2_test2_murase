#include "BaseStageScene.h"
#include "SceneManager.h"
#include "WindowSize.h"
#include <Novice.h>
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif

BaseStageScene::BaseStageScene(SceneManager& manager, GameShared& shared, int stageIndex)
	: manager_(manager), shared_(shared), stageIndex_(stageIndex) {

	drawCompBackground_ = DrawComponent2D(
		{ kWindowWidth / 2.0f, kWindowHeight / 2.0f }, // 描画する中心座標
		1280.0f, 720.0f, // スクリーンに表示する横、縦幅
		Novice::LoadTexture("./Resources/images/gamePlay/background_ver1.png"), // グラフィックハンドル
		1280, 720,  // 画像の1フレームの横幅、縦幅
		1,		    // フレーム数
		1,			// 一行の分割数(基本フレーム数と同じでいい)
		0.3f,		// アニメーションの再生速度
		true		// ループ再生するかどうか
	);

	drawCompBackground_.StopAnimation();
	drawCompBackground_.PlayAnimation();

	// BGM再生
	shared_.PlayExclusive_(BgmKind::Stage);
}

void BaseStageScene::Update(float dt, const char* keys, const char* pre) {
	// 初回のみ初期化
	if (!initialized_) {
		InitializeStage();
		initialized_ = true;
	}

	// ========================================
	// シーン遷移入力（共通）
	// ========================================

	// ESCキーでポーズ
	if (keys[DIK_ESCAPE] && !pre[DIK_ESCAPE]) {
		manager_.RequestPause();
		return;
	}

	// パッドのStartボタンでポーズ
	shared_.pad.Update();
	if (shared_.pad.Trigger(Pad::Button::Start)) {
		manager_.RequestPause();
		return;
	}

	// ========================================
	// ステージ固有の更新処理
	// ========================================
	UpdateStage(dt, keys, pre);

#ifdef _DEBUG
	// デバッグ用：ステージセレクトに戻る
	if (keys[DIK_F1] && !pre[DIK_F1]) {
		manager_.RequestStageSelect();
	}

	// デバッグ用：リスタート
	if (keys[DIK_F5] && !pre[DIK_F5]) {
		initialized_ = false; // 再初期化フラグ
	}
#endif

	drawCompBackground_.UpdateAnimation(dt);
}

void BaseStageScene::Draw() {
	if (!initialized_) {
		return;
	}

	drawCompBackground_.DrawAnimationScreen();

	// ========================================
	// ステージ固有の描画処理
	// ========================================
	DrawStage();

}
