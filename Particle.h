#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"

/// <summary>
/// パーティクルの「挙動」の種類
/// マネージャー側で「爆発」などの種類を選んだ際、内部的にこれが設定される
/// </summary>
enum class ParticleBehavior {
	Physics,    // 物理挙動（速度・重力・回転あり：デブリなど）
	Stationary, // その場に留まる（アニメーションのみ：爆発など）
	Ghost       // 残像（動きなし、その場でフェードアウト：ダッシュ残像）
};

class Particle {
public:
	Particle();
	~Particle();

	/// <summary>
	/// パーティクルの初期化（再利用時に呼ぶ）
	/// </summary>
	void Initialize(const Vector2& pos, const Vector2& vel, const Vector2& acc, int life,
		int texHandle, int divX = 1, int divY = 1, int frames = 1, float animSpeed = 0.0f, bool loop = false);

	// 更新・描画
	void Update(float deltaTime);
	void Draw(const Camera2D& camera);

	// 状態取得
	bool IsAlive() const { return isAlive_; }

	// DrawComponentへのアクセス（エフェクト追加用）
	DrawComponent2D* GetDrawComponent() { return drawComp_; }

	// 挙動タイプ設定
	void SetBehavior(ParticleBehavior behavior) { behavior_ = behavior; }

private:
	// 描画・演出コンポーネント
	DrawComponent2D* drawComp_ = nullptr;

	// 物理・寿命パラメータ
	Vector2 position_ = { 0,0 };
	Vector2 velocity_ = { 0,0 };
	Vector2 acceleration_ = { 0,0 };

	int lifeTimer_ = 0;
	int maxLife_ = 0;
	bool isAlive_ = false;

	// デフォルトは物理挙動
	ParticleBehavior behavior_ = ParticleBehavior::Physics;
};