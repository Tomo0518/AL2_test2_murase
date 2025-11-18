#include "CollisionManager.h"
#include <Novice.h>
#include <cmath>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

CollisionManager::CollisionManager() {
	colliders_.reserve(100);  // 事前確保でパフォーマンス向上
}

CollisionManager::~CollisionManager() {
	ClearAllColliders();
}

// ========================================
// コライダー登録
// ========================================
Collider* CollisionManager::RegisterCircleCollider(
	CollisionLayer layer,
	const Vector2& position,
	float radius,
	void* owner
) {
	auto collider = std::make_unique<Collider>();
	collider->layer = layer;
	collider->shape = CollisionShape::Circle;
	collider->position = position;
	collider->circle.radius = radius;
	collider->owner = owner;
	collider->isActive = true;

	Collider* ptr = collider.get();
	colliders_.push_back(std::move(collider));
	return ptr;
}

Collider* CollisionManager::RegisterRectCollider(
	CollisionLayer layer,
	const Vector2& position,
	float width,
	float height,
	float angle,
	void* owner
) {
	auto collider = std::make_unique<Collider>();
	collider->layer = layer;
	collider->shape = CollisionShape::Rectangle;
	collider->position = position;
	collider->rect.width = width;
	collider->rect.height = height;
	collider->rect.angle = angle;
	collider->owner = owner;
	collider->isActive = true;

	Collider* ptr = collider.get();
	colliders_.push_back(std::move(collider));
	return ptr;
}

Collider* CollisionManager::RegisterLineCollider(
	CollisionLayer layer,
	const Vector2& start,
	const Vector2& end,
	float thickness,
	void* owner
) {
	auto collider = std::make_unique<Collider>();
	collider->layer = layer;
	collider->shape = CollisionShape::Line;
	collider->line.start = start;
	collider->line.end = end;
	collider->line.thickness = thickness;
	collider->owner = owner;
	collider->isActive = true;

	Collider* ptr = collider.get();
	colliders_.push_back(std::move(collider));
	return ptr;
}

void CollisionManager::UnregisterCollider(Collider* collider) {
	auto it = std::find_if(colliders_.begin(), colliders_.end(),
		[collider](const std::unique_ptr<Collider>& c) { return c.get() == collider; });

	if (it != colliders_.end()) {
		colliders_.erase(it);
	}
}

void CollisionManager::ClearAllColliders() {
	colliders_.clear();
	collisionCountThisFrame_ = 0;
	collisionEventsThisFrame_.clear();
}

// ========================================
// 衝突判定実行
// ========================================

void CollisionManager::ProcessAllCollisions() {
	collisionCountThisFrame_ = 0;
	collisionEventsThisFrame_.clear();

	// 全ての組み合わせをチェック
	for (size_t i = 0; i < colliders_.size(); ++i) {
		for (size_t j = i + 1; j < colliders_.size(); ++j) {
			Collider* a = colliders_[i].get();
			Collider* b = colliders_[j].get();

			if (!a->isActive || !b->isActive) continue;
			if (!ShouldCheckCollision(a->layer, b->layer)) continue;

			CollisionEvent event;
			if (CheckCollision(a, b, event)) {
				collisionCountThisFrame_++;
				collisionEventsThisFrame_.push_back(event);

				// コールバック呼び出し
				if (a->layer == CollisionLayer::PlayerWeapon && b->layer == CollisionLayer::Boss) {
					if (onScrapHitBoss_) {
						onScrapHitBoss_(
							static_cast<Scrap*>(a->owner),
							static_cast<Boss*>(b->owner),
							event
						);
					}
				}
				else if (a->layer == CollisionLayer::PlayerWeapon && b->layer == CollisionLayer::BossPart) {
					if (onScrapHitBossPart_) {
						onScrapHitBossPart_(
							static_cast<Scrap*>(a->owner),
							static_cast<BossParts*>(b->owner),
							event
						);
					}
				}
				else if (a->layer == CollisionLayer::BossWeapon && b->layer == CollisionLayer::Player) {
					if (onBossAttackHitPlayer_) {
						onBossAttackHitPlayer_(
							static_cast<Player*>(b->owner),
							a->owner,
							event
						);
					}
				}
				else if (a->layer == CollisionLayer::Player && b->layer == CollisionLayer::Boss) {
					if (onPlayerTouchBoss_) {
						onPlayerTouchBoss_(
							static_cast<Player*>(a->owner),
							static_cast<Boss*>(b->owner),
							event
						);
					}
				}
			}
		}
	}
}

bool CollisionManager::CheckCollision(Collider* a, Collider* b, CollisionEvent& outEvent) {
	outEvent.colliderA = a;
	outEvent.colliderB = b;

	// 形状の組み合わせに応じて判定
	if (a->shape == CollisionShape::Circle && b->shape == CollisionShape::Circle) {
		return CheckCircleVsCircle(a, b, outEvent);
	}
	else if (a->shape == CollisionShape::Circle && b->shape == CollisionShape::Rectangle) {
		return CheckCircleVsRect(a, b, outEvent);
	}
	else if (a->shape == CollisionShape::Rectangle && b->shape == CollisionShape::Circle) {
		return CheckCircleVsRect(b, a, outEvent);
	}
	else if (a->shape == CollisionShape::Circle && b->shape == CollisionShape::Line) {
		return CheckCircleVsLine(a, b, outEvent);
	}
	else if (a->shape == CollisionShape::Line && b->shape == CollisionShape::Circle) {
		return CheckCircleVsLine(b, a, outEvent);
	}

	return false;
}

bool CollisionManager::CheckCircleVsCircle(Collider* a, Collider* b, CollisionEvent& outEvent) {
	float dx = b->position.x - a->position.x;
	float dy = b->position.y - a->position.y;
	float distance = std::sqrt(dx * dx + dy * dy);
	float radiusSum = a->circle.radius + b->circle.radius;

	if (distance < radiusSum) {
		// 衝突点と法線を計算
		outEvent.contactPoint = {
			a->position.x + (dx / distance) * a->circle.radius,
			a->position.y + (dy / distance) * a->circle.radius
		};
		outEvent.normal = { dx / distance, dy / distance };
		return true;
	}

	return false;
}

bool CollisionManager::CheckCircleVsRect(Collider* circle, Collider* rect, CollisionEvent& outEvent) {
	// 簡易実装：矩形を円として扱う（正確な矩形判定は複雑なので省略）
	float rectRadius = std::max(rect->rect.width, rect->rect.height) * 0.5f;
	float dx = rect->position.x - circle->position.x;
	float dy = rect->position.y - circle->position.y;
	float distance = std::sqrt(dx * dx + dy * dy);
	float radiusSum = circle->circle.radius + rectRadius;

	if (distance < radiusSum) {
		outEvent.contactPoint = circle->position;
		outEvent.normal = { dx / distance, dy / distance };
		return true;
	}

	return false;
}

bool CollisionManager::CheckCircleVsLine(Collider* circle, Collider* line, CollisionEvent& outEvent) {
	// 点と線分の最短距離を計算
	Vector2 lineVec = {
		line->line.end.x - line->line.start.x,
		line->line.end.y - line->line.start.y
	};
	Vector2 circleVec = {
		circle->position.x - line->line.start.x,
		circle->position.y - line->line.start.y
	};

	float lineLenSq = lineVec.x * lineVec.x + lineVec.y * lineVec.y;
	float t = (circleVec.x * lineVec.x + circleVec.y * lineVec.y) / lineLenSq;
	t = std::max(0.0f, std::min(1.0f, t));

	Vector2 closestPoint = {
		line->line.start.x + lineVec.x * t,
		line->line.start.y + lineVec.y * t
	};

	float dx = circle->position.x - closestPoint.x;
	float dy = circle->position.y - closestPoint.y;
	float distance = std::sqrt(dx * dx + dy * dy);

	if (distance < circle->circle.radius + line->line.thickness) {
		outEvent.contactPoint = closestPoint;
		outEvent.normal = { dx / distance, dy / distance };
		return true;
	}

	return false;
}

bool CollisionManager::ShouldCheckCollision(CollisionLayer layerA, CollisionLayer layerB) {
	// レイヤーマスクテーブル
	// Player vs BossWeapon, PlayerWeapon vs Boss, など判定すべき組み合わせのみtrueに
	if (layerA == CollisionLayer::PlayerWeapon && layerB == CollisionLayer::Boss) return true;
	if (layerA == CollisionLayer::PlayerWeapon && layerB == CollisionLayer::BossPart) return true;
	if (layerA == CollisionLayer::BossWeapon && layerB == CollisionLayer::Player) return true;
	if (layerA == CollisionLayer::Player && layerB == CollisionLayer::Boss) return true;

	// 逆順もチェック
	if (layerB == CollisionLayer::PlayerWeapon && layerA == CollisionLayer::Boss) return true;
	if (layerB == CollisionLayer::PlayerWeapon && layerA == CollisionLayer::BossPart) return true;
	if (layerB == CollisionLayer::BossWeapon && layerA == CollisionLayer::Player) return true;
	if (layerB == CollisionLayer::Player && layerA == CollisionLayer::Boss) return true;

	return false;
}

// ========================================
// デバッグ描画
// ========================================

void CollisionManager::DrawDebugColliders(const Vector2& cameraOffset) {
	for (const auto& collider : colliders_) {
		if (!collider->isActive) continue;

		unsigned int color = 0x00FF00FF;  // 緑

		switch (collider->shape) {
		case CollisionShape::Circle:
			/*Novice::DrawEllipse(
			//	static_cast<int>(collider->position.x - cameraOffset.x),
			//	static_cast<int>(collider->position.y - cameraOffset.y),
			//	static_cast<int>(collider->circle.radius),
			//	static_cast<int>(collider->circle.radius),
			//	0.0f, color, kFillModeWireFrame
			//);*/
			break;

		case CollisionShape::Rectangle:
			Novice::DrawBox(
				static_cast<int>(collider->position.x - collider->rect.width * 0.5f - cameraOffset.x),
				static_cast<int>(collider->position.y - collider->rect.height * 0.5f - cameraOffset.y),
				static_cast<int>(collider->rect.width),
				static_cast<int>(collider->rect.height),
				0.0f, color, kFillModeWireFrame
			);
			break;

		case CollisionShape::Line:
			Novice::DrawLine(
				static_cast<int>(collider->line.start.x - cameraOffset.x),
				static_cast<int>(collider->line.start.y - cameraOffset.y),
				static_cast<int>(collider->line.end.x - cameraOffset.x),
				static_cast<int>(collider->line.end.y - cameraOffset.y),
				color
			);
			break;
		}
	}
}

void CollisionManager::DrawDebugCollisionPoints(const Vector2& cameraOffset) {
	for (const auto& event : collisionEventsThisFrame_) {
		Novice::DrawEllipse(
			static_cast<int>(event.contactPoint.x - cameraOffset.x),
			static_cast<int>(event.contactPoint.y - cameraOffset.y),
			5, 5, 0.0f, 0xFF0000FF, kFillModeSolid
		);
	}
}