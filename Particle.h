#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"


class Particle {
public:
	Particle();
	~Particle();

	/// <summary>
	/// パーティクルの初期化（再利用時に呼ぶ）
	/// </summary>
	/// <param name="pos">初期位置</param>
	/// <param name="vel">速度</param>
	/// <param name="acc">加速度（重力など）</param>
	/// <param name="life">寿命（フレーム数）</param>
	/// <param name="texHandle">画像ハンドル</param>
	/// <param name="divX">横分割数（アニメなしなら1）</param>
	/// <param name="divY">縦分割数（アニメなしなら1）</param>
	/// <param name="frames">総フレーム数</param>
	/// <param name="animSpeed">アニメ速度</param>
	/// <param name="loop">ループするか</param>
	void Initialize(const Vector2& pos, const Vector2& vel, const Vector2& acc, int life,
		int texHandle, int divX = 1, int divY = 1, int frames = 1, float animSpeed = 0.0f, bool loop = false);

	// 更新・描画
	void Update(float deltaTime);
	void Draw(const Camera2D& camera);

	// 状態取得
	bool IsAlive() const { return isAlive_; }

	// DrawComponentへのアクセス（エフェクト追加用）
	DrawComponent2D* GetDrawComponent() { return drawComp_; }

	// タイプ設定
	void SetType(ParticleType type) { type_ = type; }

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

	ParticleType type_ = ParticleType::Physics;
};