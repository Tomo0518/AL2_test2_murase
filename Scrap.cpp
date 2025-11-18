#include "Scrap.h"
#include <Novice.h>
#include <cmath>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

void Scrap::Initialize(ScrapType type, ScrapTrait trait, const Vector2& position, const Vector2& initialVelocity) {
	type_ = type;
	trait_ = trait;
	position_ = position;
	velocity_ = initialVelocity;
	state_ = ScrapState::Free;
	isActive_ = true;
	angle_ = 0.0f;
	orbitAngle_ = 0.0f;
	lifetimeTimer_ = 0;

	// タイプに応じた半径設定
	switch (type_) {
	case ScrapType::Small:
		radius_ = kSmallRadius;
		break;
	case ScrapType::Medium:
		radius_ = kMediumRadius;
		break;
	case ScrapType::Large:
		radius_ = kLargeRadius;
		break;
	}

	width_ = radius_ * 2.0f;
	height_ = radius_ * 2.0f;

	// 描画コンポーネントの初期化
	drawComponent_ = DrawComponent2D(
		position_, width_, height_,
		Novice::LoadTexture("./NoviceResources/white1x1.png"),
		1, 1, 1, 1, 0.0f, false
	);

	// ヒット時の破壊エフェクト用コンポーネント
	drawCompBreak_ = DrawComponent2D(
		position_, width_ * 2.0f, height_ * 2.0f,
		Novice::LoadTexture("./Resources/images/tomo/scrap_break_ver1.png"),
		64, 64, 4, 4, 0.1f, false
	);

	drawComponent_.StopAnimation();
	drawComponent_.PlayAnimation();
	drawCompBreak_.StopAnimation();
	drawCompBreak_.PlayAnimation();
}

float Scrap::GetWeight() const {
	switch (type_) {
	case ScrapType::Small:  return kSmallWeight;
	case ScrapType::Medium: return kMediumWeight;
	case ScrapType::Large:  return kLargeWeight;
	default: return kSmallWeight;
	}
}

float Scrap::GetCollisionRadius() const {
	// 状態に応じた当たり判定半径を返す
	switch (state_) {
	case ScrapState::BeingSucked:
		return radius_ * kSuckCollisionScale;
	case ScrapState::Held:
		return radius_ * kHeldCollisionScale;
	default:
		return radius_;
	}
}

void Scrap::Update(float dt) {

	switch (state_) {
	case ScrapState::Free:
		// 摩擦による減速
		velocity_ *= kFriction;
		break;

	case ScrapState::BeingSucked:
		// 吸引中は何もしない（ApplySuctionで処理）
		break;

	case ScrapState::Held:
		// 保持中は何もしない（UpdateHeldPositionで処理）
		velocity_ = { 0.0f, 0.0f };
		break;

	case ScrapState::Fired:

		// 破壊アニメーションの位置更新
		drawCompBreak_.position_ = position_;

		// 発射後は直進
		lifetimeTimer_++;
		if (lifetimeTimer_ > kFiredLifetime) {
			isActive_ = false;
		}
		break;

	case ScrapState::Hit:
		// ヒット後はアニメーション更新のみ
		drawCompBreak_.UpdateAnimation(dt, position_, angle_, scale_);
		if (drawCompBreak_.animation_->GetCurrentFrame() >= drawCompBreak_.animation_->GetTotalFrames() - 1) {
			isActive_ = false;
		}
		return; // 位置更新しない
	}

	// 位置更新
	if (state_ != ScrapState::Hit) {
		position_ += velocity_ * dt;
	}

	// 回転アニメーション
	angle_ += 2.0f * dt;

	// 描画コンポーネントの更新
	drawComponent_.position_ = position_;
	drawComponent_.angle_ = angle_;
}

void Scrap::ApplySuction(const Vector2& vaccumPos, float vaccumRadius, float dt) {
	dt;

	if (state_ != ScrapState::BeingSucked) {
		return;
	}

	Vector2 toVaccum = vaccumPos - position_;
	float distance = std::sqrt(toVaccum.x * toVaccum.x + toVaccum.y * toVaccum.y);

	// 正規化
	Vector2 direction = { toVaccum.x / distance, toVaccum.y / distance };

	// 距離に応じた速度（近いほど速い）
	float distanceRatio = 1.0f - (distance / vaccumRadius);
	distanceRatio = std::max(0.0f, std::min(1.0f, distanceRatio));

	float speed = kSuctionBaseSpeed + (kSuctionAcceleration * distanceRatio);

	switch (type_) {
	case ScrapType::Small:
		// 軽いのでそのまま1.0倍
		break;
	case ScrapType::Medium:
		speed *= kWeightMediumFriction;
		break;
	case ScrapType::Large:
		speed *= kWeightLargeFriction;
		break;
	}

	// 急激な方向転換を緩和
	Vector2 targetVelocity = { direction.x * speed, direction.y * speed };
	const float smoothFactor = 0.3f; // 0.0(即座に変更) ～ 1.0(変更なし)

	velocity_.x = velocity_.x * smoothFactor + targetVelocity.x * (1.0f - smoothFactor);
	velocity_.y = velocity_.y * smoothFactor + targetVelocity.y * (1.0f - smoothFactor);
}

void Scrap::UpdateHeldPosition(const Vector2& vaccumPos, float orbitRadius, float dt) {
	if (state_ != ScrapState::Held) {
		return;
	}

	// 公転角度を更新
	orbitAngle_ += kOrbitRotationSpeed * dt;

	// vaccumPos周辺を円軌道で回転
	position_ = {
		vaccumPos.x + std::cos(orbitAngle_) * orbitRadius,
		vaccumPos.y + std::sin(orbitAngle_) * orbitRadius
	};

	// 描画コンポーネントの位置も更新
	drawComponent_.position_ = position_;
}

void Scrap::Fire(const Vector2& direction, float speed) {
	state_ = ScrapState::Fired;
	velocity_ = { direction.x * speed, direction.y * speed };
	lifetimeTimer_ = 0;
}

int Scrap::GetDamage() const {
	// 重量ベースのダメージ計算
	float baseDamage = GetWeight() * kBaseDamagePerWeight_ * kDamageMultiplier_;

	// 整数化してクランプ
	int damage = static_cast<int>(baseDamage);
	damage = std::max(kMinDamage_, std::min(damage, kMaxDamage_));

	return damage;
}

void Scrap::Draw(const Vector2& scrollOffset) {
	if (!isActive_) {
		return;
	}

	// スクロールオフセットを考慮した描画位置
	Vector2 drawPos = position_ - scrollOffset;

	// 状態に応じた色変更（デバッグ用）
	uint32_t color = 0xFFFFFFFF;
	switch (state_) {
	case ScrapState::Free:        color = 0x0000FFFF; break;
	case ScrapState::BeingSucked: color = 0xFFFF00FF; break;
	case ScrapState::Held:        color = 0x1ed760FF; break;
	case ScrapState::Fired:       color = 0xFF0000FF; break;
	}

	// タイプに応じた色調整
	switch (type_) {
	case ScrapType::Small:  color = (color & 0xFFFFFF00) | 0xAAu; break;
	case ScrapType::Medium: color = (color & 0xFFFFFF00) | 0xCCu; break;
	case ScrapType::Large:  color = (color & 0xFFFFFF00) | 0xFFu; break;
	}

	drawComponent_.DrawAnimationScreen(color);

	if (state_ == ScrapState::Hit) {
		drawCompBreak_.DrawAnimationScreen();
	}
}