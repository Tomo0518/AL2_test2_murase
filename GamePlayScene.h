#pragma once
#include "GameSceneBase.h"
#include "GameShared.h"
#include "SceneType.h"
#include "Vector2.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

class SceneManager;
class DrawComponent2D;

class DemoPlayer {
public:
	Vector2 pos_{};
	Vector2 velocity_{};
	float   radius_ = 32.0f;
	float size_ = 64.0f; // 当たり判定一辺のサイズ(正方形)
	float angle_ = 0.0f;
	Vector2 scale_{ 1.0f, 1.0f };
	bool isAlive_ = true;
	int grHandle_ = -1; // 描画用テクスチャ
	DrawComponent2D* drawCompNormal_;

	void Move(float deltaTime, const char* keys) {

		velocity_ = { 0.0f, 0.0f };

		if (keys[DIK_W]) {
			velocity_.y = -200.0f;
		}
		else if (keys[DIK_S]) {
			velocity_.y = 200.0f;
		}

		if (keys[DIK_A]) {
			velocity_.x = -200.0f;
		}
		else if (keys[DIK_D]) {
			velocity_.x = 200.0f;
		}

		pos_ += velocity_ * deltaTime;
	}

	void Update(float deltaTime, const char* keys, const char* pre) {

		// シェイクエフェクト開始
		if (keys[DIK_Q] && !pre[DIK_Q]) {
			drawCompNormal_->StartShake(7.0f, 2.0f);
		}

		// 回転エフェクト開始
		if (keys[DIK_R] && !pre[DIK_R]) {
			drawCompNormal_->StartRotation(3.0f, 5.0f);
		}

		// 潰しと伸ばしエフェクト開始
		if (keys[DIK_E] && !pre[DIK_E]) {
			drawCompNormal_->StartSquashStretch({ 1.5f, 0.5f }, 0.3f, 6);
		}

		// フェードエフェクト開始
		if (keys[DIK_T] && !pre[DIK_T]) {
			drawCompNormal_->StartFade(2.0f);
		}

		if (keys[DIK_Y] && !pre[DIK_Y]) {
			drawCompNormal_->StartFadeToColor(0xFF0000FF, 2.0f);
		}

		// 拡大縮小エフェクト開始
		if (keys[DIK_U] && !pre[DIK_U]) {
			drawCompNormal_->StartScale(0.5f, 1.5f, 4.2f, 6);
		}

		// エフェクトをリセット
		if (keys[DIK_F]) {
			drawCompNormal_->ResetAllEffects();
		}

		Move(deltaTime, keys);

		drawCompNormal_->UpdateAnimation(deltaTime, pos_, angle_, scale_);

		drawCompNormal_->UpdateEffects(deltaTime);
	}

	void Draw() {
		drawCompNormal_->DrawAnimationScreen(pos_);


	}

	void DrawDebugWindow() {
#ifdef _DEBUG
		ImGui::Begin("Demo Player Debug");
		ImGui::Text("Position: (%.2f, %.2f)", pos_.x, pos_.y);
		ImGui::Text("Velocity: (%.2f, %.2f)", velocity_.x, velocity_.y);
		ImGui::Text("Angle: %.2f", angle_);
		ImGui::Text("Scale: (%.2f, %.2f)", scale_.x, scale_.y);
		ImGui::End();
#endif // DEBUG
	}
};

class GamePlayScene : public GameSceneBase {
public:
	GamePlayScene(SceneManager& mgr, GameShared& shared);
	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

	int grHandleBackground = Novice::LoadTexture("./Resources/images/explanation/background.png");
	int grHandleFrame = Novice::LoadTexture("./Resources/images/explanation/frame.png");

	DemoPlayer demoPlayer_;

private:
	SceneManager& manager_;
	GameShared& shared_;
	float fade_ = 0.0f;
};