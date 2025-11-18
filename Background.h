#pragma once

#include "Novice.h"
#include "Vector2.h"
#include "WindowSize.h"
#include <vector>
#include "DrawComponent2D.h"

class BackGround {
public:
	Vector2 pos_{};
	float   width_ = 1280.0f;
	float   height_ = 720.0f;
	int grHandle_ = -1;

	BackGround(float x, float y, int grHandle) : pos_{ x, y }, grHandle_{ grHandle } {}

	/// <summary>
	/// カメラの視界内にあるかチェック
	/// </summary>
	bool IsVisible(const Vector2& cameraOffset, float cameraWidth = 1280.0f, float cameraHeight = 720.0f) const {
		// カメラの視界範囲を計算
		float cameraLeft = cameraOffset.x;
		float cameraRight = cameraOffset.x + cameraWidth;
		float cameraTop = cameraOffset.y + cameraHeight;
		float cameraBottom = cameraOffset.y;

		// 背景の範囲を計算
		float bgLeft = pos_.x;
		float bgRight = pos_.x + width_;
		float bgTop = pos_.y + height_;
		float bgBottom = pos_.y;

		// AABB（軸平行境界ボックス）による交差判定
		bool horizontalOverlap = (bgLeft < cameraRight) && (bgRight > cameraLeft);
		bool verticalOverlap = (bgBottom < cameraTop) && (bgTop > cameraBottom);

		return horizontalOverlap && verticalOverlap;
	}

	void Draw(Vector2 offset) {
		// カリング：画面外なら早期リターン
		if (!IsVisible(offset)) {
			return;
		}

		Novice::DrawSpriteRect(
			static_cast<int>(pos_.x - offset.x),
			static_cast<int>((pos_.y - offset.y) * -1),
			0, 0,
			static_cast<int>(kWindowWidth),
			static_cast<int>(kWindowHeight),
			grHandle_,
			1, 1,
			0.0f,
			0xFFFFFFFF
		);
	}

	void Draw(Vector2 offset, unsigned int color) {
		// カリング：画面外なら早期リターン
		if (!IsVisible(offset)) {
			return;
		}

		Novice::DrawSpriteRect(
			static_cast<int>(pos_.x - offset.x),
			static_cast<int>((pos_.y - offset.y) * -1),
			0, 0,
			static_cast<int>(kWindowWidth),
			static_cast<int>(kWindowHeight),
			grHandle_,
			1, 1,
			0.0f,
			color
		);
	}

	void Draw(unsigned int color) {
		Novice::DrawSprite(
			0,
			0,
			grHandle_,
			1.0f,
			1.0f,
			0.0f,
			color
		);
	}
};


// ========================================
// 新しい背景レイヤーシステム
// ========================================

/// <summary>
/// 背景レイヤーの情報を保持する構造体
/// </summary>
struct BackgroundLayer {
	std::vector<BackGround> tiles;      // 背景タイル配列
	int normalTexture = -1;             // 通常テクスチャ
	int goalTexture = -1;               // ゴールテクスチャ（使用しない場合は-1）
	float parallaxFactor = 1.0f;        // 視差係数（0.0～1.0）
	float baseY = 0.0f;                 // 基準Y座標
	bool isScreenSpace = false;         // スクリーン空間で描画するか
	unsigned int color = 0xFFFFFFFF;

	BlendMode blendMode = kBlendModeNormal;

	// ========================================
	// アニメーション機能
	// ========================================
	bool useAnimation = false;          // アニメーションを使用するか
	DrawComponent2D animDrawComp;       // アニメーション描画コンポーネント
	Vector2 animScreenPos{ 0.0f, 0.0f }; // スクリーン座標（アニメーション用）

	BackgroundLayer() = default;
};