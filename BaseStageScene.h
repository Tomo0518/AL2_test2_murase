#pragma once
#include <memory>
#include "GameSceneBase.h"
#include "GameShared.h"
#include "Background.h"

class SceneManager;

/// <summary>
/// ステージシーンの基底クラス
/// シーン遷移入力と背景描画のみ
/// </summary>
class BaseStageScene : public GameSceneBase {
public:
	BaseStageScene(SceneManager& manager, GameShared& shared, int stageIndex);
	virtual ~BaseStageScene() = default;

	int GetStageIndex() const override { return stageIndex_; }

	void Update(float dt, const char* keys, const char* pre) override;
	void Draw() override;

protected:
	// ========================================
	// 派生クラスでオーバーライド可能なメソッド
	// ========================================

	/// <summary>
	/// ステージ固有の初期化処理
	/// </summary>
	virtual void InitializeStage() {}

	/// <summary>
	/// ステージ固有の更新処理
	/// </summary>
	virtual void UpdateStage(float dt, const char* keys, const char* pre) {
		(void)dt; (void)keys; (void)pre;
	}

	/// <summary>
	/// ステージ固有の描画処理
	/// </summary>
	virtual void DrawStage() {}


	// ========================================
	// メンバ変数
	// ========================================

	int stageIndex_ = 0;             // ステージ番号
	GameShared& shared_;             // 共有リソース
	SceneManager& manager_;          // シーンマネージャー
	// 初期化フラグ
	bool initialized_ = false;

private:
	int grHandleBackground_ = -1; // 背景テクスチャハンドル
	DrawComponent2D drawCompBackground_; // 背景描画コンポーネント
};