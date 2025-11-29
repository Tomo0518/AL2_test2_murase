#pragma once
#include "GameSceneBase.h"
#include "Camera2D.h"
#include "DrawComponent2D.h"
#include <vector>
#include <memory>

/// <summary>
/// 夜空を望遠鏡で覗く演出シーン
/// </summary>
class NightSkyScene : public GameSceneBase {
public:
	NightSkyScene();
	~NightSkyScene() override = default;

	// 初期化
	void Initialize();

	// 更新
	void Update(float deltaTime, const char* keys, const char* preKeys) override;

	// 描画
	void Draw() override;

private:
	// 星構造体
	struct Star {
		std::unique_ptr<DrawComponent2D> drawComp;
		Vector2 originalPosition;
		float originalScale;
		unsigned int baseColorOriginal; // 初期色（RGB保持用）
		float fadeTime = 0.0f;          // 現在のフェードタイマー
		float fadeCycle = 3.0f;         // 透明→0.5→1→透明 の総時間
	};

	Camera2D camera_;
	std::vector<Star> stars_;
	int starTextureHandle_ = -1;

	// レンズ設定
	Vector2 lensPosition_ = { 0.0f, 0.0f };
	float lensRadius_ = 150.0f;
	float lensMagnification_ = 1.5f;

	unsigned int bgColor_ = 0x050510FF;

	// 星生成
	void SpawnStars(int count);
};