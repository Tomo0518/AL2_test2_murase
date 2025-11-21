#include "ParticleManager.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

#ifdef _DEBUG
#include <imgui.h>
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

// 度数法 -> ラジアン変換
static const float kDeg2Rad = 3.14159265f / 180.0f;

ParticleManager::ParticleManager() {
	LoadParams();
}

void ParticleManager::LoadParams() {
	// 1. 爆発（全方位に広がるアニメーション）
	ParticleParam explosion;
	explosion.count = 1;
	explosion.lifeMin = 40;
	explosion.lifeMax = 40;
	explosion.speedMin = 0.0f;
	explosion.speedMax = 0.0f;
	explosion.angleBase = 0.0f;
	explosion.angleRange = 0.0f;
	explosion.gravity = { 0.0f, 0.0f };
	explosion.sizeMin = 64.0f;
	explosion.sizeMax = 64.0f;
	explosion.scaleStart = 1.0f;
	explosion.scaleEnd = 1.0f;
	explosion.colorStart = 0xFFFFFFFF;
	explosion.colorEnd = 0xFFFFFFFF;
	explosion.useAnimation = true;
	explosion.divX = 4;
	explosion.divY = 1;
	explosion.totalFrames = 4;
	explosion.animSpeed = 0.05f;
	params_[ParticleType::Explosion] = explosion;

	// 2. デブリ（岩の破片が飛び散る）
	ParticleParam debris;
	debris.count = 10;
	debris.lifeMin = 40;
	debris.lifeMax = 60;
	debris.speedMin = 150.0f;
	debris.speedMax = 300.0f;
	debris.angleBase = 0.0f;
	debris.angleRange = 360.0f;
	debris.gravity = { 0.0f, -800.0f };
	debris.sizeMin = 12.0f;    // 破片は小さめ
	debris.sizeMax = 24.0f;
	debris.scaleStart = 1.0f;
	debris.scaleEnd = 0.3f;
	debris.colorStart = 0xFFFFFFFF;
	debris.colorEnd = 0xFFFFFF00;
	debris.useAnimation = false;
	debris.divX = 1;
	debris.divY = 1;
	debris.totalFrames = 1;
	debris.animSpeed = 0.0f;
	params_[ParticleType::Debris] = debris;

	// 3. ヒットエフェクト（破片が多方面に飛び散る）
	ParticleParam hit;
	hit.count = 18;
	hit.lifeMin = 15;
	hit.lifeMax = 55;
	hit.speedMin = 225.0f;
	hit.speedMax = 280.0f;
	hit.angleBase = 90.0f;
	hit.angleRange = 1550.0f;
	hit.gravity = { 0.0f, -300.0f };
	hit.sizeMin = 27.0f;       // ヒット破片は小さめ
	hit.sizeMax = 43.0f;
	hit.scaleStart = 0.8f;
	hit.scaleEnd = 0.2f;
	hit.colorStart = 0xFFFFFFFF;
	hit.colorEnd = 0xFFFFFF00;
	hit.useAnimation = false;
	hit.divX = 1;
	hit.divY = 1;
	hit.totalFrames = 1;
	hit.animSpeed = 0.0f;
	params_[ParticleType::Hit] = hit;

	// 4. 土煙
	ParticleParam dust;
	dust.count = 8;
	dust.lifeMin = 30;
	dust.lifeMax = 45;
	dust.speedMin = 50.0f;
	dust.speedMax = 100.0f;
	dust.angleBase = -90.0f;
	dust.angleRange = 60.0f;
	dust.gravity = { 0.0f, -50.0f };
	dust.sizeMin = 32.0f;     // 土煙は大きめ
	dust.sizeMax = 64.0f;
	dust.scaleStart = 0.5f;
	dust.scaleEnd = 1.5f;
	dust.colorStart = 0xAAAAAAAA;
	dust.colorEnd = 0xAAAAAA00;
	dust.useAnimation = false;
	dust.divX = 1;
	dust.divY = 1;
	dust.totalFrames = 1;
	dust.animSpeed = 0.0f;
	params_[ParticleType::Dust] = dust;

	// 5. マズルフラッシュ
	ParticleParam muzzle;
	muzzle.count = 1;
	muzzle.lifeMin = 8;
	muzzle.lifeMax = 12;
	muzzle.speedMin = 0.0f;
	muzzle.speedMax = 0.0f;
	muzzle.angleBase = 0.0f;
	muzzle.angleRange = 0.0f;
	muzzle.gravity = { 0.0f, 0.0f };
	muzzle.sizeMin = 32.0f;
	muzzle.sizeMax = 48.0f;
	muzzle.scaleStart = 1.2f;
	muzzle.scaleEnd = 0.3f;
	muzzle.colorStart = 0xFFFFFFFF;
	muzzle.colorEnd = 0xFFFFFF00;
	muzzle.useAnimation = false;
	muzzle.divX = 1;
	muzzle.divY = 1;
	muzzle.totalFrames = 1;
	muzzle.animSpeed = 0.0f;
	params_[ParticleType::MuzzleFlash] = muzzle;
}

void ParticleManager::Update(float deltaTime) {
	for (auto& p : particles_) {
		if (p.IsAlive()) {
			p.Update(deltaTime);
		}
	}
}

void ParticleManager::Draw(const Camera2D& camera) {
	for (auto& p : particles_) {
		if (p.IsAlive()) {
			p.Draw(camera);
		}
	}
}

void ParticleManager::Emit(ParticleType type, const Vector2& pos) {
	// 指定されたタイプの設定を取得
	if (params_.find(type) == params_.end()) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager::Emit - Invalid ParticleType\n");
#endif
		return;
	}

	const ParticleParam& param = params_[type];

	// テクスチャが無効な場合はスキップ
	if (param.textureHandle < 0) {
#ifdef _DEBUG
		Novice::ConsolePrintf("ParticleManager::Emit - Invalid texture handle: %d\n", param.textureHandle);
#endif
		return;
	}

	// 設定された個数ぶん発生させる
	for (int i = 0; i < param.count; ++i) {
		Particle& p = GetNextParticle();

		// --- ランダム計算 ---
		int life = static_cast<int>(RandomFloat(static_cast<float>(param.lifeMin), static_cast<float>(param.lifeMax)));

		// 速度ベクトル
		float speed = RandomFloat(param.speedMin, param.speedMax);
		float halfRange = param.angleRange / 2.0f;
		float angleDeg = param.angleBase + RandomFloat(-halfRange, halfRange);
		float angleRad = angleDeg * kDeg2Rad;
		Vector2 vel = { cosf(angleRad) * speed, sinf(angleRad) * speed };

		// サイズをランダムに決定（0.7～1.3のランダム要素を含む）
		float baseSize = RandomFloat(param.sizeMin, param.sizeMax);
		float sizeVariation = RandomFloat(0.7f, 1.3f);
		float finalSize = baseSize * sizeVariation;

		// パーティクル初期化
		p.Initialize(
			pos, vel, param.gravity, life,
			param.textureHandle,
			param.divX, param.divY, param.totalFrames, param.animSpeed, false
		);
		p.SetBehavior(ParticleBehavior::Physics);

		// --- DrawComponentへの追加設定 ---
		auto* drawComp = p.GetDrawComponent();
		if (!drawComp) {
#ifdef _DEBUG
			Novice::ConsolePrintf("ParticleManager::Emit - DrawComponent is null\n");
#endif
			continue;
		}

		// サイズを設定
		drawComp->SetDrawSize(finalSize, finalSize);

		// 色設定
		drawComp->SetBaseColor(param.colorStart);
		if (param.colorStart != param.colorEnd) {
			float duration = life / 60.0f;
			drawComp->StartColorTransition(ColorRGBA::FromUInt(param.colorEnd), duration);
		}

		// スケール設定
		drawComp->SetScale(param.scaleStart, param.scaleStart);

		// スケール変化がある場合
		if (std::abs(param.scaleEnd - param.scaleStart) > 0.01f) {
			float duration = life / 60.0f;
			drawComp->StartSquash({ param.scaleEnd, param.scaleEnd }, duration);
		}

		// 全てのパーティクルにフェードアウトを適用
		float duration = life / 60.0f;
		drawComp->StartFadeOut(duration);
	}
}

void ParticleManager::EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle) {
	if (texHandle < 0) return;

	Particle& p = GetNextParticle();
	p.Initialize(pos, { 0,0 }, { 0,0 }, 20, texHandle, 1, 1, 1, 0.0f, false);
	p.SetBehavior(ParticleBehavior::Ghost);

	auto* drawComp = p.GetDrawComponent();
	if (!drawComp) return;

	drawComp->SetScale(scale, scale);
	drawComp->SetRotation(rotation);
	drawComp->SetFlipX(isFlipX);
	drawComp->SetBaseColor(0x8888FFFF);
	drawComp->StartFadeOut(20.0f / 60.0f);
}

void ParticleManager::Clear() {
	for (auto& p : particles_) {
		p.Initialize({ 0,0 }, { 0,0 }, { 0,0 }, 0, -1, 1, 1, 1, 0.0f, false);
	}
	nextIndex_ = 0;
}

void ParticleManager::LoadCommonResources() {
	// テクスチャを一括ロード
	texExplosion_ = Novice::LoadTexture("./Resources/images/effect/explosion.png");
	texDebris_ = Novice::LoadTexture("./Resources/images/effect/debris.png");
	texHit_ = Novice::LoadTexture("./Resources/images/effect/hit_ver1.png");
	texDust_ = Novice::LoadTexture("./Resources/images/effect/hit_ver1.png");

#ifdef _DEBUG
	Novice::ConsolePrintf("ParticleManager::LoadCommonResources\n");
	Novice::ConsolePrintf("  Explosion: %d\n", texExplosion_);
	Novice::ConsolePrintf("  Debris: %d\n", texDebris_);
	Novice::ConsolePrintf("  Hit: %d\n", texHit_);
	Novice::ConsolePrintf("  Dust: %d\n", texDust_);
#endif

	// パラメータにハンドルをセット
	params_[ParticleType::Explosion].textureHandle = texExplosion_;
	params_[ParticleType::Debris].textureHandle = texDebris_;
	params_[ParticleType::Hit].textureHandle = texHit_;
	params_[ParticleType::Dust].textureHandle = texDust_;
	params_[ParticleType::MuzzleFlash].textureHandle = texHit_;
}

Particle& ParticleManager::GetNextParticle() {
	Particle& p = particles_[nextIndex_];
	nextIndex_ = (nextIndex_ + 1) % kMaxParticles;
	return p;
}

float ParticleManager::RandomFloat(float min, float max) {
	if (min >= max) return min;
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

void ParticleManager::DrawDebugWindow() {
#ifdef _DEBUG
	ImGui::Begin("Particle Manager", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// ========== エフェクトタイプ選択 ==========
	ImGui::Text("=== Effect Type ===");
	const char* items[] = { "Explosion", "Debris", "Hit", "Dust", "MuzzleFlash" };
	int currentItem = static_cast<int>(currentDebugType_);
	if (ImGui::Combo("Type", &currentItem, items, IM_ARRAYSIZE(items))) {
		currentDebugType_ = static_cast<ParticleType>(currentItem);
	}

	ParticleParam& p = params_[currentDebugType_];

	ImGui::Separator();

	// ========== 基本設定 ==========
	ImGui::Text("=== Basic Settings ===");

	ImGui::SliderInt("Count", &p.count, 1, 50);
	ImGui::SameLine();
	if (ImGui::Button("-##Count")) p.count = std::max(1, p.count - 1);
	ImGui::SameLine();
	if (ImGui::Button("+##Count")) p.count = std::min(50, p.count + 1);

	ImGui::SliderInt("Life Min", &p.lifeMin, 1, 300);
	ImGui::SliderInt("Life Max", &p.lifeMax, 1, 300);
	if (p.lifeMin > p.lifeMax) p.lifeMax = p.lifeMin;

	ImGui::Separator();

	// ========== サイズ設定 ==========
	ImGui::Text("=== Size (Pixels) ===");
	ImGui::SliderFloat("Size Min", &p.sizeMin, 4.0f, 256.0f);
	ImGui::SliderFloat("Size Max", &p.sizeMax, 4.0f, 256.0f);
	if (p.sizeMin > p.sizeMax) p.sizeMax = p.sizeMin;

	ImGui::Separator();

	// ========== 物理設定 ==========
	ImGui::Text("=== Physics ===");

	ImGui::SliderFloat("Speed Min", &p.speedMin, 0.0f, 1000.0f);
	ImGui::SliderFloat("Speed Max", &p.speedMax, 0.0f, 1000.0f);
	if (p.speedMin > p.speedMax) p.speedMax = p.speedMin;

	ImGui::SliderFloat("Base Angle", &p.angleBase, -180.0f, 180.0f);
	ImGui::SameLine();
	if (ImGui::Button("↑##Up")) p.angleBase = -90.0f;
	ImGui::SameLine();
	if (ImGui::Button("→##Right")) p.angleBase = 0.0f;
	ImGui::SameLine();
	if (ImGui::Button("↓##Down")) p.angleBase = 90.0f;
	ImGui::SameLine();
	if (ImGui::Button("←##Left")) p.angleBase = 180.0f;

	ImGui::SliderFloat("Angle Range", &p.angleRange, 0.0f, 360.0f);
	ImGui::SameLine();
	if (ImGui::Button("360°##Full")) p.angleRange = 360.0f;

	ImGui::DragFloat2("Gravity", &p.gravity.x, 10.0f, -2000.0f, 2000.0f);

	ImGui::Separator();

	// ========== 見た目設定 ==========
	ImGui::Text("=== Appearance ===");

	ImGui::SliderFloat("Start Scale", &p.scaleStart, 0.1f, 5.0f);
	ImGui::SliderFloat("End Scale", &p.scaleEnd, 0.0f, 5.0f);

	ImGui::Text("Start Color:");
	ColorRGBA startColor = ColorRGBA::FromUInt(p.colorStart);
	float startRGBA[4] = { startColor.r, startColor.g, startColor.b, startColor.a };
	if (ImGui::ColorEdit4("##StartColor", startRGBA)) {
		p.colorStart = ColorRGBA(startRGBA[0], startRGBA[1], startRGBA[2], startRGBA[3]).ToUInt();
	}

	ImGui::Text("End Color:");
	ColorRGBA endColor = ColorRGBA::FromUInt(p.colorEnd);
	float endRGBA[4] = { endColor.r, endColor.g, endColor.b, endColor.a };
	if (ImGui::ColorEdit4("##EndColor", endRGBA)) {
		p.colorEnd = ColorRGBA(endRGBA[0], endRGBA[1], endRGBA[2], endRGBA[3]).ToUInt();
	}

	ImGui::Separator();

	// ========== アニメーション設定 ==========
	ImGui::Text("=== Animation (Explosion Only) ===");
	ImGui::Checkbox("Use Animation", &p.useAnimation);

	if (p.useAnimation) {
		ImGui::Indent();
		ImGui::SliderInt("Div X", &p.divX, 1, 10);
		ImGui::SliderInt("Div Y", &p.divY, 1, 10);
		ImGui::SliderInt("Total Frames", &p.totalFrames, 1, 100);
		ImGui::SliderFloat("Anim Speed", &p.animSpeed, 0.01f, 0.5f);
		ImGui::Unindent();
	}

	ImGui::Separator();

	// ========== テスト発射 ==========
	ImGui::Text("=== Test ===");
	if (ImGui::Button("Emit at Center (640, 360)", ImVec2(200, 30))) {
		Emit(currentDebugType_, { 640.0f, 360.0f });
	}

	if (ImGui::Button("Reset to Default")) {
		LoadParams();
	}

	ImGui::End();
#endif
}