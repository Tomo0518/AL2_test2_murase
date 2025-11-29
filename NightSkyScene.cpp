#include "NightSkyScene.h"
#include <Novice.h>
#include <cstdlib>
#include <cmath>
#include <algorithm>

static float RandomRange(float min, float max) {
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

NightSkyScene::NightSkyScene() {
	Initialize();
}

void NightSkyScene::Initialize() {
	camera_ = Camera2D({ 640.0f, 360.0f }, { 1280.0f, 720.0f });
	starTextureHandle_ = Novice::LoadTexture("./Resources/images/effect/particle_white.png");
	SpawnStars(300);
}

void NightSkyScene::SpawnStars(int count) {
	stars_.clear();
	stars_.reserve(count);

	for (int i = 0; i < count; ++i) {
		Star star;
		float posX = RandomRange(-100.0f, 1380.0f);
		float posY = RandomRange(-100.0f, 820.0f);
		star.originalPosition = { posX, posY };
		float scale = RandomRange(0.01f, 0.03f);
		star.originalScale = scale;

		star.drawComp = std::make_unique<DrawComponent2D>(starTextureHandle_);
		star.drawComp->SetPosition(star.originalPosition);
		star.drawComp->SetScale(scale, scale);

		int colorType = rand() % 3;
		unsigned int color = 0xFFFFFFFF;
		if (colorType == 0) color = 0xAAAAFFFF;
		if (colorType == 1) color = 0xFFFFAAAA;
		// 白はそのまま
		star.baseColorOriginal = color;
		star.drawComp->SetBaseColor(color);

		// パルス（たまに）
		if (rand() % 5 == 0) {
			float duration = RandomRange(1.0f, 3.0f);
			star.drawComp->StartPulse(scale * 0.8f, scale * 1.2f, duration, true);
		}

		// フェード周期（少し個性を出す）
		star.fadeCycle = RandomRange(2.5f, 5.0f);
		star.fadeTime = RandomRange(0.0f, star.fadeCycle);

		stars_.push_back(std::move(star));
	}
}

void NightSkyScene::Update(float deltaTime, const char* keys, const char* preKeys) {
	int mx, my;
	Novice::GetMousePosition(&mx, &my);
	lensPosition_ = { static_cast<float>(mx), static_cast<float>(my) };

	camera_.Update(deltaTime);

	// フェード更新 + DrawComponent内部エフェクト更新
	for (auto& star : stars_) {
		// フェード時間進行
		star.fadeTime += deltaTime;
		if (star.fadeTime >= star.fadeCycle) {
			star.fadeTime -= star.fadeCycle;
		}

		// 3区間に分割 (0→0.5→1→0)
		float segment = star.fadeCycle / 3.0f;
		float t = star.fadeTime;
		float alpha = 1.0f;

		if (t < segment) {
			// 透明→半透明(0.5)
			float u = t / segment;
			alpha = u * 0.5f;
		}
		else if (t < segment * 2.0f) {
			// 半透明(0.5)→不透明(1.0)
			float u = (t - segment) / segment;
			alpha = 0.5f + u * 0.5f;
		}
		else {
			// 不透明(1.0)→透明(0.0)
			float u = (t - segment * 2.0f) / segment;
			alpha = 1.0f - u;
		}

		alpha = std::clamp(alpha, 0.0f, 1.0f);
		unsigned int rgb = star.baseColorOriginal & 0xFFFFFF00;
		unsigned int a = static_cast<unsigned int>(alpha * 255.0f) & 0xFF;
		unsigned int finalColor = rgb | a;
		star.drawComp->SetBaseColor(finalColor);

		star.drawComp->Update(deltaTime);
	}

	if (keys[DIK_R] && !preKeys[DIK_R]) {
		SpawnStars(300);
	}

	if (keys[DIK_UP]) lensMagnification_ += 0.05f;
	if (keys[DIK_DOWN]) lensMagnification_ = (std::max)(0.2f, lensMagnification_ - 0.05f);
}

void NightSkyScene::Draw() {
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, bgColor_, kFillModeSolid);

	float r2 = lensRadius_ * lensRadius_;

	// 1. 通常レイヤー（レンズ内は描画しない）
	for (auto& star : stars_) {
		// レンズ内ならスキップ
		Vector2 diff = {
			star.originalPosition.x - lensPosition_.x,
			star.originalPosition.y - lensPosition_.y
		};
		float distSq = diff.x * diff.x + diff.y * diff.y;
		if (distSq < r2) {
			continue; // レンズ領域は拡大版のみ描画する
		}

		star.drawComp->SetPosition(star.originalPosition);
		star.drawComp->SetScale(star.originalScale, star.originalScale);
		star.drawComp->Draw(camera_);
	}

	// 2. レンズ内の拡大描画
	for (auto& star : stars_) {
		Vector2 diff = {
			star.originalPosition.x - lensPosition_.x,
			star.originalPosition.y - lensPosition_.y
		};
		float distSq = diff.x * diff.x + diff.y * diff.y;
		if (distSq >= r2) {
			continue; // レンズ外は不要
		}

		Vector2 zoomedPos = {
			lensPosition_.x + diff.x * lensMagnification_,
			lensPosition_.y + diff.y * lensMagnification_
		};

		float zoomedDistSq =
			(zoomedPos.x - lensPosition_.x) * (zoomedPos.x - lensPosition_.x) +
			(zoomedPos.y - lensPosition_.y) * (zoomedPos.y - lensPosition_.y);

		if (zoomedDistSq < r2) {
			star.drawComp->SetPosition(zoomedPos);
			star.drawComp->SetScale(star.originalScale * lensMagnification_, star.originalScale * lensMagnification_);
			star.drawComp->Draw(camera_);
			star.drawComp->SetPosition(star.originalPosition);
			star.drawComp->SetScale(star.originalScale, star.originalScale);
		}
	}

	// 3. レンズ枠
	Novice::DrawEllipse(
		static_cast<int>(lensPosition_.x),
		static_cast<int>(lensPosition_.y),
		static_cast<int>(lensRadius_),
		static_cast<int>(lensRadius_),
		0.0f,
		0xFFFFFFAF,
		kFillModeWireFrame
	);
	Novice::DrawEllipse(
		static_cast<int>(lensPosition_.x),
		static_cast<int>(lensPosition_.y),
		static_cast<int>(lensRadius_ + 1.0f),
		static_cast<int>(lensRadius_ + 1.0f),
		0.0f,
		0xFFFFFF55,
		kFillModeWireFrame
	);

	Novice::DrawEllipse(
		static_cast<int>(lensPosition_.x),
		static_cast<int>(lensPosition_.y),
		static_cast<int>(lensRadius_ + 2.0f),
		static_cast<int>(lensRadius_ + 2.0f),
		0.0f,
		0xFFFFFF55,
		kFillModeWireFrame
	);

	Novice::DrawEllipse(
		static_cast<int>(lensPosition_.x),
		static_cast<int>(lensPosition_.y),
		static_cast<int>(lensRadius_ + 3.0f),
		static_cast<int>(lensRadius_ + 3.0f),
		0.0f,
		0xFFFFFF55,
		kFillModeWireFrame
	);

	Novice::DrawEllipse(
		static_cast<int>(lensPosition_.x),
		static_cast<int>(lensPosition_.y),
		static_cast<int>(lensRadius_ + 4.0f),
		static_cast<int>(lensRadius_ + 4.0f),
		0.0f,
		0xFFFFFF55,
		kFillModeWireFrame
	);
}