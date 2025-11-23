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

// 前方宣言
class Camera2D;

// 1種類のエフェクトの設定データ（拡張版）
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
	Vector2 acceleration = { 0.0f, 0.0f };
	Vector2 emitRange = { 0.0f, 0.0f };
	float sizeMin = 16.0f;
	float sizeMax = 32.0f;
	float scaleStart = 1.0f;
	float scaleEnd = 0.0f;
	unsigned int colorStart = 0xFFFFFFFF;
	unsigned int colorEnd = 0xFFFFFF00;
	float rotationSpeedMin = 0.0f;
	float rotationSpeedMax = 0.0f;
	bool useAnimation = false;
	int divX = 1;
	int divY = 1;
	int totalFrames = 1;
	float animSpeed = 0.1f;
	BlendMode blendMode = kBlendModeNormal;

	// Emitter Shape
	EmitterShape emitterShape = EmitterShape::Point;
	Vector2 emitterSize = { 0.0f, 0.0f };

	// Homing（追従）
	bool useHoming = false;
	float homingStrength = 0.0f;

	// 連続発生設定
	bool isContinuous = false;         // 連続発生するか
	float emitInterval = 0.0f;         // 発生間隔（秒）
};

class ParticleManager {
public:
	ParticleManager();
	~ParticleManager() = default;

	void Update(float deltaTime);
	void Draw(const Camera2D& camera);
	void Emit(ParticleType type, const Vector2& pos);
	void EmitWithTarget(ParticleType type, const Vector2& pos, const Vector2* target);
	void EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle);
	void DrawDebugWindow();
	void Clear();
	void LoadCommonResources();

	// 連続発生の開始/停止
	void StartContinuousEmit(ParticleType type, const Vector2& pos);
	void StartContinuousEmitWithTarget(ParticleType type, const Vector2& pos, const Vector2* target);
	void StopContinuousEmit(ParticleType type);
	void StopAllContinuousEmit();

	// 地面との衝突判定を設定
	void SetGroundLevel(float groundY);
	float GetGroundLevel() const { return groundLevel_; }

	// JSON保存/読み込み
	bool SaveParamsToJson(const std::string& filepath);
	bool LoadParamsFromJson(const std::string& filepath);

private:
	void LoadParams();
	Particle& GetNextParticle();
	float RandomFloat(float min, float max);
	Vector2 GenerateEmitPosition(const Vector2& basePos, const ParticleParam& param);

	// JSONシリアライズ用ヘルパー
	nlohmann::json SerializeParams() const;
	bool DeserializeParams(const nlohmann::json& j);

	// ブレンドモード変換ヘルパー
	static const char* BlendModeToString(BlendMode mode);
	static BlendMode StringToBlendMode(const std::string& str);

	// 連続発生の管理構造体
	struct ContinuousEmitter {
		ParticleType type;
		Vector2 position;
		const Vector2* target = nullptr;
		float timer = 0.0f;
		bool isActive = false;
	};

	static const int kMaxParticles = 2048;
	std::array<Particle, kMaxParticles> particles_;
	int nextIndex_ = 0;

	std::map<ParticleType, ParticleParam> params_;
	std::map<ParticleType, ContinuousEmitter> continuousEmitters_;  // ★追加

	float groundLevel_ = 0.0f;  // ★追加：地面のY座標

	int texExplosion_ = -1;
	int texDebris_ = -1;
	int texHit_ = -1;
	int texDust_ = -1;
	int texRain_ = -1;
	int texSnow_ = -1;
	int texOrb_ = -1;

	int texGlow_ = -1;
	int texRing_ = -1;
	int texSparkle_ = -1;
	int texScratch_ = -1;
	int texSmoke_ = -1;

	ParticleType currentDebugType_ = ParticleType::Explosion;
};