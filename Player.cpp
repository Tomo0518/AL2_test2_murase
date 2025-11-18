#include "Player.h"
#include <Novice.h>
#include <algorithm>
#include <cmath>
#include "Easing.h"
#include "GameShared.h"
#include "ScrapManager.h"
#include "Boss.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

Player::Player(Pad& pad, GameShared& shared) {
	pad_ = &pad;
	shared_ = &shared;

	InitializeDrawComponents();
	Initialize();
}

void Player::Initialize() {
	// 位置の初期化
	pos_ = { 128.0f, 96.0f };

	// 速度の初期化
	velocity_ = { 0.0f, 0.0f };
	recoilVelocity_ = { 0.0f, 0.0f };

	isAlive_ = true;
	wasAlive_ = true;

	hitPoint_ = kMaxHitPoint_;

	// スケールの初期化
	scale_ = { 1.0f, 1.0f };
	angle_ = 0.0f;
	recoilAngleOffset_ = 0.0f;

	// スクラップ関連の初期化
	vaccumPos_ = pos_;
	currentWeight_ = 0.0f;
	isSucking_ = false;
	wasSucking_ = false;
	isShooting_ = false;

	// 描画状態の初期化
	drawState_ = PlayerState::Normal;
	prevDrawState_ = PlayerState::Normal;
	shootingStateTimer_ = 0.0f;

	// アニメーションのリセット
	ResetAnimations();


	// 軌跡エフェクトの初期化
	InitializeTrailEffect();
	drawCompShotMovementEffect_.reserve(kMaxTrailEffects_);

	InitializeMagneticEffect();
}

void Player::InitializeDrawComponents() {
	// 通常時の描画コンポーネントの初期化
	drawCompNormal_ = DrawComponent2D(
		pos_, radius_ * 2.0f, radius_ * 2.0f,
		Novice::LoadTexture("./Resources/images/tomo/player_normal_ver1.png"), 64, 64, 1, 1, 0.1f, true
	);

	// 吸収中の描画コンポーネントの初期化
	drawCompSucking_ = DrawComponent2D(
		pos_, radius_ * 2.0f, radius_ * 2.0f,
		Novice::LoadTexture("./Resources/images/tomo/player_sucking_ver1.png"), 64, 64, 1, 1, 0.1f, true
	);

	// 発射中の描画コンポーネントの初期化
	drawCompShooting_ = DrawComponent2D(
		pos_, radius_ * 2.0f, radius_ * 2.0f,
		Novice::LoadTexture("./Resources/images/tomo/player_shooting_ver1.png"), 64, 64, 1, 1, 0.1f, true
	);

	// 死亡エフェクトの描画コンポーネントの初期化
	drawCompDeadEffect_ = DrawComponent2D(
		pos_, 64.0f * 3.0f, 64.0f * 3.0f,
		Novice::LoadTexture("./Resources/images/tomo/playerDeadEffect.png"), 192, 192, 8, 8, 0.1f, false
	);

	drawCompShotMovementEffect_.reserve(20);

	for (auto& moveEffect : drawCompShotMovementEffect_) {
		moveEffect = std::make_unique<DrawComponent2D>(
			pos_, 32.0f, 32.0f,
			Novice::LoadTexture("./Resources/images/tomo/player_particle.png"), 600, 600, 1, 1, 0.05f, false
		);
	}
}

void Player::ResetAnimations() {
	drawCompNormal_.StopAnimation();
	drawCompNormal_.PlayAnimation();

	drawCompSucking_.StopAnimation();
	drawCompSucking_.PlayAnimation();

	drawCompShooting_.StopAnimation();
	drawCompShooting_.PlayAnimation();

	drawCompDeadEffect_.StopAnimation();
	drawCompDeadEffect_.PlayAnimation();
}

void Player::UpdateDrawState() {
	// 前回の状態を保存
	prevDrawState_ = drawState_;

	// 死亡状態
	if (!isAlive_) {
		drawState_ = PlayerState::DeadEffect;
		return;
	}

	// 発射状態のタイマー更新
	if (drawState_ == PlayerState::Shooting) {
		shootingStateTimer_ -= 1.0f / 60.0f; // フレーム時間を引く
		if (shootingStateTimer_ <= 0.0f) {
			isShooting_ = false;
		}
	}

	// 状態判定の優先順位: 発射 > 吸引 > 通常
	if (isShooting_) {
		drawState_ = PlayerState::Shooting;
	}
	else if (isSucking_) {
		drawState_ = PlayerState::Sucking;
	}
	else {
		drawState_ = PlayerState::Normal;
	}

	// 状態が変わったときの処理
	if (prevDrawState_ != drawState_) {
		switch (drawState_) {
		case PlayerState::Normal:
			drawCompNormal_.PlayAnimation();
			break;
		case PlayerState::Sucking:
			drawCompSucking_.PlayAnimation();
			break;
		case PlayerState::Shooting:
			drawCompShooting_.PlayAnimation();
			break;
		case PlayerState::DeadEffect:
			drawCompDeadEffect_.PlayAnimation();
			break;
		}
	}
}

void Player::UpdateDrawComponents(float dt) {
	// 反動角度オフセットを含めた描画用の角度を計算
	float displayAngle = angle_ + recoilAngleOffset_;

	// 現在の状態に応じてアニメーションを更新
	switch (drawState_) {
	case PlayerState::Normal:
		drawCompNormal_.UpdateEffects(dt);
		drawCompNormal_.UpdateAnimation(dt, pos_, displayAngle, scale_);
		break;
	case PlayerState::Sucking:
		drawCompSucking_.UpdateEffects(dt);
		drawCompSucking_.UpdateAnimation(dt, pos_, displayAngle, scale_);
		break;
	case PlayerState::Shooting:
		drawCompShooting_.UpdateEffects(dt);
		drawCompShooting_.UpdateAnimation(dt, pos_, displayAngle, scale_);
		break;
	case PlayerState::DeadEffect:
		drawCompDeadEffect_.UpdateEffects(dt);
		drawCompDeadEffect_.UpdateAnimation(dt, pos_, displayAngle, scale_);
		break;
	}
}

// ビジュアルエフェクトの更新
void Player::UpdateVisualEffects() {
	// 重量の割合を計算（maxWeightを超えた場合は1.0にクランプ）
	float weightRatio = std::min(currentWeight_ / maxWeight_, 1.0f);

	switch (drawState_) {
	case PlayerState::Normal:
		// 通常状態では全エフェクトを停止
		if (drawCompNormal_.IsShakeActive()) {
			drawCompNormal_.StopShake();
		}
		break;

	case PlayerState::Sucking:
		// 吸引中：重量に応じたシェイクエフェクト
		if (weightRatio > 0.01f) {
			// シェイク範囲を重量に応じて設定
			float shakeRange = kShakeRangeMax_ * weightRatio;

			// 既に動作中でも、範囲を更新するため再設定
			drawCompSucking_.StopShake();
			drawCompSucking_.StartShakeContinuous(shakeRange);
		}
		else {
			// 重量がほぼゼロの場合はシェイクを停止
			if (drawCompSucking_.IsShakeActive()) {
				drawCompSucking_.StopShake();
			}
		}
		break;

	case PlayerState::Shooting:
		// 発射状態では追加処理なし(UpdateFireでSquashStretchを開始済み）

		break;

	case PlayerState::DeadEffect:
		// 死亡エフェクト中は全エフェクトを停止
		if (drawCompDeadEffect_.IsShakeActive()) {
			drawCompDeadEffect_.StopShake();
		}
		break;
	}

	// 吸引状態以外ではSuckingコンポーネントのシェイクを確実に停止
	if (drawState_ != PlayerState::Sucking) {
		if (drawCompSucking_.IsShakeActive()) {
			drawCompSucking_.StopShake();
		}
	}
}

void Player::Update(float dt, const char* keys, const char* preKeys, const Vector2& offset, Pad& pad, ScrapManager* scrapManager, Boss* boss) {
	offset;
	preKeys;

	if (!isAlive_) {
		// 死亡状態の更新
		UpdateDrawState();
		UpdateDrawComponents(dt);
		return;
	}

	// マウス座標取得
	Novice::GetMousePosition(&mouseX_, &mouseY_);

	// 吸引位置の更新
	UpdateVaccumPosition(pad);

	// 吸引処理
	UpdateSuction(scrapManager, pad, boss);

	// 発射処理
	UpdateFire(scrapManager);

	// 移動処理
	UpdateMovement(dt, keys, pad);

	// 反動の更新
	UpdateRecoil(dt);

	// 軌跡エフェクトの更新
	UpdateShotMovementEffects(dt);

	// 位置更新
	pos_ += velocity_ * dt;
	pos_ += recoilVelocity_ * dt;

	// 描画状態の更新
	UpdateDrawState();

	// ビジュアルエフェクトの更新
	UpdateVisualEffects();

	// 磁場エフェクトの更新
	if (boss) {
		// プレイヤーが吸引中 かつ ボスの供給ポイントが範囲内の場合に発動
		bool shouldActivate = boss->CanSupplyScrapToPlayer(vaccumPos_, vaccumRadius_);
		UpdateMagneticEffect(dt, boss->GetCenter(), shouldActivate);
	}
	else {
		UpdateMagneticEffect(dt, {}, false);
	}

	// 描画コンポーネントの更新
	UpdateDrawComponents(dt);
}

// ========================================
// 吸引位置の更新
// ========================================
void Player::UpdateVaccumPosition(Pad& pad) {
	Vector2 direction = { 0.0f, 0.0f };
	bool hasInput = false;

	// ゲームパッド入力の取得
	if (shared_->IsGamepadMode()) {
		float rightX = pad.RightX();
		float rightY = -pad.RightY(); // Y軸反転

		const float deadZone = 0.4f;
		if (std::abs(rightX) > deadZone || std::abs(rightY) > deadZone) {
			direction = { rightX, rightY };
			hasInput = true;
		}
	}

	// キーボード&マウス入力の取得
	if (!hasInput && shared_->IsKeyboardMouseMode()) {
		Vector2 mousePos = { static_cast<float>(mouseX_), static_cast<float>(mouseY_) };
		Vector2 toMouse = { mousePos.x - pos_.x, mousePos.y - pos_.y };
		float distance = std::sqrt(toMouse.x * toMouse.x + toMouse.y * toMouse.y);

		const float minDistance = 10.0f;
		if (distance > minDistance) {
			direction = { toMouse.x / distance, toMouse.y / distance };
			hasInput = true;
		}
	}

	// 入力がある場合は方向を正規化して記憶
	if (hasInput) {
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
		if (length > 0.01f) {
			direction.x /= length;
			direction.y /= length;
		}
		prevPadRightStickDirection_ = direction;
	}
	else {
		// 入力がない場合は前回の方向を使用
		direction = prevPadRightStickDirection_;

		// 初回起動時など、前回の方向が未設定の場合はデフォルト方向（右）
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
		if (length < 0.01f) {
			direction = { 1.0f, 0.0f };
			prevPadRightStickDirection_ = direction;
		}
	}

	// 吸引位置の計算
	vaccumPos_ = {
		pos_.x + direction.x * vaccumDistance_,
		pos_.y + direction.y * vaccumDistance_
	};

	// エイム方向に角度を更新
	angle_ = std::atan2(direction.y, direction.x);
}

// ========================================
// 吸引処理
// ========================================
void Player::UpdateSuction(ScrapManager* scrapManager, Pad& pad, Boss* boss) {

	if (!scrapManager) {
		return;
	}

	// 前フレームの状態を保存（UpdateFireで使用）
	wasSucking_ = isSucking_;

	// RT/SPACE押下中
	bool pressingRT = pad.RightTrigger() > 0.5f;
	isSucking_ = (Novice::CheckHitKey(DIK_SPACE) != 0) || pressingRT;

	if (isSucking_) {
		// 通常のスクラップ吸引処理
		scrapManager->ProcessSuction(vaccumPos_, vaccumRadius_, currentWeight_, maxWeight_);

		// ボスからのスクラップ吸収処理
		if (boss) {
			boss->ProcessPlayerSuction(vaccumPos_, vaccumRadius_, true);
		}

		// 重量更新
		currentWeight_ = scrapManager->GetHeldWeight();
	}
	else {
		// 吸引停止時に BeingSucked 状態のスクラップを解放
		scrapManager->ReleaseBeingSuckedScraps();

		// ボスにも吸引停止を通知
		if (boss) {
			boss->ProcessPlayerSuction(vaccumPos_, vaccumRadius_, false);
		}
	}
}

// ========================================
// 移動処理
// ========================================
void Player::UpdateMovement(float dt, const char* keys, Pad& pad) {
	dt; // 未使用

	Vector2 moveDir = { 0.0f, 0.0f };

	// キーボード入力
	if (keys[DIK_W]) moveDir.y -= 1.0f;
	if (keys[DIK_S]) moveDir.y += 1.0f;
	if (keys[DIK_A]) moveDir.x -= 1.0f;
	if (keys[DIK_D]) moveDir.x += 1.0f;

	// パッド左スティック入力
	float leftX = pad.LeftX();
	float leftY = -pad.LeftY(); // Y軸反転

	const float deadZone = 0.2f;
	if (std::abs(leftX) > deadZone || std::abs(leftY) > deadZone) {
		moveDir.x += leftX;
		moveDir.y += leftY;
	}

	// 正規化
	float length = std::sqrt(moveDir.x * moveDir.x + moveDir.y * moveDir.y);
	if (length > 0.01f) {
		moveDir.x /= length;
		moveDir.y /= length;
	}

	// 重量に応じた移動速度
	float speed = GetCurrentMoveSpeed();

	velocity_ = {
		moveDir.x * speed,
		moveDir.y * speed
	};

	// 端部処理
	if (pos_.x <= radius_) {
		pos_.x = radius_;
	}
	else if (pos_.x >= kWindowWidth - radius_) {
		pos_.x = kWindowWidth - radius_;
	}

	if (pos_.y <= radius_) {
		pos_.y = radius_;
	}
	else if (pos_.y >= kWindowHeight - radius_) {
		pos_.y = kWindowHeight - radius_;
	}
}

// ========================================
// 発射処理
// ========================================
void Player::UpdateFire(ScrapManager* scrapManager) {
	if (!scrapManager) {
		return;
	}

	bool releasedSuction = wasSucking_ && !isSucking_;

	if (releasedSuction && currentWeight_ > 0.0f) {
		Vector2 fireDirection = GetFireDirection();
		float spreadAngle = GetFireSpreadAngle();

		scrapManager->FireAllHeldScraps(fireDirection, kFireSpeed_, spreadAngle);

		// 反動のコントローラ振動の適用
		VibratePadShot(std::min(currentWeight_ / maxWeight_, 1.0f));

		// 音再生
		shared_->PlaySE_PlayerShot(currentWeight_ / maxWeight_);

		ApplyRecoil(fireDirection);

		// 発射エフェクトを適用（重量に応じた潰し伸ばし）
		float weightRatio = std::min(currentWeight_ / maxWeight_, 1.0f);
		Vector2 squashScale = {
			1.0f + (kSquashStretchScale_.x - 1.0f) * weightRatio,
			1.0f - (1.0f - kSquashStretchScale_.y) * weightRatio
		};
		drawCompShooting_.StartSquashStretch(squashScale, kSquashStretchDuration_, kSquashStretchCount_);

		currentWeight_ = 0.0f;

		// 発射状態に遷移
		isShooting_ = true;
		shootingStateTimer_ = kShootingStateDuration_;
	}
}

// ========================================
// 反動の更新
// ========================================
void Player::UpdateRecoil(float dt) {
	// 位置の反動：摩擦による減速
	recoilVelocity_.x *= kRecoilFriction_;
	recoilVelocity_.y *= kRecoilFriction_;

	// 十分小さくなったら0に
	if (std::abs(recoilVelocity_.x) < 0.1f) recoilVelocity_.x = 0.0f;
	if (std::abs(recoilVelocity_.y) < 0.1f) recoilVelocity_.y = 0.0f;

	// 角度の反動：減衰処理
	if (std::abs(recoilAngleOffset_) > 0.001f) {
		// 指数関数的に減衰
		recoilAngleOffset_ -= recoilAngleOffset_ * kRecoilAngleDecay_ * dt;

		// 十分小さくなったら0に
		if (std::abs(recoilAngleOffset_) < 0.001f) {
			recoilAngleOffset_ = 0.0f;
		}
	}

	// 反動速度を計算
	float recoilSpeed = sqrtf(recoilVelocity_.x * recoilVelocity_.x +
		recoilVelocity_.y * recoilVelocity_.y);

	// 一定速度以上で移動している場合、軌跡エフェクトを生成
	if (recoilSpeed >= trailEffectMinRecoilSpeed_) {
		trailEffectTimer_ += dt;

		if (trailEffectTimer_ >= trailEffectInterval_) {
			SpawnTrailEffect(pos_, recoilSpeed);
			trailEffectTimer_ = 0.0f;
		}
	}
}

// ========================================
// 反動の適応
// ========================================
void Player::ApplyRecoil(const Vector2& fireDirection) {
	// ウェイトの割合を計算（0.0～1.0）
	float weight_t = std::min(currentWeight_ / maxWeight_, 1.0f);

	// 反動距離を線形補間（最小値～最大値）
	float recoilDistance = kRecoilDistanceMin_ + (kRecoilDistanceMax_ - kRecoilDistanceMin_) * weight_t;

	// 反動速度を計算（加速度 × ウェイト比率）
	float recoilSpeed = recoilDistance * (kRecoilAcceleration_ / kRecoilDistanceMax_);

	recoilVelocity_ = {
		-fireDirection.x * recoilSpeed,
		-fireDirection.y * recoilSpeed
	};

	// 反動角度を計算（重量に応じて角度が大きくなる

	if (kUseRecoilAngle_) {
		float recoilAngle = kRecoilAngleMin_ + (kRecoilAngleMax_ - kRecoilAngleMin_) * weight_t;

		// 発射方向と逆方向に角度をオフセット
		// 発射方向ベクトルから垂直方向（反時計回り90度）を計算して適用
		recoilAngleOffset_ = -recoilAngle;  // マイナスで反対方向にのけぞる
	}
}

// ========================================
// ヘルパー関数
// ========================================

float Player::GetCurrentMoveSpeed() const {
	float weight_t = std::min(currentWeight_ / maxWeight_, 1.0f);
	float speed = kMoveSpeedAtZeroWeight_ + (kMoveSpeedAtMaxWeight_ - kMoveSpeedAtZeroWeight_) * weight_t;
	return speed;
}

float Player::GetFireSpreadAngle() const {
	float weight_t = std::min(currentWeight_ / maxWeight_, 1.0f);
	float angle = kFireSpreadAngleMin_ + (kFireSpreadAngleMax_ - kFireSpreadAngleMin_) * weight_t;
	return angle;
}

Vector2 Player::GetFireDirection() const {
	// vaccumPosの方向を発射方向とする
	Vector2 direction = { vaccumPos_.x - pos_.x, vaccumPos_.y - pos_.y };
	float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

	if (length > 0.01f) {
		// 正規化
		direction.x /= length;
		direction.y /= length;
	}
	else {
		// vaccumPosがプレイヤー位置と同じ場合はデフォルト方向（右方向）
		direction = { 1.0f, 0.0f };
	}

	return direction;
}

// ========================================
// プレイヤーの描画
// ========================================
void Player::Draw(const Vector2& scrollOffset) {


	// 軌跡エフェクトを先に描画
	DrawShotMovementEffects(scrollOffset);

	// 磁場エフェクトを描画
	DrawMagneticEffect(scrollOffset);

	// 状態に応じて描画を切り替え
	switch (drawState_) {
	case PlayerState::Normal:
		drawCompNormal_.DrawAnimationScreen();
		break;
	case PlayerState::Sucking:
		drawCompSucking_.DrawAnimationScreen();
		break;
	case PlayerState::Shooting:
		drawCompShooting_.DrawAnimationScreen();
		break;
	case PlayerState::DeadEffect:
		drawCompDeadEffect_.DrawAnimationScreen();
		return; // 死亡エフェクトの場合はデバッグ描画も行わない
	}

	scrollOffset;

#ifdef _DEBUG
	// デバッグ用：吸引範囲の可視化
	if (isSucking_) {
		Novice::DrawEllipse(
			static_cast<int>(vaccumPos_.x - scrollOffset.x),
			static_cast<int>(vaccumPos_.y - scrollOffset.y),
			static_cast<int>(vaccumRadius_),
			static_cast<int>(vaccumRadius_),
			0.0f, 0xFFFF0088, kFillModeWireFrame
		);

		// 吸引位置マーカー
		Novice::DrawEllipse(
			static_cast<int>(vaccumPos_.x - scrollOffset.x),
			static_cast<int>(vaccumPos_.y - scrollOffset.y),
			5, 5, 0.0f, 0xFF0000FF, kFillModeSolid
		);

		// プレイヤーから吸引位置への線
		Novice::DrawLine(
			static_cast<int>(pos_.x - scrollOffset.x),
			static_cast<int>(pos_.y - scrollOffset.y),
			static_cast<int>(vaccumPos_.x - scrollOffset.x),
			static_cast<int>(vaccumPos_.y - scrollOffset.y),
			0xFFFF00FF
		);
	}
	else {
		// 吸引位置マーカー
		Novice::DrawEllipse(
			static_cast<int>(vaccumPos_.x - scrollOffset.x),
			static_cast<int>(vaccumPos_.y - scrollOffset.y),
			5, 5, 0.0f, 0xFF0000FF, kFillModeSolid
		);

		// プレイヤーから吸引位置への線
		Novice::DrawLine(
			static_cast<int>(pos_.x - scrollOffset.x),
			static_cast<int>(pos_.y - scrollOffset.y),
			static_cast<int>(vaccumPos_.x - scrollOffset.x),
			static_cast<int>(vaccumPos_.y - scrollOffset.y),
			0xFFFF00FF
		);
	}
#endif
}

/// <summary>
/// 現在の重量割合を基に、画面上に重量ゲージを描画する
/// </summary>
void Player::DrawWeightGauge() {
	//// Guiでの表示位置調整
	//ImGui::Begin("WeightGauge");
	//// ウィンドウ位置を調整
	//ImGui::SliderFloat2("Position", &weightGaugePos_.x, 0.0f, 1920.0f);
	//// ウィンドウサイズを調整
	//ImGui::SliderFloat2("Size", &weightGaugeSize_.x, 50.0f, 800.0f);
	//ImGui::End();

	// 背景
	Novice::DrawBox(
		static_cast<int>(weightGaugePos_.x),
		static_cast<int>(weightGaugePos_.y),
		static_cast<int>(weightGaugeSize_.x),
		static_cast<int>(weightGaugeSize_.y),
		0.0f,
		0xFF444444,
		kFillModeSolid
	);

	// ゲージ本体
	float weightRatio = std::min(currentWeight_ / maxWeight_, 1.0f);
	int gaugeWidth = static_cast<int>(weightGaugeSize_.x * weightRatio);

	Novice::DrawBox(
		static_cast<int>(weightGaugePos_.x),
		static_cast<int>(weightGaugePos_.y),
		gaugeWidth,
		static_cast<int>(weightGaugeSize_.y),
		0.0f,
		0x00FFFFFF,
		kFillModeSolid
	);

	// 枠線
	Novice::DrawBox(
		static_cast<int>(weightGaugePos_.x),
		static_cast<int>(weightGaugePos_.y),
		static_cast<int>(weightGaugeSize_.x),
		static_cast<int>(weightGaugeSize_.y),
		0.0f,
		0xFFFFFFFF,
		kFillModeWireFrame
	);
}

void Player::DrawHitPointGauge() {
	// 配置用定数
	//// Guiでの表示位置調整
	//ImGui::Begin("HitPointGauge");
	//// ウィンドウ位置を調整
	//ImGui::SliderFloat2("Position", &hitPointGaugePos_.x, 0.0f, 1920.0f);
	//// ウィンドウサイズを調整
	//ImGui::SliderFloat2("Size", &hitPointGaugeSize_.x, 50.0f, 800.0f);

	//ImGui::End();

	// 背景
	Novice::DrawBox(
		static_cast<int>(hitPointGaugePos_.x),
		static_cast<int>(hitPointGaugePos_.y),
		static_cast<int>(hitPointGaugeSize_.x),
		static_cast<int>(hitPointGaugeSize_.y),
		0.0f,
		0xFF444444,
		kFillModeSolid
	);

	// ゲージ本体
	float hpRatio = (float)hitPoint_ / (float)kMaxHitPoint_;
	int filledWidth = static_cast<int>(hitPointGaugeSize_.x * hpRatio);
	Novice::DrawBox(
		static_cast<int>(hitPointGaugePos_.x),
		static_cast<int>(hitPointGaugePos_.y),
		filledWidth,
		static_cast<int>(hitPointGaugeSize_.y),
		0.0f,
		0x00FF00FF,
		kFillModeSolid
	);

	// 枠線
	Novice::DrawBox(
		static_cast<int>(hitPointGaugePos_.x),
		static_cast<int>(hitPointGaugePos_.y),
		static_cast<int>(hitPointGaugeSize_.x),
		static_cast<int>(hitPointGaugeSize_.y),
		0.0f,
		0xFFFFFFFF,
		kFillModeWireFrame
	);
}

void Player::VibratePadShot(float shotRatio) {
	if (pad_) {
		if (shared_->GetInputMode() == GameShared::InputMode::Pad) {
			pad_->StartVibration(2.5f * shotRatio, 2.5f * shotRatio, (int)(40 * shotRatio));
		}
	}
}


// ========================================
// ショット移動エフェクト関連
// ========================================

void Player::InitializeTrailEffect() {
	// エフェクト用の画像を読み込み
	trailEffectGraphHandle_ = Novice::LoadTexture("./Resources/images/tomo/player_particle.png");
	trailEffectColor_ = 0xAAFFFFFF; // 半透明の白
	trailEffectTimer_ = 0.0f;
}

void Player::UpdateShotMovementEffects(float dt) {
	// 全エフェクトを更新
	for (auto& effect : drawCompShotMovementEffect_) {
		if (effect) {
			effect->UpdateEffects(dt);
		}
	}

	// 完了したエフェクトを削除
	RemoveFinishedTrailEffects();
}

void Player::SpawnTrailEffect(const Vector2& position, float recoilSpeed) {
	// リコイル速度に応じた基本スケールを計算（0.5f～1.0f）
	float speedRatio = recoilSpeed / trailEffectMaxRecoilSpeed_;
	speedRatio = std::clamp(speedRatio, 0.0f, 1.0f);
	float baseScale = trailEffectMinScale_ + (trailEffectMaxScale_ - trailEffectMinScale_) * speedRatio;

	// 指定個数のエフェクトを生成
	for (int i = 0; i < trailEffectSpawnCount_; ++i) {
		// ランダムオフセットを計算
		float offsetX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * trailEffectRandomOffset_;
		float offsetY = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * trailEffectRandomOffset_;

		Vector2 spawnPos = {
			position.x + offsetX,
			position.y + offsetY
		};

		// 個別のスケールにランダムな変動を加える（オプション）
		float scaleVariation = 0.8f + ((float)rand() / RAND_MAX * 0.4f); // 0.8～1.2倍
		float finalScale = baseScale * scaleVariation;

		SpawnSingleTrailEffect(spawnPos, finalScale);
	}
}

void Player::SpawnSingleTrailEffect(const Vector2& position, float scale) {
	// 最大数を超えている場合は古いものから削除
	if (drawCompShotMovementEffect_.size() >= kMaxTrailEffects_) {
		drawCompShotMovementEffect_.erase(drawCompShotMovementEffect_.begin());
	}

	// 新しいエフェクトを生成
	auto effect = std::make_unique<DrawComponent2D>(
		position,
		moveEffectSize_,
		moveEffectSize_,
		trailEffectGraphHandle_,
		800, 800, 1, 1, 0.1f, false,
		trailEffectColor_
	);

	// 縮小消滅エフェクトを開始
	effect->StartScaleExtinction(
		scale * trailEffectStartScale_,
		trailEffectDuration_,
		trailEffectEasingType_,
		trailEffectFadeWithScale_
	);

	drawCompShotMovementEffect_.push_back(std::move(effect));
}


void Player::RemoveFinishedTrailEffects() {
	// 完了したエフェクトを削除
	drawCompShotMovementEffect_.erase(
		std::remove_if(
			drawCompShotMovementEffect_.begin(),
			drawCompShotMovementEffect_.end(),
			[](const std::unique_ptr<DrawComponent2D>& effect) {
				return effect->IsScaleExtinctionFinished();
			}
		),
		drawCompShotMovementEffect_.end()
	);
}

void Player::DrawShotMovementEffects(const Vector2& scrollOffset) {

	Novice::SetBlendMode(kBlendModeAdd);
	for (const auto& effect : drawCompShotMovementEffect_) {
		if (effect) {
			effect->DrawAnimationScreen(scrollOffset);
		}
	}
	Novice::SetBlendMode(kBlendModeNormal);
}

void Player::InitializeMagneticEffect() {
	magneticEffectGraphHandle_ = Novice::LoadTexture("./Resources/images/tomo/magneticFieldEffects_ver1.png");
	magneticEffectBaseColor_ = 0xFFFFFFFF; // 青白い色
	magneticEffectAlpha_ = 0.7f;
	magneticIsActive_ = false;
	magneticIsFadingIn_ = false;
	magneticIsFadingOut_ = false;
	magneticFadeTimer_ = 0.0f;
	magneticWaveTimer_ = 0.0f;
	magneticSegments_.clear();
}

// ========================================
// 磁場エフェクト
// ========================================

void Player::UpdateMagneticEffect(float dt, const Vector2& bossPos, bool isActive) {

	if (isActive) {
		// 範囲内: タイマーリセット
		if (magneticIsExpiring_) {
			magneticIsExpiring_ = false;
			magneticLifespanTimer_ = magneticLifespanDuration_;
		}

		// エフェクト表示
		if (!magneticIsActive_) {
			magneticIsActive_ = true;
			magneticLifespanTimer_ = magneticLifespanDuration_;
			// 初回のみセグメント生成
			GenerateInitialMagneticSegments();
		}
	}
	else {
		// 範囲外: タイマー開始
		if (magneticIsActive_ && !magneticIsExpiring_) {
			magneticIsExpiring_ = true;
			magneticLifespanTimer_ = magneticLifespanDuration_;
		}
	}

	// タイマー更新
	if (magneticIsExpiring_) {
		magneticLifespanTimer_ -= dt;
		if (magneticLifespanTimer_ <= 0.0f) {
			ClearMagneticEffect();
			return;
		}
	}

	// 非アクティブなら何もしない
	if (!magneticIsActive_) {
		return;
	}

	// ゆらぎタイマー更新
	if (magneticEnableWave_) {
		magneticWaveTimer_ += dt * magneticWaveSpeed_;
	}

	// ベジェ曲線の制御点を計算
	Vector2 start = pos_;
	Vector2 end = bossPos;
	Vector2 midPoint = {
		(start.x + end.x) * 0.5f + magneticCurveOffsetX_,
		(start.y + end.y) * 0.5f + magneticCurveOffsetY_
	};

	// ゆらぎを適用
	if (magneticEnableWave_) {
		float waveOffset = sinf(magneticWaveTimer_) * magneticWaveAmplitude_;
		midPoint.y += waveOffset;
	}

	// セグメントの位置と角度を更新（再生成せず）
	UpdateMagneticSegmentPositions(start, end, midPoint);

	// アニメーション更新
	for (auto& segment : magneticSegments_) {
		if (segment.drawComp) {
			segment.drawComp->UpdateAnimation(dt, segment.position, segment.angle, { 1.0f, 1.0f });
		}
	}
}

void Player::GenerateMagneticSegments(const Vector2& start, const Vector2& end, const Vector2& control) {
	magneticSegments_.clear();

	// 距離を計算してセグメント数を決定
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float distance = sqrtf(dx * dx + dy * dy);

	if (distance < 1.0f) return;

	// セグメント数を計算
	int segmentCount = static_cast<int>(distance / magneticSegmentInterval_);
	segmentCount = std::max(1, std::min(segmentCount, 50)); // 最大50個

	// ベジェ曲線上にセグメントを配置
	for (int i = 0; i < segmentCount; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(segmentCount);

		// 位置を計算
		Vector2 pos = CalculateBezierPoint(start, control, end, t);

		// 接線方向を計算して角度を求める
		Vector2 tangent = CalculateBezierTangent(start, control, end, t);
		float angle = atan2f(tangent.y, tangent.x) + 1.5708f; // +π/2で90度回転

		// セグメントを作成
		MagneticSegment segment;
		segment.position = pos;
		segment.angle = angle;
		segment.alpha = 1.0f;

		segment.drawComp = std::make_unique<DrawComponent2D>(
			pos,
			magneticSegmentWidth_,
			magneticSegmentHeight_,
			magneticEffectGraphHandle_,
			32, 64, 3, 3, 0.1f, true,
			magneticEffectBaseColor_
		);

		magneticSegments_.push_back(std::move(segment));
	}
}

Vector2 Player::CalculateBezierPoint(const Vector2& p0, const Vector2& p1, const Vector2& p2, float t) {
	// 2次ベジェ曲線: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
	float u = 1.0f - t;
	float tt = t * t;
	float uu = u * u;

	Vector2 result;
	result.x = uu * p0.x + 2.0f * u * t * p1.x + tt * p2.x;
	result.y = uu * p0.y + 2.0f * u * t * p1.y + tt * p2.y;

	return result;
}

Vector2 Player::CalculateBezierTangent(const Vector2& p0, const Vector2& p1, const Vector2& p2, float t) {
	// 接線ベクトル: B'(t) = 2(1-t)(P1-P0) + 2t(P2-P1)
	float u = 1.0f - t;

	Vector2 tangent;
	tangent.x = 2.0f * u * (p1.x - p0.x) + 2.0f * t * (p2.x - p1.x);
	tangent.y = 2.0f * u * (p1.y - p0.y) + 2.0f * t * (p2.y - p1.y);

	// 正規化
	float length = sqrtf(tangent.x * tangent.x + tangent.y * tangent.y);
	if (length > 0.01f) {
		tangent.x /= length;
		tangent.y /= length;
	}

	return tangent;
}

void Player::UpdateMagneticFade(float dt) {
	if (magneticIsFadingIn_) {
		magneticFadeTimer_ += dt;
		if (magneticFadeTimer_ >= magneticFadeDuration_) {
			magneticFadeTimer_ = magneticFadeDuration_;
			magneticIsFadingIn_ = false;

#ifdef _DEBUG
			Novice::ConsolePrintf("Magnetic: FADE IN COMPLETE\n");
#endif
		}
	}
	else if (magneticIsFadingOut_) {
		magneticFadeTimer_ += dt;
		if (magneticFadeTimer_ >= magneticFadeDuration_) {
			magneticFadeTimer_ = magneticFadeDuration_;

#ifdef _DEBUG
			Novice::ConsolePrintf("Magnetic: FADE OUT COMPLETE\n");
#endif
		}
	}
}

void Player::ClearMagneticEffect() {
#ifdef _DEBUG
	Novice::ConsolePrintf("ClearMagneticEffect: Clearing %zu segments\n", magneticSegments_.size());
#endif

	magneticSegments_.clear();
	magneticIsActive_ = false;
	magneticIsFadingIn_ = false;
	magneticIsFadingOut_ = false;
	magneticFadeTimer_ = 0.0f;
}

void Player::DrawMagneticEffect(const Vector2& scrollOffset) {

	// セグメントが空、または非アクティブなら描画しない
	if (magneticSegments_.empty() || !magneticIsActive_) {
		return;
	}

	// タイマー中は透明度を減少
	float currentAlpha = magneticEffectAlpha_;
	if (magneticIsExpiring_ && magneticLifespanDuration_ > 0.0f) {
		// 残り時間の割合を計算（1.0 → 0.0）
		float ratio = magneticLifespanTimer_ / magneticLifespanDuration_;
		ratio = std::clamp(ratio, 0.0f, 1.0f);

		// 透明度を線形補間
		currentAlpha = magneticEffectAlpha_ * ratio;

#ifdef _DEBUG
		// 透明度変化のデバッグログ（60フレームごと）
		static int alphaLogCounter = 0;
		if (++alphaLogCounter % 60 == 0) {
			Novice::ConsolePrintf("Magnetic Alpha: %.3f (Ratio: %.3f)\n", currentAlpha, ratio);
		}
#endif
	}

	// 加算合成モードで描画
	Novice::SetBlendMode(kBlendModeAdd);

	for (auto& segment : magneticSegments_) {
		if (!segment.drawComp) continue;

		// 透明度を適用した色を計算
		unsigned int alphaValue = static_cast<unsigned int>(currentAlpha * 255.0f);
		unsigned int color = (magneticEffectBaseColor_ & 0xFFFFFF00) | alphaValue;

		segment.drawComp->graph_.color = color;
		segment.drawComp->position_ = segment.position;
		segment.drawComp->angle_ = segment.angle;

		segment.drawComp->DrawAnimationScreen(scrollOffset);
	}

	Novice::SetBlendMode(kBlendModeNormal);
}

void Player::GenerateInitialMagneticSegments() {
	magneticSegments_.clear();

	// 初期位置での距離計算
	Vector2 start = pos_;
	Vector2 end = pos_; // 初期状態では同じ位置

	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float distance = sqrtf(dx * dx + dy * dy);

	// 初期セグメント数（後で動的に調整）
	int segmentCount = std::max(10, static_cast<int>(distance / magneticSegmentInterval_));
	segmentCount = std::min(segmentCount, 50);

	for (int i = 0; i < segmentCount; ++i) {
		MagneticSegment segment;
		segment.position = start;
		segment.angle = 0.0f;
		segment.alpha = 1.0f;

		segment.drawComp = std::make_unique<DrawComponent2D>(
			start,
			magneticSegmentWidth_,
			magneticSegmentHeight_,
			magneticEffectGraphHandle_,
			32, 64, 3, 3, 0.1f, true,
			magneticEffectBaseColor_
		);

		magneticSegments_.push_back(std::move(segment));
	}
}

void Player::UpdateMagneticSegmentPositions(const Vector2& start, const Vector2& end, const Vector2& control) {
	if (magneticSegments_.empty()) return;

	int segmentCount = static_cast<int>(magneticSegments_.size());

	for (int i = 0; i < segmentCount; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(segmentCount);

		// ベジェ曲線上の位置を計算
		Vector2 pos = CalculateBezierPoint(start, control, end, t);

		// 接線方向から角度を計算
		Vector2 tangent = CalculateBezierTangent(start, control, end, t);
		float angle = atan2f(tangent.y, tangent.x) + 1.5708f;

		// セグメントの位置と角度を更新
		magneticSegments_[i].position = pos;
		magneticSegments_[i].angle = angle;
		magneticSegments_[i].drawComp->position_ = pos;
		magneticSegments_[i].drawComp->angle_ = angle;
	}
}