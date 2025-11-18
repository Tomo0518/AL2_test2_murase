#pragma once
#include "Vector2.h"
#include "DrawComponent2D.h"
#include "FontAtlas.h"
#include "TextRenderer.h"
#include <functional>
#include <string>

/// <summary>
/// 個別のボタンクラス
/// </summary>
class Button {
public:
	Button() = default;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="position">ボタンの位置（中心座標）</param>
	/// <param name="size">ボタンのサイズ</param>
	/// <param name="label">ボタンに表示するテキスト</param>
	/// <param name="callback">ボタンが押された時のコールバック関数</param>
	Button(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	/// <param name="isSelected">選択中かどうか</param>
	void Update(float deltaTime, bool isSelected);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="textureHandle">ボタンのテクスチャハンドル</param>
	/// <param name="font">フォントアトラス</param>
	/// <param name="textRenderer">テキストレンダラー</param>
	void Draw(int textureHandle, FontAtlas* font, TextRenderer* textRenderer) const;

	/// <summary>
	/// ボタンのアクションを実行
	/// </summary>
	void Execute();

	// ゲッター
	Vector2 GetPosition() const { return position_; }
	Vector2 GetSize() const { return size_; }
	std::string GetLabel() const { return label_; }
	bool IsSelected() const { return isSelected_; }

	// セッター
	void SetPosition(const Vector2& position) { position_ = position; }
	void SetSize(const Vector2& size) { size_ = size; }
	void SetLabel(const std::string& label) { label_ = label; }
	void SetCallback(std::function<void()> callback) { callback_ = callback; }

	// 色設定
	void SetColorNormal(uint32_t color) { colorNormal_ = color; }
	void SetColorSelected(uint32_t color) { colorSelected_ = color; }
	void SetColorFrame(uint32_t color) { colorFrame_ = color; }
	void SetColorFrameSelected(uint32_t color) { colorFrameSelected_ = color; }
	void SetColorText(uint32_t color) { colorText_ = color; }
	void SetColorTextSelected(uint32_t color) { colorTextSelected_ = color; }

	// スケール設定
	void SetScaleMin(float scale) { scaleMin_ = scale; }
	void SetScaleMax(float scale) { scaleMax_ = scale; }
	void SetEaseSpeed(float speed) { easeSpeed_ = speed; }

	// アンカー設定
	void SetAnchor(const Vector2& anchor) { anchor_ = anchor; }

private:
	Vector2 position_;              // ボタンの位置
	Vector2 size_;                  // ボタンのサイズ
	Vector2 anchor_ = { 0.5f, 0.5f }; // アンカー（デフォルトは中心）
	std::string label_;             // ボタンのラベル
	std::function<void()> callback_; // コールバック関数

	DrawComponent2D drawComp_;      // 描画コンポーネント

	bool isSelected_ = false;       // 選択中かどうか
	float easeT_ = 0.0f;            // イージング用の時間パラメータ

	// 色設定
	uint32_t colorNormal_ = 0x582626FF;     // 通常時の色
	uint32_t colorSelected_ = 0xEB8787FF;   // 選択時の色
	uint32_t colorFrame_ = 0xB5B5B5FF;      // 枠線の色（通常）
	uint32_t colorFrameSelected_ = 0xFFFFFFFF; // 枠線の色（選択時）
	uint32_t colorText_ = 0x7E7E7EFF;       // テキストの色（通常）
	uint32_t colorTextSelected_ = 0xFFFFFFFF; // テキストの色（選択時）

	// スケール設定
	float scaleMin_ = 1.0f;         // 最小スケール
	float scaleMax_ = 1.18f;        // 最大スケール
	float easeSpeed_ = 8.0f;        // イージング速度

	// テキスト設定
	float textScale_ = 1.0f;        // テキストのスケール
	float textScaleSelected_ = 1.05f; // 選択時のテキストスケール
};