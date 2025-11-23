#include "DebugWindow.h"
#include "Camera2D.h"
#include "Player.h"
#include "Easing.h"
#include "ParticleManager.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

DebugWindow::DebugWindow() {
	// デフォルト値は既にヘッダーで初期化されているので、
	// 特に何もする必要がない場合は空でOK
}

void DebugWindow::DrawDebugGui() {
#ifdef _DEBUG
	ImGui::Begin("Debug Window - Unified Control");

	ImGui::Text("=== Debug Options ===");
	ImGui::Checkbox("Show Camera Debug", &showCameraWindow_);
	ImGui::Checkbox("Show Player Debug", &showPlayerWindow_);
	ImGui::Checkbox("Show Particle Debug", &showParticleWindow_);

	ImGui::End();
#endif
}

void DebugWindow::DrawCameraDebugWindow(Camera2D* camera) {
#ifdef _DEBUG
	if (!camera || !showCameraWindow_) return;

	ImGui::Begin("Camera Debug", &showCameraWindow_);

	// ========================================
	// デバッグモード切り替え
	// ========================================
	ImGui::Text("=== Debug Mode ===");
	if (ImGui::Checkbox("Enable Camera Debug Mode", &cameraDebugMode_)) {
		// デバッグモードの状態を camera に反映
		camera->isDebugCamera_ = cameraDebugMode_;
	}

	if (cameraDebugMode_) {
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");
	}

	ImGui::Separator();

	// ========================================
	// カメラ情報表示
	// ========================================
	if (ImGui::CollapsingHeader("Camera Info", ImGuiTreeNodeFlags_DefaultOpen)) {
		Vector2 pos = camera->GetPosition();
		float zoom = camera->GetZoom();

		ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);
		ImGui::Text("Zoom: %.2fx", zoom);
		ImGui::Text("Rotation: %.2f rad", camera->rotation_);

		ImGui::Separator();

		// 編集可能な値
		ImGui::Text("Edit Values:");

		float editPos[2] = { pos.x, pos.y };
		if (ImGui::DragFloat2("Position##edit", editPos, 1.0f)) {
			camera->SetPosition({ editPos[0], editPos[1] });
		}

		float editZoom = zoom;
		if (ImGui::SliderFloat("Zoom##edit", &editZoom, 0.1f, 5.0f)) {
			camera->SetZoom(editZoom);
		}

		float editRotation = camera->rotation_;
		if (ImGui::SliderAngle("Rotation##edit", &editRotation)) {
			camera->rotation_ = editRotation;
		}

		if (ImGui::Button("Reset Camera")) {
			camera->SetPosition({ 640.0f, 360.0f });
			camera->SetZoom(1.0f);
			camera->rotation_ = 0.0f;
		}
	}

	// ========================================
	// エフェクトテスト
	// ========================================
	if (ImGui::CollapsingHeader("Camera Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Test Camera Effects:");

		// シェイク
		if (ImGui::TreeNode("Shake Effect")) {
			static float shakeIntensity = 10.0f;
			static float shakeDuration = 0.5f;

			ImGui::SliderFloat("Intensity", &shakeIntensity, 1.0f, 50.0f);
			ImGui::SliderFloat("Duration", &shakeDuration, 0.1f, 3.0f);

			if (ImGui::Button("Start Shake")) {
				camera->Shake(shakeIntensity, shakeDuration);
			}

			ImGui::SameLine();

			if (ImGui::Button("Stop Shake")) {
				camera->StopShake();
			}

			ImGui::Text("Active: %s", camera->shakeEffect_.isActive ? "Yes" : "No");
			if (camera->shakeEffect_.isActive) {
				ImGui::Text("Elapsed: %.2f / %.2f",
					camera->shakeEffect_.elapsed,
					camera->shakeEffect_.duration);
			}

			ImGui::TreePop();
		}

		// ズーム
		if (ImGui::TreeNode("Zoom Effect")) {
			static float targetZoom = 2.0f;
			static float zoomDuration = 1.0f;

			ImGui::SliderFloat("Target Zoom", &targetZoom, 0.1f, 5.0f);
			ImGui::SliderFloat("Duration##zoom", &zoomDuration, 0.1f, 5.0f);

			if (ImGui::Button("Zoom In (2x)")) {
				camera->ZoomTo(2.0f, 1.0f, Easing::EaseOutQuad);
			}

			ImGui::SameLine();

			if (ImGui::Button("Zoom Out (1x)")) {
				camera->ZoomTo(1.0f, 1.0f, Easing::EaseInOutQuad);
			}

			if (ImGui::Button("Zoom to Target")) {
				camera->ZoomTo(targetZoom, zoomDuration, Easing::EaseOutQuad);
			}

			ImGui::Text("Active: %s", camera->zoomEffect_.isActive ? "Yes" : "No");
			if (camera->zoomEffect_.isActive) {
				ImGui::Text("Progress: %.1f%%",
					(camera->zoomEffect_.elapsed / camera->zoomEffect_.duration) * 100.0f);
			}

			ImGui::TreePop();
		}

		// 移動
		if (ImGui::TreeNode("Move Effect")) {
			static float targetPos[2] = { 640.0f, 360.0f };
			static float moveDuration = 2.0f;

			ImGui::DragFloat2("Target Position", targetPos, 1.0f);
			ImGui::SliderFloat("Duration##move", &moveDuration, 0.1f, 5.0f);

			if (ImGui::Button("Move to Center")) {
				camera->MoveTo({ 640.0f, 360.0f }, 2.0f, Easing::EaseOutCubic);
			}

			ImGui::SameLine();

			if (ImGui::Button("Move to Target")) {
				camera->MoveTo({ targetPos[0], targetPos[1] }, moveDuration, Easing::EaseOutQuad);
			}

			ImGui::Text("Active: %s", camera->moveEffect_.isActive ? "Yes" : "No");
			if (camera->moveEffect_.isActive) {
				ImGui::Text("Progress: %.1f%%",
					(camera->moveEffect_.elapsed / camera->moveEffect_.duration) * 100.0f);
			}

			ImGui::TreePop();
		}
	}

	// ========================================
	// 追従設定
	// ========================================
	if (ImGui::CollapsingHeader("Follow Settings")) {
		ImGui::Text("Follow Target: %s", camera->follow_.target ? "Active" : "None");

		float followSpeed = camera->follow_.speed;
		if (ImGui::SliderFloat("Follow Speed", &followSpeed, 0.0f, 1.0f)) {
			camera->SetFollowSpeed(followSpeed);
		}

		float deadZone[2] = { camera->follow_.deadZoneWidth, camera->follow_.deadZoneHeight };
		if (ImGui::DragFloat2("Dead Zone", deadZone, 1.0f, 0.0f, 500.0f)) {
			camera->SetDeadZone(deadZone[0], deadZone[1]);
		}

		if (ImGui::Button("Clear Target")) {
			camera->SetTarget(nullptr);
		}
	}

	// ========================================
	// 境界設定
	// ========================================
	if (ImGui::CollapsingHeader("Bounds Settings")) {
		ImGui::Text("Bounds Enabled: %s", camera->bounds_.enabled ? "Yes" : "No");

		if (camera->bounds_.enabled) {
			ImGui::Text("Left: %.1f, Top: %.1f", camera->bounds_.left, camera->bounds_.top);
			ImGui::Text("Right: %.1f, Bottom: %.1f", camera->bounds_.right, camera->bounds_.bottom);

			if (ImGui::Button("Clear Bounds")) {
				camera->ClearBounds();
			}
		}
		else {
			if (ImGui::Button("Set Default Bounds")) {
				camera->SetBounds(0.0f, 720.0f, 1280.0f, 0.0f);
			}
		}
	}

	// ========================================
	// キーボード操作ガイド
	// ========================================
	if (ImGui::CollapsingHeader("Keyboard Controls")) {
		if (cameraDebugMode_) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Debug Mode Active - Keyboard Controls Enabled");

			ImGui::Separator();

			ImGui::Text("=== Movement ===");
			ImGui::BulletText("Arrow Keys: Move Camera");

			ImGui::Separator();

			ImGui::Text("=== Zoom ===");
			ImGui::BulletText("PageUp: Zoom In");
			ImGui::BulletText("PageDown: Zoom Out");

			ImGui::Separator();

			ImGui::Text("=== Rotation ===");
			ImGui::BulletText("Q: Rotate Left");
			ImGui::BulletText("E: Rotate Right");

			ImGui::Separator();

			ImGui::Text("=== Quick Actions ===");
			ImGui::BulletText("R: Reset Camera");
			ImGui::BulletText("Space: Test Shake");
			ImGui::BulletText("1: Zoom In Animation");
			ImGui::BulletText("2: Zoom Out Animation");
			ImGui::BulletText("3: Move to Center");
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Debug Mode Inactive - Enable to use keyboard controls");
		}
	}

	ImGui::End();
#endif
}

void DebugWindow::DrawPlayerDebugWindow(Player* player) {
#ifdef _DEBUG
	if (!player || !showPlayerWindow_) return;

	ImGui::Begin("Player Debug", &showPlayerWindow_);

	Vector2 pos = player->GetPosition();
	Vector2 vel = player->GetVelocity();

	ImGui::Text("=== Transform ===");
	ImGui::Text("Position: (%.1f, %.1f)", pos.x, pos.y);
	ImGui::Text("Velocity: (%.1f, %.1f)", vel.x, vel.y);

	ImGui::Separator();

	ImGui::Text("=== Status ===");
	ImGui::Text("Alive: %s", player->IsAlive() ? "Yes" : "No");
	ImGui::Text("Radius: %.1f", player->GetRadius());

	ImGui::Separator();

	ImGui::Text("=== Controls ===");
	ImGui::Text("WASD: Move Player");

	ImGui::Separator();

	ImGui::Text("=== Effect Controls ===");
	ImGui::BulletText("Q: Shake");
	ImGui::BulletText("E: Rotation Start");
	ImGui::BulletText("R: Rotation Stop");
	ImGui::BulletText("T: Pulse");
	ImGui::BulletText("Y: Flash");
	ImGui::BulletText("U: Hit Effect");
	ImGui::BulletText("I: Color Red");
	ImGui::BulletText("O: Color Reset");
	ImGui::BulletText("P: Fade Out");
	ImGui::BulletText("L: Fade In");
	ImGui::BulletText("F: Reset All Effects");

	ImGui::End();
#endif
}


// ========================================
// ★追加：パーティクルデバッグウィンドウ
// ========================================
void DebugWindow::DrawParticleDebugWindow(ParticleManager* particleManager) {
#ifdef _DEBUG
	if (!particleManager || !showParticleWindow_) return;

	ImGui::Begin("Particle System Debug", &showParticleWindow_);

	// ========================================
	// 環境パーティクルコントロール
	// ========================================
	if (ImGui::CollapsingHeader("Environment Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent();

		// 雨のコントロール
		ImGui::PushID("Rain");
		ImGui::Text("Rain:");
		ImGui::SameLine(150);

		bool rainActive = particleManager->continuousEmitters_.find(ParticleType::Rain) !=
			particleManager->continuousEmitters_.end() &&
			particleManager->continuousEmitters_[ParticleType::Rain].isActive;

		if (ImGui::Button(rainActive ? "Stop##Rain" : "Start##Rain", ImVec2(80, 0))) {
			if (rainActive) {
				particleManager->StopEnvironmentEffect(ParticleType::Rain);
			}
			else {
				particleManager->StartEnvironmentEffect(ParticleType::Rain, EmitterFollowMode::FollowTarget);
			}
		}

		ImGui::SameLine();
		if (rainActive) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");
		}
		else {
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "INACTIVE");
		}
		ImGui::PopID();

		// 雪のコントロール
		ImGui::PushID("Snow");
		ImGui::Text("Snow:");
		ImGui::SameLine(150);

		bool snowActive = particleManager->continuousEmitters_.find(ParticleType::Snow) !=
			particleManager->continuousEmitters_.end() &&
			particleManager->continuousEmitters_[ParticleType::Snow].isActive;

		if (ImGui::Button(snowActive ? "Stop##Snow" : "Start##Snow", ImVec2(80, 0))) {
			if (snowActive) {
				particleManager->StopEnvironmentEffect(ParticleType::Snow);
			}
			else {
				particleManager->StartEnvironmentEffect(ParticleType::Snow, EmitterFollowMode::FollowTarget);
			}
		}

		ImGui::SameLine();
		if (snowActive) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");
		}
		else {
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "INACTIVE");
		}
		ImGui::PopID();

		// オーブのコントロール
		ImGui::PushID("Orb");
		ImGui::Text("Orb:");
		ImGui::SameLine(150);

		bool orbActive = particleManager->continuousEmitters_.find(ParticleType::Orb) !=
			particleManager->continuousEmitters_.end() &&
			particleManager->continuousEmitters_[ParticleType::Orb].isActive;

		if (ImGui::Button(orbActive ? "Stop##Orb" : "Start##Orb", ImVec2(80, 0))) {
			if (orbActive) {
				particleManager->StopEnvironmentEffect(ParticleType::Orb);
			}
			else {
				particleManager->StartEnvironmentEffect(ParticleType::Orb, EmitterFollowMode::FollowTarget);
			}
		}

		ImGui::SameLine();
		if (orbActive) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ACTIVE");
		}
		else {
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "INACTIVE");
		}
		ImGui::PopID();

		ImGui::Separator();

		// 全停止ボタン
		if (ImGui::Button("Stop All Environment Effects", ImVec2(250, 0))) {
			particleManager->StopEnvironmentEffect(ParticleType::Rain);
			particleManager->StopEnvironmentEffect(ParticleType::Snow);
			particleManager->StopEnvironmentEffect(ParticleType::Orb);
		}

		ImGui::Unindent();
	}

	ImGui::Separator();

	// ========================================
	// アクティブパーティクル統計
	// ========================================
	if (ImGui::CollapsingHeader("Active Particles", ImGuiTreeNodeFlags_DefaultOpen)) {
		// パーティクルタイプごとにカウント
		std::map<ParticleType, int> particleCounts;
		int totalActive = 0;

		for (const auto& p : particleManager->particles_) {
			if (p.IsAlive()) {
				particleCounts[p.GetType()]++;
				totalActive++;
			}
		}

		ImGui::Text("Total Active: %d / %d", totalActive, particleManager->kMaxParticles);
		ImGui::ProgressBar(static_cast<float>(totalActive) / particleManager->kMaxParticles,
			ImVec2(-1, 0), "");

		ImGui::Separator();

		// タイプ別の詳細
		ImGui::BeginChild("ParticleTypeList", ImVec2(0, 200), true);

		const char* typeNames[] = {
			"Explosion", "Debris", "Hit", "Dust", "MuzzleFlash",
			"Rain", "Snow", "Orb", "Charge",
			"Glow", "Shockwave", "Sparkle", "Slash", "SmokeCloud"
		};

		for (int i = 0; i < 14; ++i) {
			ParticleType type = static_cast<ParticleType>(i);
			int count = particleCounts[type];

			if (count > 0) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			}

			ImGui::BulletText("%s: %d", typeNames[i], count);
			ImGui::PopStyleColor();
		}

		ImGui::EndChild();
	}

	ImGui::Separator();

	// ========================================
	// パラメータ編集
	// ========================================
	if (ImGui::CollapsingHeader("Parameter Editor")) {
		static int selectedType = 5; // デフォルトはRain
		const char* typeNames[] = {
			"Explosion", "Debris", "Hit", "Dust", "MuzzleFlash",
			"Rain", "Snow", "Orb", "Charge",
			"Glow", "Shockwave", "Sparkle", "Slash", "SmokeCloud"
		};

		ImGui::Combo("Particle Type", &selectedType, typeNames, 14);

		ParticleType type = static_cast<ParticleType>(selectedType);
		ParticleParam* param = particleManager->GetParam(type);

		if (param) {
			ImGui::Separator();
			ImGui::Text("=== Basic Parameters ===");
			ImGui::SliderInt("Count", &param->count, 1, 100);
			ImGui::SliderInt("Life Min", &param->lifeMin, 1, 600);
			ImGui::SliderInt("Life Max", &param->lifeMax, 1, 600);

			if (type == ParticleType::Rain || type == ParticleType::Snow || type == ParticleType::Orb) {
				ImGui::Separator();
				ImGui::Text("=== Environment Specific ===");

				if (type == ParticleType::Rain) {
					ImGui::SliderFloat("Bounce Damping", &param->bounceDamping, 0.0f, 1.0f);
				}

				if (type == ParticleType::Snow) {
					ImGui::SliderFloat("Wind Strength", &param->windStrength, 0.0f, 100.0f);
				}

				if (type == ParticleType::Orb) {
					ImGui::SliderFloat("Float Amplitude", &param->floatAmplitude, 0.0f, 100.0f);
					ImGui::SliderFloat("Float Frequency", &param->floatFrequency, 0.1f, 5.0f);
				}
			}

			ImGui::Separator();

			if (ImGui::Button("Save Parameters to JSON", ImVec2(250, 0))) {
				particleManager->SaveParamsToJson("Resources/Data/particle_params.json");
			}
		}
	}

	ImGui::Separator();

	// ========================================
	// クイックテスト
	// ========================================
	if (ImGui::CollapsingHeader("Quick Test")) {
		ImGui::Text("Emit particles at center:");

		if (ImGui::Button("Explosion", ImVec2(100, 0))) {
			particleManager->Emit(ParticleType::Explosion, { 640.0f, 360.0f });
		}
		ImGui::SameLine();
		if (ImGui::Button("Debris", ImVec2(100, 0))) {
			particleManager->Emit(ParticleType::Debris, { 640.0f, 360.0f });
		}
		ImGui::SameLine();
		if (ImGui::Button("Hit", ImVec2(100, 0))) {
			particleManager->Emit(ParticleType::Hit, { 640.0f, 360.0f });
		}

		if (ImGui::Button("Clear All Particles", ImVec2(250, 0))) {
			particleManager->Clear();
		}
	}

	ImGui::End();
#endif
}