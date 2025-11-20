#pragma once
#include "Vector2.h"

class Camera2D;
class Player;

/// <summary>
/// 統合デバッグウィンドウ
/// カメラ、プレイヤー、その他のデバッグ情報を一元管理
/// </summary>
class DebugWindow {
public:
	DebugWindow() = default;
	~DebugWindow() = default;

	// ========================================
	// 統合デバッグGUI描画
	// ========================================
	void DrawDebugGui();

	// ========================================
	// カメラデバッグGUI
	// ========================================

	/// <summary>
	/// カメラのデバッグウィンドウを描画
	/// </summary>
	/// <param name="camera">デバッグ対象のカメラ</param>
	void DrawCameraDebugWindow(Camera2D* camera);

	// ========================================
	// プレイヤーデバッグGUI
	// ========================================

	/// <summary>
	/// プレイヤーのデバッグウィンドウを描画
	/// </summary>
	/// <param name="player">デバッグ対象のプレイヤー</param>
	void DrawPlayerDebugWindow(Player* player);

private:
	// カメラデバッグモードの状態
	bool cameraDebugMode_ = false;
	bool showCameraWindow_ = true;
	bool showCameraInfo_ = true;
	bool showCameraEffects_ = true;
	bool showCameraControls_ = true;

	// プレイヤーデバッグの状態
	bool showPlayerWindow_ = true;
};