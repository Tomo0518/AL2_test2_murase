#include "Particle.h"

Particle::Particle() {
	// 空の状態でコンポーネントを生成しておく
	drawComp_ = new DrawComponent2D();
}

Particle::~Particle() {
	delete drawComp_;
}

void Particle::Initialize(const Vector2& pos, const Vector2& vel, const Vector2& acc, int life,
	int texHandle, int divX, int divY, int frames, float animSpeed, bool loop) {

	// 1. パラメータのリセット
	isAlive_ = true;
	position_ = pos;
	velocity_ = vel;
	acceleration_ = acc;
	lifeTimer_ = life;
	maxLife_ = life;
	type_ = ParticleType::Physics; // デフォルト

	// 2. DrawComponentの再セットアップ
	drawComp_->Setup(texHandle, divX, divY, frames, animSpeed, loop);

	// 3. 基本的な変形情報をセット
	drawComp_->SetPosition(pos);
	drawComp_->SetScale(1.0f, 1.0f);
	drawComp_->SetRotation(0.0f);
	drawComp_->SetBaseColor(0xFFFFFFFF);
	drawComp_->SetAnchorPoint({ 0.5f, 0.5f }); // 基本は中心基準
}

void Particle::Update(float deltaTime) {
	if (!isAlive_) return;

	// 1. 寿命更新
	lifeTimer_--;
	if (lifeTimer_ <= 0) {
		isAlive_ = false;
		return;
	}

	// 2. タイプ別の挙動
	if (type_ == ParticleType::Physics) {
		// 物理移動
		velocity_.x += acceleration_.x * deltaTime;
		velocity_.y += acceleration_.y * deltaTime;
		position_.x += velocity_.x * deltaTime;
		position_.y += velocity_.y * deltaTime;
	}
	else if (type_ == ParticleType::Ghost) {
		// 残像：移動はしないが、フェードアウトなどを管理
		// (DrawCompのエフェクトで処理するのでここは空でもOK)
	}

	// 3. DrawComponentへ位置を同期
	drawComp_->SetPosition(position_);

	// 4. コンポーネント更新（アニメーション・エフェクト進行）
	drawComp_->Update(deltaTime);
}

void Particle::Draw(const Camera2D& camera) {
	if (!isAlive_) return;

	// カメラ描画を委譲
	drawComp_->Draw(camera);
}