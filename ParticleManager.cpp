#include "ParticleManager.h"

ParticleManager::ParticleManager() {
	// 配列確保済みなので特になし
}

void ParticleManager::Update(float deltaTime) {
	for (auto& p : particles_) {
		if (p.IsAlive()) {
			p.Update(deltaTime);
		}
	}
}

void ParticleManager::Draw(const Camera2D& camera) {
	// まとめて描画
	// ※必要なら加算合成への切り替えをここで行う
	// Novice::SetBlendMode(kBlendModeAdd); 

	for (auto& p : particles_) {
		if (p.IsAlive()) {
			p.Draw(camera);
		}
	}

	// Novice::SetBlendMode(kBlendModeNormal);
}

Particle& ParticleManager::GetNextParticle() {
	// リングバッファ
	Particle& p = particles_[nextIndex_];
	nextIndex_ = (nextIndex_ + 1) % kMaxParticles;
	return p;
}

// ========== 各種Emit実装 ==========

void ParticleManager::EmitDebris(const Vector2& pos, const Vector2& vel, const Vector2& acc, int life,
	int texHandle, float startScale, float endScale, unsigned int color) {

	Particle& p = GetNextParticle();

	// 1. 基本初期化（アニメなし）
	p.Initialize(pos, vel, acc, life, texHandle);
	p.SetType(ParticleType::Physics);

	// 2. 演出設定（DrawComponentの機能を活用）
	auto* drawComp = p.GetDrawComponent();

	drawComp->SetBaseColor(color);
	drawComp->SetScale(startScale, startScale);

	// 回転させる（物理挙動っぽく）
	drawComp->StartRotationContinuous(5.0f); // くるくる回る

	// スケール変化（徐々に小さくなるなど）
	// DrawComponentにはSquashやPulseはあるが、単純な線形縮小機能はないため、
	// Effectクラスに「ScaleTransition」を作るか、Pulseを応用する。
	// 今回は「寿命に合わせてフェードアウト」を設定
	drawComp->StartFadeOut((float)life * (1.0f / 60.0f));
}

void ParticleManager::EmitExplosion(const Vector2& pos, float scale, int texHandle,
	int divX, int divY, int frames, float animSpeed) {

	Particle& p = GetNextParticle();

	// 1. アニメーションとして初期化
	// 寿命 = フレーム数 * 速度 * 60(秒→フレーム変換) + マージン
	int life = static_cast<int>(frames * animSpeed * 60.0f) + 5;

	// 動きはなし
	p.Initialize(pos, { 0,0 }, { 0,0 }, life, texHandle, divX, divY, frames, animSpeed, false);
	p.SetType(ParticleType::Stationary);

	// 2. 演出設定
	auto* drawComp = p.GetDrawComponent();
	drawComp->SetScale(scale, scale);
}

void ParticleManager::EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle, int life) {
	Particle& p = GetNextParticle();

	// 1. 静止画として初期化
	p.Initialize(pos, { 0,0 }, { 0,0 }, life, texHandle);
	p.SetType(ParticleType::Ghost);

	// 2. プレイヤーの状態をコピー
	auto* drawComp = p.GetDrawComponent();
	drawComp->SetScale(scale, scale);
	drawComp->SetRotation(rotation);
	drawComp->SetFlipX(isFlipX);
	drawComp->SetBaseColor(0x8888FFFF); // 青っぽく半透明に

	// 3. フェードアウト設定
	float duration = (float)life / 60.0f;
	drawComp->StartFadeOut(duration);
}

void ParticleManager::Clear() {
	// 強制的に全員殺すには、Initializeでリセットされるので
	// ここでは特に何もしなくて良いが、実装するならフラグを折る
	// for(auto& p : particles_) p.Kill(); // Killメソッドを作れば
}