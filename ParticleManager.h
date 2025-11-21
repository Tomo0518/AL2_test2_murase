#pragma once
#include "Particle.h"
#include "Vector2.h"
#include <array>
#include <vector>
#include <map>
#include <string>

// エフェクトの種類
enum class ParticleType {
	Explosion,   // 爆発
	Debris,      // 岩の破片
	Hit,         // 打撃エフェクト
	Dust,        // 土煙
	MuzzleFlash, // マズルフラッシュ
	// 特殊なものは専用関数でやるので、ここには汎用的なものを入れる
};

// 1種類のエフェクトの設定データ（ImGuiで調整する対象）
struct ParticleParam {
	// --- 基本設定 ---
	int count = 1;           // 1回のEmitで出る数
	int textureHandle = -1;  // テクスチャ

	// --- 寿命（フレーム数） ---
	int lifeMin = 30;
	int lifeMax = 60;

	// --- 物理挙動 ---
	float speedMin = 100.0f;
	float speedMax = 200.0f;
	float angleBase = 0.0f;
	float angleRange = 360.0f;
	Vector2 gravity = { 0.0f, 0.0f };

	// --- 見た目（サイズ） ---
	float sizeMin = 16.0f;    // 最小サイズ（ピクセル）
	float sizeMax = 32.0f;    // 最大サイズ（ピクセル）

	// --- 見た目（スケール） ---
	float scaleStart = 1.0f;
	float scaleEnd = 0.0f;

	// --- 見た目（色） ---
	unsigned int colorStart = 0xFFFFFFFF;
	unsigned int colorEnd = 0xFFFFFF00;

	// --- アニメーション ---
	bool useAnimation = false;
	int divX = 1;
	int divY = 1;
	int totalFrames = 1;
	float animSpeed = 0.1f;
};

class ParticleManager {
public:
	ParticleManager();
	~ParticleManager() = default;

	// 更新（dtを受け取ることでスローモーションに対応）
	void Update(float deltaTime);

	// 描画
	void Draw(const Camera2D& camera);

	// === メインの発生関数 ===
	// 登録されたパラメータに基づいてエフェクトを発生させる
	void Emit(ParticleType type, const Vector2& pos);

	// === 特殊演出用（残像などパラメータ化しにくいもの） ===
	void EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle);

	// === デバッグ調整用 ===
	// ImGuiでパラメータをリアルタイムに書き換える
	void DrawDebugWindow();

	// 全消去
	void Clear();

	// リソース読み込み（GameSharedから呼ぶ）
	void LoadCommonResources();

private:
	// 初期設定（デフォルト値をロード）
	void LoadParams();

	// 次に使えるパーティクルを取得
	Particle& GetNextParticle();

	// 乱数ヘルパー
	float RandomFloat(float min, float max);

private:
	// パーティクルプール
	static const int kMaxParticles = 2048;
	std::array<Particle, kMaxParticles> particles_;
	int nextIndex_ = 0;

	// パラメータ辞書
	std::map<ParticleType, ParticleParam> params_;

	// 汎用エフェクト用テクスチャハンドル
	int texExplosion_ = -1;
	int texDebris_ = -1;
	int texHit_ = -1;
	int texDust_ = -1;

	// デバッグ用：現在選択中のエフェクトタイプ
	ParticleType currentDebugType_ = ParticleType::Explosion;
};