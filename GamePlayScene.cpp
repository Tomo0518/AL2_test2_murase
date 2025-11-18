#include "GamePlayScene.h"
#include "SceneManager.h"
#include <Novice.h>
#include <cstdio>

GamePlayScene::GamePlayScene(SceneManager& mgr, GameShared& shared)
	: manager_(mgr), shared_(shared) {
	shared_.pad.Update();
	shared_.MarkExplanationViewed();


	demoPlayer_.drawCompNormal_ = new DrawComponent2D(
		demoPlayer_.pos_, demoPlayer_.size_, demoPlayer_.size_,
		Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png"), 80, 80, 5, 5, 0.1f, true
	);
}

void GamePlayScene::Update(float dt, const char* keys, const char* pre) {
	shared_.pad.Update();

	if (fade_ < 1.0f) fade_ += dt * 4.0f;

	bool close =
		shared_.pad.Trigger(Pad::Button::Start) ||
		shared_.pad.Trigger(Pad::Button::Y) ||
		shared_.pad.Trigger(Pad::Button::Back) ||
		(pre[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE]) ||
		(pre[DIK_RETURN] == 0 && keys[DIK_RETURN]);

	if (close) {
		// 戻る音を再生
		shared_.PlayBackSe();

		manager_.RequestBackToTitle();
	}

	demoPlayer_.Update(dt, keys, pre);

}

void GamePlayScene::Draw() {

	demoPlayer_.DrawDebugWindow();

	demoPlayer_.drawCompNormal_->DrawDebugWindow("Player Demo Draw");

	// 背景を暗く
	Novice::DrawSprite(0, 0, grHandleBackground, 1.0f, 1.0f, 0.0f, 0xFFFFFFFF);

	//Novice::DrawSprite(50, 50, grHandleFrame, 1.0f, 1.0f, 0.0f, 0xFFFFFF88);

	demoPlayer_.drawCompNormal_->DrawAnimationScreen();
}