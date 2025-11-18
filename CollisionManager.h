#pragma once
#include "Vector2.h"
#include "Scrap.h"
#include "Boss.h"
#include "Player.h"
#include <vector>
#include <memory>
#include <functional>

// ========================================
// 衝突判定の種類
// ========================================
enum class CollisionLayer {
	Player,           // プレイヤー本体
	PlayerWeapon,     // プレイヤーの攻撃（スクラップ）
	Boss,             // ボス本体
	BossPart,         // ボスの部位
	BossWeapon,       // ボスの攻撃（弾、ビームなど）
	Neutral           // 中立（地形など）
};

// ========================================
// 衝突形状の種類
// ========================================
enum class CollisionShape {
	Circle,
	Rectangle,
	Line    // ビーム用
};

// ========================================
// 衝突判定用の汎用データ構造
// ========================================
struct Collider {
	CollisionLayer layer;
	CollisionShape shape;
	Vector2 position;

	// 形状別パラメータ
	union {
		struct { float radius; } circle;
		struct { float width; float height; float angle; } rect;
		struct { Vector2 start; Vector2 end; float thickness; } line;
	};

	void* owner = nullptr;  // 所有者オブジェクトへのポインタ
	bool isActive = true;
};

// ========================================
// 衝突イベント用コールバック
// ========================================
struct CollisionEvent {
	Collider* colliderA;
	Collider* colliderB;
	Vector2 contactPoint;    // 衝突点
	Vector2 normal;          // 衝突法線
};

// ========================================
// CollisionManager クラス
// ========================================
class CollisionManager {
public:
	CollisionManager();
	~CollisionManager();

	// ========================================
	// コライダー登録
	// ========================================

	/// <summary>
	/// 円形コライダーを登録
	/// </summary>
	Collider* RegisterCircleCollider(
		CollisionLayer layer,
		const Vector2& position,
		float radius,
		void* owner
	);

	/// <summary>
	/// 矩形コライダーを登録
	/// </summary>
	Collider* RegisterRectCollider(
		CollisionLayer layer,
		const Vector2& position,
		float width,
		float height,
		float angle,
		void* owner
	);

	/// <summary>
	/// ライン（ビーム）コライダーを登録
	/// </summary>
	Collider* RegisterLineCollider(
		CollisionLayer layer,
		const Vector2& start,
		const Vector2& end,
		float thickness,
		void* owner
	);

	/// <summary>
	/// コライダーの削除
	/// </summary>
	void UnregisterCollider(Collider* collider);

	/// <summary>
	/// 全コライダーをクリア
	/// </summary>
	void ClearAllColliders();

	// ========================================
	// 衝突判定実行
	// ========================================

	/// <summary>
	/// 全ての衝突判定を実行
	/// </summary>
	void ProcessAllCollisions();

	/// <summary>
	/// 特定レイヤー間の衝突判定
	/// </summary>
	void ProcessLayerCollision(CollisionLayer layerA, CollisionLayer layerB);

	// ========================================
	// コールバック設定
	// ========================================

	/// <summary>
	/// スクラップ → ボス本体 のヒット時
	/// </summary>
	void SetOnScrapHitBoss(std::function<void(Scrap*, Boss*, const CollisionEvent&)> callback) {
		onScrapHitBoss_ = callback;
	}

	/// <summary>
	/// スクラップ → ボス部位 のヒット時
	/// </summary>
	void SetOnScrapHitBossPart(std::function<void(Scrap*, BossParts*, const CollisionEvent&)> callback) {
		onScrapHitBossPart_ = callback;
	}

	/// <summary>
	/// ボス攻撃 → プレイヤー のヒット時
	/// </summary>
	void SetOnBossAttackHitPlayer(std::function<void(Player*, void*, const CollisionEvent&)> callback) {
		onBossAttackHitPlayer_ = callback;
	}

	/// <summary>
	/// プレイヤー → ボス本体 の接触時
	/// </summary>
	void SetOnPlayerTouchBoss(std::function<void(Player*, Boss*, const CollisionEvent&)> callback) {
		onPlayerTouchBoss_ = callback;
	}

	// ========================================
	// デバッグ描画
	// ========================================

	/// <summary>
	/// 全コライダーをデバッグ描画
	/// </summary>
	void DrawDebugColliders(const Vector2& cameraOffset);

	/// <summary>
	/// 衝突点をデバッグ描画
	/// </summary>
	void DrawDebugCollisionPoints(const Vector2& cameraOffset);

	// ========================================
	// 統計情報
	// ========================================

	int GetColliderCount() const { return static_cast<int>(colliders_.size()); }
	int GetCollisionCount() const { return collisionCountThisFrame_; }

private:
	// コライダーリスト
	std::vector<std::unique_ptr<Collider>> colliders_;

	// コールバック
	std::function<void(Scrap*, Boss*, const CollisionEvent&)> onScrapHitBoss_;
	std::function<void(Scrap*, BossParts*, const CollisionEvent&)> onScrapHitBossPart_;
	std::function<void(Player*, void*, const CollisionEvent&)> onBossAttackHitPlayer_;
	std::function<void(Player*, Boss*, const CollisionEvent&)> onPlayerTouchBoss_;

	// 今フレームの衝突回数
	int collisionCountThisFrame_ = 0;

	// 今フレームの衝突イベント（デバッグ用）
	std::vector<CollisionEvent> collisionEventsThisFrame_;

	// ========================================
	// 内部判定関数
	// ========================================
	bool CheckCollision(Collider* a, Collider* b, CollisionEvent& outEvent);
	bool CheckCircleVsCircle(Collider* a, Collider* b, CollisionEvent& outEvent);
	bool CheckCircleVsRect(Collider* a, Collider* b, CollisionEvent& outEvent);
	bool CheckCircleVsLine(Collider* a, Collider* b, CollisionEvent& outEvent);
	//bool CheckRectVsRect(Collider* a, Collider* b, CollisionEvent& outEvent);

	// レイヤーマスク（判定する/しないの設定）
	bool ShouldCheckCollision(CollisionLayer layerA, CollisionLayer layerB);
};