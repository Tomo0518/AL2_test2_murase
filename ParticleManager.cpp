#include "ParticleManager.h"
#include <cmath>
#include <cstdlib> // rand
#include <algorithm> // clamp

#ifdef _DEBUG
#include <imgui.h>
#endif

// 度数法 -> ラジアン変換
static const float kDeg2Rad = 3.14159265f / 180.0f;

ParticleManager::ParticleManager() {
	LoadParams(); // デフォルト値をロード
}

void ParticleManager::LoadParams() {
	// 1. 爆発（全方位に広がるアニメーション）
	ParticleParam explosion;
	explosion.count = 1; // アニメーション自体が大きいなら1個でいい
	explosion.lifeMin = 40; explosion.lifeMax = 40; // アニメ再生時間と合わせる
	explosion.scaleStart = 2.0f; explosion.scaleEnd = 2.0f;
	explosion.useAnimation = true;
	explosion.divX = 5; explosion.divY = 5; explosion.totalFrames = 25; explosion.animSpeed = 0.05f;
	// テクスチャは適宜ロードしたハンドルを入れる（ここでは仮に0や変数を想定）
	// explosion.textureHandle = ...
	params_[ParticleType::Explosion] = explosion;

	// 2. デブリ（岩の破片が飛び散る）
	ParticleParam debris;
	debris.count = 5; // 1回で5個散らばる
	debris.lifeMin = 40; debris.lifeMax = 60;
	debris.speedMin = 150.0f; debris.speedMax = 300.0f;
	debris.angleBase = -90.0f; // 上方向中心
	debris.angleRange = 120.0f; // 扇状に広がる
	debris.gravity = { 0.0f, 800.0f }; // 重力あり
	debris.scaleStart = 1.0f; debris.scaleEnd = 0.0f; // 小さくなって消える
	params_[ParticleType::Debris] = debris;

	// 3. ヒットエフェクト（ピカッと光る）
	ParticleParam hit;
	hit.count = 1;
	hit.lifeMin = 15; hit.lifeMax = 15;
	hit.useAnimation = true;
	hit.divX = 4; hit.divY = 1; hit.totalFrames = 4; hit.animSpeed = 0.05f;
	hit.colorStart = 0xFFFFFFFF; hit.colorEnd = 0xFFFFFF00;
	params_[ParticleType::Hit] = hit;

	// 他のタイプもここで初期化...
}

void ParticleManager::Update(float deltaTime) {
	for (auto& p : particles_) {
		if (p.IsAlive()) {
			p.Update(deltaTime);
		}
	}
}

void ParticleManager::Draw(const Camera2D& camera) {
	// 加算合成にすると爆発などが綺麗に見える
	// Novice::SetBlendMode(kBlendModeAdd);
	for (auto& p : particles_) {
		if (p.IsAlive()) {
			p.Draw(camera);
		}
	}
	// Novice::SetBlendMode(kBlendModeNormal);
}

void ParticleManager::Emit(ParticleType type, const Vector2& pos) {
	// 指定されたタイプの設定を取得
	if (params_.find(type) == params_.end()) return;
	const ParticleParam& param = params_[type];

	// 設定された個数ぶん発生させる
	for (int i = 0; i < param.count; ++i) {
		Particle& p = GetNextParticle();

		// --- ランダム計算 ---

		// 寿命
		int life = (int)RandomFloat((float)param.lifeMin, (float)param.lifeMax);

		// 速度ベクトル
		float speed = RandomFloat(param.speedMin, param.speedMax);
		// 角度: Baseを中心に -Range/2 ～ +Range/2 の範囲
		float halfRange = param.angleRange / 2.0f;
		float angleDeg = param.angleBase + RandomFloat(-halfRange, halfRange);
		float angleRad = angleDeg * kDeg2Rad;
		Vector2 vel = { cosf(angleRad) * speed, sinf(angleRad) * speed };

		// 初期化
		p.Initialize(
			pos, vel, param.gravity, life,
			param.textureHandle,
			param.divX, param.divY, param.totalFrames, param.animSpeed, false
		);
		p.SetType(ParticleType::Physics); // 基本は物理挙動

		// --- DrawComponentへの追加設定 ---
		auto* drawComp = p.GetDrawComponent();

		// 色：開始色をセットし、終了色へ遷移させる
		drawComp->SetBaseColor(param.colorStart);
		if (param.colorStart != param.colorEnd) {
			// 寿命(秒)に合わせて色変化
			drawComp->StartColorTransition(
				ColorRGBA::FromUInt(param.colorEnd),
				life / 60.0f
			);
		}

		// スケール：開始サイズをセット
		drawComp->SetScale(param.scaleStart, param.scaleStart);
		// 終了サイズが違うなら、フェードやPulseではなく自前の線形補間が必要だが、
		// 簡易的に「FadeOut」を使うか、Particleクラス側でスケール補間機能をつける。
		// 今回は「寿命が来たら消える」のが基本なので、FadeOutを設定
		if (param.scaleEnd < param.scaleStart) {
			// 簡易的にフェードアウト（透明化）と合わせて縮小っぽく見せる
			drawComp->StartFadeOut(life / 60.0f);
		}
	}
}

void ParticleManager::EmitDashGhost(const Vector2& pos, float scale, float rotation, bool isFlipX, int texHandle) {
	// 特殊系はこれまで通り個別にロジックを書く
	Particle& p = GetNextParticle();
	p.Initialize(pos, { 0,0 }, { 0,0 }, 20, texHandle);
	p.SetType(ParticleType::Ghost);

	auto* drawComp = p.GetDrawComponent();
	drawComp->SetScale(scale, scale);
	drawComp->SetRotation(rotation);
	drawComp->SetFlipX(isFlipX);
	drawComp->SetBaseColor(0x8888FFFF); // 青い残像
	drawComp->StartFadeOut(20.0f / 60.0f);
}

void ParticleManager::Clear() {
	// 実装に応じてフラグクリアなど
}

Particle& ParticleManager::GetNextParticle() {
	Particle& p = particles_[nextIndex_];
	nextIndex_ = (nextIndex_ + 1) % kMaxParticles;
	return p;
}

float ParticleManager::RandomFloat(float min, float max) {
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

void ParticleManager::DrawDebugWindow() {
#ifdef _DEBUG
	ImGui::Begin("Particle Manager");

	// タイプの選択
	const char* items[] = { "Explosion", "Debris", "Hit", "Dust" };
	// 列挙型をintにキャストして扱う
	int currentItem = static_cast<int>(currentDebugType_);
	if (ImGui::Combo("Type", &currentItem, items, IM_ARRAYSIZE(items))) {
		currentDebugType_ = static_cast<ParticleType>(currentItem);
	}

	// 選択中のパラメータを取得して編集
	ParticleParam& p = params_[currentDebugType_];

	ImGui::Separator();
	ImGui::Text("Basic");
	ImGui::InputInt("Texture Handle", &p.textureHandle);
	ImGui::DragInt("Count", &p.count, 1, 1, 50);
	ImGui::DragIntRange2("Life (Frames)", &p.lifeMin, &p.lifeMax, 1, 1, 600);

	ImGui::Separator();
	ImGui::Text("Physics");
	ImGui::DragFloatRange2("Speed", &p.speedMin, &p.speedMax, 1.0f, 0.0f, 1000.0f);
	ImGui::DragFloat("Base Angle", &p.angleBase, 1.0f, -360.0f, 360.0f);
	ImGui::DragFloat("Angle Range", &p.angleRange, 1.0f, 0.0f, 360.0f);
	ImGui::DragFloat2("Gravity", &p.gravity.x, 1.0f);

	ImGui::Separator();
	ImGui::Text("Appearance");
	ImGui::DragFloat("Start Scale", &p.scaleStart, 0.1f, 0.0f, 10.0f);
	ImGui::DragFloat("End Scale", &p.scaleEnd, 0.1f, 0.0f, 10.0f);

	// 色編集はImGui::ColorEdit4を使うと便利
	// ここでは簡易的にHex表示
	// ImGui::ColorEdit4... を実装する場合は ColorRGBA変換が必要

	ImGui::Separator();
	ImGui::Text("Animation");
	ImGui::Checkbox("Use Anim", &p.useAnimation);
	if (p.useAnimation) {
		ImGui::DragInt("Div X", &p.divX, 1, 1, 10);
		ImGui::DragInt("Div Y", &p.divY, 1, 1, 10);
		ImGui::DragInt("Total Frames", &p.totalFrames, 1, 1, 100);
		ImGui::DragFloat("Anim Speed", &p.animSpeed, 0.01f, 0.0f, 1.0f);
	}

	// テスト発射ボタン
	if (ImGui::Button("Test Emit")) {
		Emit(currentDebugType_, { 640.0f, 360.0f }); // 画面中央に発射
	}

	ImGui::End();
#endif
}