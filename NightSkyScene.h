#pragma once
#include "GameSceneBase.h"
#include "Camera2D.h"
#include "DrawComponent2D.h"
#include <vector>
#include <memory>
#include <list> // 追加・削除が多いのでlistを使います

/// <summary>
/// 夜空を望遠鏡で覗く演出シーン（流れ星・キラキラ追加版）
/// </summary>
class NightSkyScene : public GameSceneBase {
public:
	NightSkyScene();
	~NightSkyScene() override = default;

	void Initialize();
	void Update(float deltaTime, const char* keys, const char* preKeys) override;
	void Draw() override;

private:
	// --- 構造体定義 ---

	// 1. 背景の星（前回と同じ）
	struct Star {
		std::unique_ptr<DrawComponent2D> drawComp;
		Vector2 originalPosition;
		float originalScale;
	};

	// 2. 流れ星本体
	struct ShootingStar {
		std::unique_ptr<DrawComponent2D> drawComp;
		Vector2 position;
		Vector2 velocity;
		bool isActive = false;
		float spawnTimer = 0.0f; // 次にパーティクルを出すまでの時間
	};

	// 3. 軌跡のキラキラ（大量に出るので軽量化）
	struct TrailParticle {
		Vector2 position;
		Vector2 velocity;
		float scale;
		float life;      // 残り寿命 (1.0 -> 0.0)
		float decayRate; // 減衰速度
		unsigned int color;
	};

	// --- メンバ変数 ---
	Camera2D camera_;

	// リソース
	int starTextureHandle_ = -1;
	int particleTextureHandle_ = -1; // キラキラ用

	// オブジェクト管理
	std::vector<Star> stars_;
	std::list<ShootingStar> shootingStars_;
	std::list<TrailParticle> trails_;

	// 【最適化】パーティクル描画用（1つのインスタンスを使い回す）
	std::unique_ptr<DrawComponent2D> particleDrawer_;

	// 流れ星の発生管理
	float shootingStarTimer_ = 0.0f;

	// 望遠鏡（レンズ）の設定
	Vector2 lensPosition_ = { 0.0f, 0.0f };
	float lensRadius_ = 150.0f;
	float lensMagnification_ = 2.0f;
	unsigned int bgColor_ = 0x050510FF;

	// --- 内部ヘルパー関数 ---
	void SpawnStars(int count);
	void SpawnShootingStar();
	void AddTrail(const Vector2& pos);

	// レンズ描画の共通処理（テンプレート的に使えるように関数化）
	void DrawWorldElements(bool isLensEffect);
};