#pragma once
#include "Scrap.h"
#include <vector>
#include <memory>
#include <random>

class Player;

enum class ScrapGanerateMode {
	None,
	Circle,
	Random,
	Explosion
};

enum class ScrapGenerateSize {
	SmallAndMedium,
	SmallAndLarge,
	MediumAndLarge,
	SmallAndMediumAndLarge,
};

class ScrapManager {
	friend class DebugWindow;
public:
	ScrapManager();
	~ScrapManager() = default;

	void Initialize();
	void Update(float dt, const Vector2& vaccumPos, bool isSucking);
	void Draw(const Vector2& scrollOffset);

	// ========================================
	// スクラップ生成系
	// ========================================

	/// <summary>
	/// 指定位置にスクラップを生成（ボスの供給ポイント用）
	/// </summary>
	/// <param name="type">スクラップの種類</param>
	/// <param name="position">生成位置</param>
	/// <param name="initialVelocity">初速度</param>
	void SpawnScrap(ScrapType type, const Vector2& position, const Vector2& initialVelocity);

	// 円形にスクラップを生成
	void SpawnScrapCircle(const Vector2& center, int count, float radius, ScrapType type, float spreadSpeed = 100.0f);

	// ランダム位置にスクラップを生成
	void SpawnScrapRandom(const Vector2& center, int count, float minRadius, float maxRadius, ScrapType type);

	// 爆発的にスクラップを生成
	void SpawnScrapExplosion(const Vector2& center, int count, ScrapType type, float explosionForce = 200.0f);

	// 種類別にスクラップを生成（新しい爆発生成メソッド）
	void SpawnScrapExplosionKinds(const Vector2& center, int maxCount, int bigSizeCount, ScrapGenerateSize size, float explosionForce = 200.0f, int midSizeCount = 0);

	// 吸引処理
	void ProcessSuction(const Vector2& vaccumPos, float vaccumRadius, float playerWeight, float maxWeight);

	// ========================================
	// ボスのスクラップ管理用
	// ========================================

	// ボス移動時のスクラップ生成用フレームカウンター
	int bossMoveSpawnFrameCounter_ = 0;

	/// <summary>
	/// ボスの移動時にスクラップを生成（移動中に継続的に生成）
	/// </summary>
	/// <param name="isMoveGenerateScrap">生成フラグ（移動中true）</param>
	/// <param name="bossCenter">ボスの中心座標</param>
	/// <param name="bossRadius">ボスの当たり判定半径（デフォルト: 160.0f）</param>
	/// <param name="spawnInterval">生成間隔（フレーム数、デフォルト: 10フレームに1回）</param>
	/// <param name="spawnCountPerInterval">1回あたりの生成数（デフォルト: 2-3個）</param>
	/// <param name="outwardSpeed">外側への初速度（デフォルト: 80.0f）</param>
	void SpawnBossScrapMove(
		bool isMoveGenerateScrap,
		const Vector2& bossCenter,
		float bossRadius = 160.0f,
		int spawnInterval = 10,
		int spawnCountPerInterval = 3,
		float outwardSpeed = 80.0f
	);

	/// <summary>
	/// ボスのパンチ攻撃時にスクラップを爆発的に生成
	/// </summary>
	/// <param name="bossPunchPos">パンチの衝突位置</param>
	/// <param name="maxCount">総スクラップ数（デフォルト: 15個）</param>
	/// <param name="bigSizeCount">Large サイズの数（デフォルト: 2個）</param>
	/// <param name="size">サイズ組み合わせ（デフォルト: SmallAndLarge）</param>
	/// <param name="explosionForce">爆発力（デフォルト: 250.0f）</param>
	/// <param name="midSizeCount">Medium サイズの数（3サイズ混合時のみ、デフォルト: 0）</param>
	void SpawnBossScrapPunch(
		const Vector2& bossPunchPos,
		int maxCount = 15,
		int bigSizeCount = 2,
		ScrapGenerateSize size = ScrapGenerateSize::SmallAndLarge,
		float explosionForce = 250.0f,
		int midSizeCount = 0
	);

	/// <summary>
	/// ボスのビーム軌道上にスクラップを生成
	/// </summary>
	/// <param name="startPos">ビーム開始位置</param>
	/// <param name="endPos">ビーム終了位置</param>
	/// <param name="width">ビームの幅（デフォルト: 128.0f）</param>
	/// <param name="maxCount">軌道上の総スクラップ数（デフォルト: 15個）</param>
	/// <param name="size">サイズ組み合わせ（デフォルト: SmallAndMedium）</param>
	/// <param name="randomVelocityRange">ランダムな初速度の範囲（デフォルト: 100.0f）</param>
	void SpawnBossScrapBeam(
		const Vector2& startPos,
		const Vector2& endPos,
		float width = 128.0f,
		int maxCount = 15,
		ScrapGenerateSize size = ScrapGenerateSize::SmallAndMedium,
		float randomVelocityRange = 100.0f
	);

	/// <summary>
	/// ボスのタックル時にスクラップを大量生成（※仕様確定後に実装）
	/// </summary>
	/// <param name="isTackleDone">タックル完了フラグ</param>
	/// <param name="bossCenterPos">ボスの中心座標</param>
	void SpawnBossScrapTackle(
		bool isTackleDone,
		const Vector2& bossCenterPos
	);

	// 吸引停止時に BeingSucked 状態のスクラップを解放
	void ReleaseBeingSuckedScraps();

	// 保持中のスクラップを取得
	std::vector<Scrap*> GetHeldScraps();
	float GetHeldWeight() const { return heldWeight_; }
	int GetHeldCount() const { return heldCount_; }

	// 保持中のスクラップ数に応じた最大半径を計算
	float CalculateMaxHeldRadius(int heldCount) const;

	// 発射処理
	void FireAllHeldScraps(const Vector2& fireDirection, float fireSpeed, float spreadAngle);

	// スクラップのクリア
	void ClearAll();

	// 非アクティブなスクラップのクリア
	void ClearInactive();

	// 画面外判定
	void RemoveOutOfBoundsScraps(const Vector2& screenSize, float margin = 200.0f);

	// スクラップ配列を取得
	std::vector<std::unique_ptr<Scrap>>& GetScraps() { return scraps_; }

	// ========================================
	// デバッグ用
	// ========================================

	int GetActiveScrapsCount() const;
	int GetFreeScrapsCount() const;

	/// <summary>
	/// デバッグ入力処理（マウスクリックでスクラップ生成など）
	/// </summary>
	/// <param name="mousePos">マウス座標</param>
	/// <param name="keys">現在のキー入力状態</param>
	/// <param name="preKeys">前フレームのキー入力状態</param>
	void HandleDebugInput(const Vector2& mousePos, const char* keys, const char* preKeys);

	/// <summary>
	/// デバッグ用の可視化描画（吸引範囲など）
	/// </summary>
	/// <param name="mousePos">マウス座標</param>
	/// <param name="scrollOffset">スクロールオフセット</param>
	void DrawDebugVisualization(const Vector2& mousePos, const Vector2& scrollOffset);

	// デバッグパラメータのGetter
	bool IsUsingNewExplosionMethod() const { return useNewExplosionMethod_; }
	float GetDebugSpawnRadius() const { return debugSpawnRadius_; }

	enum class SpawnPattern {
		Circle,
		Random,
		Explosion
	};

private:
	// スクラップ配列
	std::vector<std::unique_ptr<Scrap>> scraps_;
	// 乱数生成器
	std::mt19937 randomEngine_;

	// 保持中のスクラップ情報
	float heldWeight_ = 0.0f;
	int heldCount_ = 0;

	// スクラップ生成の内部処理
	Scrap* CreateScrap(ScrapType type, ScrapTrait trait, const Vector2& position, const Vector2& velocity);

	// 重複回避処理
	bool IsOverlapping(const Vector2& position, float radius, const std::vector<Vector2>& existingPositions, const std::vector<float>& existingRadii);

	// 重複しない位置を探索
	Vector2 FindNonOverlappingPosition(const Vector2& basePosition, float radius, float searchRadius, const std::vector<Vector2>& existingPositions, const std::vector<float>& existingRadii, int maxAttempts = 30);

	// 吸引中・保持中の衝突処理
	void ResolveCollisions(const Vector2& vaccumPos);

	// 保持中のスクラップを整列
	void ArrangeHeldScraps(const Vector2& vaccumPos);

	// ========================================
	// デバッグ用パラメータ
	// ========================================

	// 共通パラメータ
	int debugSpawnCount_ = 10;                        // 生成するスクラップ数
	float debugSpawnRadius_ = 100.0f;                 // 円の半径
	float debugSpreadSpeed_ = 100.0f;                 // 広がる速度
	ScrapType debugScrapType_ = ScrapType::Small;     // 生成するスクラップタイプ
	float debugExplosionForce_ = 200.0f;              // 爆発力

	// 新しい爆発生成用パラメータ
	bool useNewExplosionMethod_ = true;               // 新しい爆発生成メソッドを使用するか
	int debugMaxCount_ = 15;                          // 総スクラップ数
	int debugBigSizeCount_ = 3;                       // 大きいサイズの数
	int debugMidSizeCount_ = 5;                       // 中サイズの数（3サイズ混合時のみ）
	ScrapGenerateSize debugGenerateSize_ = ScrapGenerateSize::SmallAndMedium; // サイズ組み合わせ

	// 吸引→保持の移行判定パラメータ
	bool useAdvancedHoldTransition_ = true;       // 動的判定を使用するか
	float holdTransitionRadiusRatio_ = 0.8f;      // 最外層半径の何%で保持するか (0.0-1.0)
	float holdTransitionMaxRadius_ = 30.0f;       // 判定距離の上限
	float holdTransitionMinRadius_ = 5.0f;        // 判定距離の下限（保持数が少ない時）

private: // 定数
	constexpr static int kMaxScraps = 500;
	constexpr static float kMinSpawnDistance = 4.0f;
	constexpr static float kCollisionPushForce = 50.0f;
	constexpr static float kHeldOrbitRadiusBase = 30.0f;  // 保持中の基本軌道半径
	constexpr static float kHeldOrbitRadiusStep = 15.0f;  // 層ごとの半径増加量

	constexpr static float kOutOfBoundsMargin = 200.0f;  // 画面外判定のマージン
};