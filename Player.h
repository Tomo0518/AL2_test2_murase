#pragma once
#include "Vector2.h"
#include <algorithm>
#include "DrawComponent2D.h"
#include "Pad.h"
#include <vector>
#include <memory>

#undef min

class Pad;
class GameShared;
class ScrapManager;
class Boss;

enum class PlayerState {
	Normal,
	Sucking,
	Shooting,
	DeadEffect
};

class Player {
	friend class DebugWindow;
public:
	Player(Pad& pad, GameShared& shared);

	void Update(float dt, const char* keys, const char* preKeys, const Vector2& offset, Pad& pad, ScrapManager* scrapManager = nullptr, Boss* boss = nullptr);
	void Draw(const Vector2& scrollOffset);

	// Getter
	Vector2 GetPosition() const { return pos_; }
	Vector2 GetVaccumPos() const { return vaccumPos_; }
	float   GetRadius() const { return radius_; }
	float   GetVaccumRadius() const { return vaccumRadius_; }
	float   GetCurrentWeight() const { return currentWeight_; }
	float   GetMaxWeight() const { return maxWeight_; }
	float   GetCurrentMoveSpeed() const;
	bool    IsAlive() const { return isAlive_; }
	bool    IsSucking() const { return isSucking_; }
	PlayerState GetDrawState() const { return drawState_; }
	int    GetHitPoint() const { return hitPoint_; }

	// Setter
	void SetPosition(const Vector2& pos) { pos_ = pos; }
	void SetAlive(bool alive) { isAlive_ = alive; }
	void SetHitPoint(int hp) { hitPoint_ = std::clamp(hp, 0, kMaxHitPoint_); }

	void TakeDamage(int damage) {
		hitPoint_ -= damage;
		if (hitPoint_ <= 0) {
			hitPoint_ = 0;
			isAlive_ = false;
		}
	}

	// スクラップ関連
	void UpdateWeight(float weight) { currentWeight_ = weight; }

	// 所持中のスクラップの割合を棒ゲージで描画
	Vector2 weightGaugePos_{ 25.0f, 645.0f };
	Vector2 weightGaugeSize_{ 300.0f, 50.0f };
	void DrawWeightGauge();

	// ヒットポイントの割合を棒ゲージで描画
	Vector2 hitPointGaugePos_{ 25.0f, 25.0f };
	Vector2 hitPointGaugeSize_{ 300.0f, 30.0f };
	void DrawHitPointGauge();

	// バイブレーション
	void VibratePadShot(float shotRatio);

private:
	Vector2 pos_{};
	Vector2 velocity_{};
	Vector2 recoilVelocity_{};
	float   radius_ = 32.0f;
	float   size_ = 64.0f;
	float   angle_ = 0.0f;
	float   recoilAngleOffset_ = 0.0f;  // 描画用の反動角度オフセット
	Vector2 scale_{ 1.0f, 1.0f };
	bool    isAlive_ = true;
	bool    wasAlive_ = true;

	int hitPoint_ = 100;
	int kMaxHitPoint_ = 100;

	//　描画関連
	PlayerState drawState_ = PlayerState::Normal;
	PlayerState prevDrawState_ = PlayerState::Normal;
	DrawComponent2D drawCompNormal_;	// 通常時
	DrawComponent2D drawCompSucking_;	// 吸引時
	DrawComponent2D drawCompShooting_;	// 発射時
	DrawComponent2D drawCompDeadEffect_;// 死亡エフェクト時

	GameShared* shared_ = nullptr;
	Pad* pad_ = nullptr;

	// ========================================
	// スクラップシステム関連
	// ========================================
	Vector2 vaccumPos_{};
	float   vaccumRadius_ = 150.0f;
	float   vaccumDistance_ = 100.0f;

	float   currentWeight_ = 0.0f;
	float   maxWeight_ = 25.0f;

	bool    isSucking_ = false;
	bool    wasSucking_ = false;
	bool    isShooting_ = false;

	int     mouseX_ = 0;
	int     mouseY_ = 0;

	// ========================================
	// 調整可能なパラメータ（通常のメンバ変数に変更）
	// ========================================e
	// 移動速度
	float kNormalMoveSpeed_ = 300.0f;
	float kRunMoveSpeed_ = 450.0f;
	float kMoveSpeedAtZeroWeight_ = 300.0f;
	float kMoveSpeedAtMaxWeight_ = 150.0f;

	// 吸引関連
	float kVaccumDistance_ = 100.0f;
	float kVaccumRadius_ = 150.0f;

	// 発射関連
	float kFireSpreadAngleMin_ = 15.0f;
	float kFireSpreadAngleMax_ = 25.0f;
	float kFireSpeed_ = 820.0f;
	float kShootingStateDuration_ = 0.2f; // 発射状態の持続時間

	// 反動関連
	float kRecoilDistanceMin_ = 50.0f;		// 最小重量時の反動距離
	float kRecoilDistanceMax_ = 350.0f;		// 最大重量時の反動距離
	float kRecoilAcceleration_ = 2000.0f;	// 反動加速度
	float kRecoilFriction_ = 0.90f;			// 反動摩擦（0.0f～1.0f、1.0fに近いほど減衰が遅い）

	// 反動角度関連
	bool  kUseRecoilAngle_ = true;      // 反動角度を使用するかどうか
	float kRecoilAngleMin_ = 0.2f;          // 最小重量時の反動角度（ラジアン、約11度）
	float kRecoilAngleMax_ = 0.785f;        // 最大重量時の反動角度（ラジアン、約45度）
	float kRecoilAngleDecay_ = 8.0f;        // 角度減衰速度（高いほど早く戻る）

	// ビジュアルエフェクト関連
	float kShakeRangeMax_ = 5.0f;          // 最大重量時のシェイク範囲
	float kSquashStretchDuration_ = 0.15f; // 潰し伸ばしの持続時間
	Vector2 kSquashStretchScale_ = { 2.0f, 0.5f }; // 潰し伸ばしのスケール（X横、Y縦）
	int kSquashStretchCount_ = 1;          // 潰し伸ばしの回数

	// 発射状態タイマー
	float shootingStateTimer_ = 0.0f;

	// 初期化（リスポーン時に使用）
	void Initialize();

	// 描画コンポーネントの初期化
	void InitializeDrawComponents();

	// 描画コンポーネントの更新
	void UpdateDrawComponents(float dt);

	// アニメーションのリセット
	void ResetAnimations();

	// 描画状態の更新
	void UpdateDrawState();

	// ビジュアルエフェクトの更新
	void UpdateVisualEffects();

	// スクラップ関連の更新処理
	void UpdateVaccumPosition(Pad& pad);

	// 吸引、発射、移動、反動の各更新処理
	void UpdateSuction(ScrapManager* scrapManager, Pad& pad, Boss* boss = nullptr);
	void UpdateFire(ScrapManager* scrapManager);
	void UpdateMovement(float dt, const char* keys, Pad& pad);
	void UpdateRecoil(float dt);

	// ヘルパー関数
	Vector2 GetFireDirection() const;
	float GetFireSpreadAngle() const;
	void ApplyRecoil(const Vector2& fireDirection);

	// 方向を保持するためにパッドの前回のスティックの値を保存
	Vector2 prevPadRightStickDirection_{};


	// ========================================
	// 軌跡エフェクト関連
	// ========================================
private:

	// 発射移動の軌跡に残るエフェクトたち
	std::vector<std::unique_ptr<DrawComponent2D>> drawCompShotMovementEffect_;

	float   moveEffectSize_ = 100.0f;

	// エフェクト生成パラメータ（DebugWindowで調整可能）
	float trailEffectInterval_ = 0.05f;        // エフェクト生成間隔（秒）
	float trailEffectTimer_ = 0.0f;            // 生成タイマー
	float trailEffectDuration_ = 0.8f;         // エフェクトの消滅時間（秒）
	float trailEffectStartScale_ = 1.0f;       // エフェクトの初期スケール
	int trailEffectEasingType_ = 1;            // イージングタイプ（0:Linear, 1:EaseOut, 2:EaseIn, 3:EaseInOut）
	bool trailEffectFadeWithScale_ = true;     // スケールと同時にフェード
	float trailEffectMinRecoilSpeed_ = 50.0f;  // エフェクト生成する最低反動速度
	int trailEffectGraphHandle_ = -1;          // エフェクト用の画像ハンドル
	unsigned int trailEffectColor_ = 0xFFFFFFFF; // エフェクトの色
	int trailEffectSpawnCount_ = 2;          // 1回の生成で出す個数
	float trailEffectRandomOffset_ = 20.0f;  // ランダムオフセット範囲（px）
	float trailEffectMinScale_ = 0.25f;       // 最小スケール（低速時）
	float trailEffectMaxScale_ = 1.0f;       // 最大スケール（高速時）
	float trailEffectMaxRecoilSpeed_ = 500.0f; // スケール計算用の最大速度

	static constexpr int kMaxTrailEffects_ = 100; // 最大エフェクト数
	// 軌跡エフェクトを生成
	void SpawnTrailEffect(const Vector2& position, float recoilSpeed);
	void SpawnSingleTrailEffect(const Vector2& position, float scale);

	// 完了したエフェクトを削除
	void RemoveFinishedTrailEffects();

public:
	// 軌跡エフェクト用メソッド
	void DrawShotMovementEffects(const Vector2& scrollOffset);
	void UpdateShotMovementEffects(float dt);
	void InitializeTrailEffect();

private:
	// ========================================
	// 磁場エフェクト関連
	// ========================================

	// 磁場エフェクトの各セグメント
	struct MagneticSegment {
		std::unique_ptr<DrawComponent2D> drawComp;
		Vector2 position;
		float angle;
		float alpha;  // 透明度（フェード用）
	};

	std::vector<MagneticSegment> magneticSegments_;
	std::vector<DrawComponent2D> magneticSegmentComps_;

	// 磁場エフェクトのパラメータ（GUI調整可能）
	int magneticEffectGraphHandle_ = -1;
	float magneticSegmentWidth_ = 32.0f;        // セグメントの幅
	float magneticSegmentHeight_ = 64.0f;       // セグメントの高さ
	float magneticSegmentInterval_ = 24.0f;     // セグメント間隔（px）
	unsigned int magneticEffectBaseColor_ = 0x00FFFFFF; // 基本色（青白）
	float magneticEffectAlpha_ = 0.7f;          // 基本透明度（0.0～1.0）

	// ベジェ曲線の制御点オフセット
	float magneticCurveOffsetY_ = -100.0f;      // 中間点のY軸オフセット
	float magneticCurveOffsetX_ = 0.0f;         // 中間点のX軸オフセット

	// 動的エフェクト（ゆらぎ）
	bool magneticEnableWave_ = false;           // ゆらぎを有効にするか
	float magneticWaveAmplitude_ = 10.0f;       // ゆらぎの振幅（px）
	float magneticWaveSpeed_ = 2.0f;            // ゆらぎの速度
	float magneticWaveTimer_ = 0.0f;            // ゆらぎのタイマー

	// フェード関連
	bool magneticIsActive_ = false;             // エフェクトが発動中か
	bool magneticIsFadingIn_ = false;           // フェードイン中
	bool magneticIsFadingOut_ = false;          // フェードアウト中
	float magneticFadeDuration_ = 0.3f;         // フェードの時間（秒）
	float magneticFadeTimer_ = 0.0f;            // フェードタイマー

	// 磁場エフェクトの寿命システム
	float magneticLifespanDuration_ = 0.3f;   // タイマー時間（秒）
	float magneticLifespanTimer_ = 0.0f;      // 残り時間
	bool magneticIsExpiring_ = false;         // タイマー作動中か

	// 接続先の選択
	bool magneticConnectToSupplyPoints_ = false; // true: 各供給ポイントへ, false: ボス中心へ

	// 内部メソッド
	void InitializeMagneticEffect();
	void UpdateMagneticEffect(float dt, const Vector2& bossPos, bool isActive);
	void GenerateMagneticSegments(const Vector2& start, const Vector2& end, const Vector2& control);
	Vector2 CalculateBezierPoint(const Vector2& p0, const Vector2& p1, const Vector2& p2, float t);
	Vector2 CalculateBezierTangent(const Vector2& p0, const Vector2& p1, const Vector2& p2, float t);
	void UpdateMagneticFade(float dt);
	void ClearMagneticEffect();

	void GenerateInitialMagneticSegments();
	void UpdateMagneticSegmentPositions(const Vector2& start, const Vector2& end, const Vector2& control);

public:
	void DrawMagneticEffect(const Vector2& scrollOffset);
};