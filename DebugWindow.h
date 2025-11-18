#pragma once
#include "Vector2.h"

class DebugWindow {
public:
	DebugWindow() = default;
	~DebugWindow() = default;

	// ========================================
	// 統合デバッグGUI描画
	// ========================================
	void DrawDebugGui();
};