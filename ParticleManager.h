#pragma once
#include "Particle.h"
#include "Vector2.h"
#include "Novice.h"
#include <array>
#include <vector>
#include <map>
#include <string>
#include "json.hpp"
#include "ParticleEnum.h"


// 1種類のエフェクトの設定データ
struct ParticleParam {
	int count = 1;
	int textureHandle = -1;
	int lifeMin = 30;
	int lifeMax = 60;
	float speedMin = 100.0f;
	float speedMax = 200.0f;
	float angleBase = 0.0f;
	float angleRange = 360.0f;
	Vector2 gravity = { 0.0f, 0.0f };
	float sizeMin = 16.0f;
	float sizeMax = 32.0f;
	float scaleStart = 1.0f;
	float scaleEnd = 0.0f;
	unsigned int colorStart = 0xFFFFFFFF;
	unsigned int colorEnd = 0xFFFFFF00;
	bool useAnimation = false;
	int divX = 1;
	int divY = 1;
	int totalFrames = 1;
	float animSpeed = 0.1f;
	BlendMode blendMode = kBlendModeNormal;  // ブレンドモード追加
};

class ParticleManager {
public:
	ParticleManager();
	~ParticleManager() = default;

	void Update(float deltaTime);
	void Draw(const Camera2D& camera);
	void Emit(ParticleType type, const Vector2& pos);
	void EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle);
	void DrawDebugWindow();
	void Clear();
	void LoadCommonResources();

	// ========== JSON保存/読み込み ==========
	bool SaveParamsToJson(const std::string& filepath);
	bool LoadParamsFromJson(const std::string& filepath);

private:
	void LoadParams();
	Particle& GetNextParticle();
	float RandomFloat(float min, float max);

	// JSONシリアライズ用ヘルパー
	nlohmann::json SerializeParams() const;
	bool DeserializeParams(const nlohmann::json& j);

	// ブレンドモード変換ヘルパー
	static const char* BlendModeToString(BlendMode mode);
	static BlendMode StringToBlendMode(const std::string& str);

	static const int kMaxParticles = 2048;
	std::array<Particle, kMaxParticles> particles_;
	int nextIndex_ = 0;

	std::map<ParticleType, ParticleParam> params_;

	int texExplosion_ = -1;
	int texDebris_ = -1;
	int texHit_ = -1;
	int texDust_ = -1;

	ParticleType currentDebugType_ = ParticleType::Explosion;
};