#include "ResultScene.h"
#include <Novice.h>
#include "SceneManager.h"

ResultScene::ResultScene(SceneManager& mgr, GameShared& shared)
	: manager_(mgr), shared_(shared) {

	if (font_.Load("Resources/font/oxanium.fnt", "./Resources/font/oxanium_0.png")) {
		text_.SetFont(&font_);
		fontReady_ = true;
	}

	InitializeButtons();

	drawCompBgDraw_ = new DrawComponent2D(
		{ 400.0f, 400.0f }, 800.0f, 800.0f,
		Novice::LoadTexture("./Resources/images/result/result_sky.png"),
		800, 800, 4, 4, 0.13f, true
	);

	drawCompClearLabel_ = new DrawComponent2D(
		{ 400, 360.0f }, 397.0f, 251.0f,
		Novice::LoadTexture("./Resources/images/result/clear.png"),
		397, 251, 1, 1, 0.0f, false
	);

	drawCompClearLabel_->StartScaleContinuous(0.95f, 1.05f, 0.2f);

	shared_.PlayExclusive_(BgmKind::Result);
}

void ResultScene::InitializeButtons() {
	grHandleButton_ = shared_.texWhite;

	const float centerX = 1080.0f;
	const float startY = 500.0f;
	const float buttonSpacing = 80.0f;
	const Vector2 buttonSize = { 270.0f, 60.0f };

	auto retry = [&]() {
		shared_.StopAllBgm();
		manager_.RequestStageRestart();
		};

	auto backToTitle = [&]() {
		shared_.StopAllBgm();
		manager_.RequestBackToTitle();
		};

	auto quit = [&]() {
		shared_.StopAllBgm();
		manager_.RequestQuit();
		};

	buttonManager_.AddButton({ centerX, startY }, buttonSize, "RETRY", retry);
	buttonManager_.AddButton({ centerX, startY + buttonSpacing }, buttonSize, "TITLE", backToTitle);
	buttonManager_.AddButton({ centerX, startY + buttonSpacing * 2 }, buttonSize, "QUIT", quit);

	buttonManager_.SetOnSelectSound([&]() {
		shared_.PlaySelectSe();
		});

	buttonManager_.SetOnDecideSound([&]() {
		shared_.PlayDecideSe();
		});
}

void ResultScene::Update(float dt, const char* keys, const char* pre) {
	shared_.pad.Update();

	drawCompBgDraw_->UpdateAnimation(dt);
	drawCompClearLabel_->UpdateEffects(dt);

	buttonManager_.Update(dt, keys, pre, shared_.pad);
}

void ResultScene::Draw() {
	drawCompBgDraw_->DrawAnimationScreen();
	drawCompClearLabel_->DrawAnimationScreen();

	if (fontReady_) {
		buttonManager_.Draw(grHandleButton_, &font_, &text_);
	}
}