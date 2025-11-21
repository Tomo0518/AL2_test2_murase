#pragma once
#include "Particle.h"
#include <array>
#include <vector>

// 前方宣言
class Camera2D;

class ParticleManager {
public:
	ParticleManager();
	~ParticleManager() = default;

	void Update(float deltaTime);
	void Draw(const Camera2D& camera);

	// === パーティクル発生用メソッド ===

	/// <summary>
	/// 汎用：物理挙動するパーティクル（デブリ、破片、火花）
	/// </summary>
	void EmitDebris(const Vector2& pos, const Vector2& vel, const Vector2& acc, int life,
		int texHandle, float startScale = 1.0f, float endScale = 0.0f, unsigned int color = 0xFFFFFFFF);

	/// <summary>
	/// アニメーションエフェクト（爆発、ヒット）
	/// </summary>
	void EmitExplosion(const Vector2& pos, float scale, int texHandle,
		int divX, int divY, int frames, float animSpeed);

	/// <summary>
	/// ダッシュ時の残像
	/// </summary>
	void EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle, int life = 20);

	// 全消去
	void Clear();

private:
	// 最大数（2048個）
	static const int kMaxParticles = 2048;
	std::array<Particle, kMaxParticles> particles_;
	int nextIndex_ = 0;

	// 次に使えるパーティクルを取得する内部関数
	Particle& GetNextParticle();
};