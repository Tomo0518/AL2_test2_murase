#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"

enum class ScrapType {
	Small,
	Medium,
	Large
};

enum class ScrapState {
	Free,        // フィールドに散在
	BeingSucked, // 吸い込まれ中
	Held,        // プレイヤーに保持
	Fired,        // 発射済み
	Hit,         // 衝突済み
	Idle         // 非アクティブ(配列から削除)
};

enum class ScrapTrait {
	Normal,
	Magnetic,
};

class Scrap {
public:
	Scrap() = default;
	~Scrap() = default;

	void Initialize(ScrapType type, ScrapTrait trait, const Vector2& position, const Vector2& initialVelocity = { 0.0f, 0.0f });
	void Update(float dt);
	void Draw(const Vector2& scrollOffset);

	// Getter
	ScrapType GetType() const { return type_; }
	ScrapState GetState() const { return state_; }
	Vector2 GetPosition() const { return position_; }
	Vector2 GetVelocity() const { return velocity_; }
	float GetRadius() const { return radius_; }
	float GetCollisionRadius() const; // 状態に応じた当たり判定半径
	float GetWeight() const;
	bool IsActive() const { return isActive_; }
	float GetOrbitAngle() const { return orbitAngle_; } // 保持中の角度

	// Setter
	void SetState(ScrapState state) { state_ = state; }
	void SetActive(bool active) { isActive_ = active; }
	void SetVelocity(const Vector2& vel) { velocity_ = vel; }
	void AddVelocity(const Vector2& vel) { velocity_ += vel; }
	void SetPosition(const Vector2& pos) { position_ = pos; }
	void SetOrbitAngle(float angle) { orbitAngle_ = angle; }

	// 吸引処理
	void ApplySuction(const Vector2& vaccumPos, float vaccumRadius, float dt);

	// 保持中の位置更新（vaccumPos周辺で回転）
	void UpdateHeldPosition(const Vector2& vaccumPos, float orbitRadius, float dt);

	// 発射処理
	void Fire(const Vector2& direction, float speed);

private:
	ScrapType type_ = ScrapType::Small;
	ScrapTrait trait_ = ScrapTrait::Normal;
	ScrapState state_ = ScrapState::Free;

	Vector2 scale_{ 1.0f, 1.0f };
	Vector2 position_{};
	Vector2 velocity_{};
	float angle_ = 0.0f;
	float orbitAngle_ = 0.0f; // 保持中の公転角度
	float radius_ = 0.0f;
	float width_ = 0.0f;
	float height_ = 0.0f;

	float weight_ = 0.0f; // 重量（タイプに応じて設定）

	int lifetimeTimer_ = 0;
	bool isActive_ = true;

	DrawComponent2D drawComponent_;
	DrawComponent2D drawCompBreak_;

private: // 定数
	// サイズ定数
	constexpr static float kSmallRadius = 16.0f;
	constexpr static float kMediumRadius = 24.0f;
	constexpr static float kLargeRadius = 32.0f;

	// 重量定数
	constexpr static float kSmallWeight = 1.0f;
	constexpr static float kMediumWeight = 2.0f;
	constexpr static float kLargeWeight = 3.0f;

	// 吸引される速度を重さによって変える場合の定数
	constexpr static float kWeightSmallFriction = 1.0f;
	constexpr static float kWeightMediumFriction = 0.6f;
	constexpr static float kWeightLargeFriction = 0.4f;

	// 物理定数
	constexpr static float kFriction = 0.95f;
	constexpr static float kSuckCollisionScale = 0.7f;     // 吸引中の判定サイズ
	constexpr static float kHeldCollisionScale = 0.6f;     // 保持中の判定サイズ（さらに小さく）
	constexpr static float kSuctionBaseSpeed = 200.0f;
	constexpr static float kSuctionAcceleration = 500.0f;
	constexpr static int kFiredLifetime = 180;
	constexpr static float kOrbitRotationSpeed = 2.0f;     // 保持中の回転速度


public:
	// ダメージ計算
	int GetDamage() const;

	// ダメージ倍率の設定（デバッグ用）
	static void SetDamageMultiplier(float multiplier) { kDamageMultiplier_ = multiplier; }
	static float GetDamageMultiplier() { return kDamageMultiplier_; }

private:
	// ========================================
	// ダメージ定数
	// ========================================
	static constexpr float kBaseDamagePerWeight_ = 1.0f;  // 重量1あたりの基本ダメージ
	static constexpr int kMinDamage_ = 1;                 // 最小ダメージ
	static constexpr int kMaxDamage_ = 100;               // 最大ダメージ

	static inline float kDamageMultiplier_ = 1.0f;        // ダメージ倍率（調整用）
};