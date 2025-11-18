#pragma once
#include "Vector2.h"
#include "Vertex4.h"
#include "Vertex4Component.h"
#include "Matrix3x3.h"
#include "Affine2D.h"
#include "Camera2D.h"
#include <Novice.h>
#include "Animation.h"
#include <memory>
#include "WindowSize.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

struct Graph {
	int handle = -1;
	unsigned int grDrawWidth = 1;
	unsigned int grDrawHeight = 1;
	unsigned int scrX = 0;
	unsigned int scrY = 0;
	unsigned int scrWidth = 1;
	unsigned int scrHeight = 1;

	unsigned int color = 0xFFFFFFFF;
	Graph() = default;
	Graph(int handle, unsigned int width, unsigned int height, unsigned int color = 0xFFFFFFFF)
		: handle(handle), grDrawWidth(width), grDrawHeight(height), color(color) {
	}
};

// RGBA色構造体
struct ColorRGBA {
	float r = 1.0f;  // 0.0 ~ 1.0
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;

	ColorRGBA() = default;
	ColorRGBA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	// unsigned intからRGBAへ変換（0xRRGGAA形式）
	static ColorRGBA FromUInt(unsigned int color) {
		ColorRGBA rgba;
		rgba.a = ((color >> 0) & 0xFF) / 255.0f;  // 最上位バイト
		rgba.b = ((color >> 8) & 0xFF) / 255.0f;   // 最下位バイト
		rgba.g = ((color >> 16) & 0xFF) / 255.0f;
		rgba.r = ((color >> 24) & 0xFF) / 255.0f;
		return rgba;
	}

	// RGBAからunsigned intへ変換（0xRRGGBBAA形式）
	unsigned int ToUInt() const {
		unsigned int r_int = static_cast<unsigned int>(r * 255.0f);
		unsigned int g_int = static_cast<unsigned int>(g * 255.0f);
		unsigned int b_int = static_cast<unsigned int>(b * 255.0f);
		unsigned int a_int = static_cast<unsigned int>(a * 255.0f);
		return (a_int << 0) | (r_int << 24) | (g_int << 16) | (b_int << 8);
	}

	// 線形補間
	static ColorRGBA Lerp(const ColorRGBA& start, const ColorRGBA& end, float t) {
		t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
		return ColorRGBA(
			start.r + (end.r - start.r) * t,
			start.g + (end.g - start.g) * t,
			start.b + (end.b - start.b) * t,
			start.a + (end.a - start.a) * t
		);
	}
};

// 視覚効果用の構造体
struct ShakeEffect {
	bool isActive = false;
	float range = 0.0f;
	float duration = 0.0f;
	float elapsed = 0.0f;
	bool continuous = false;
	Vector2 offset = { 0.0f, 0.0f };
};

struct RotationEffect {
	bool isActive = false;
	float speed = 0.0f;
	float duration = 0.0f;
	float elapsed = 0.0f;
	bool continuous = false;
	float accumulatedAngle = 0.0f;
};

struct SquashStretchEffect {
	bool isActive = false;
	Vector2 scale = { 1.0f, 1.0f };
	float duration = 0.0f;
	float elapsed = 0.0f;
	int maxCount = 0;
	int currentCount = 0;
	bool expanding = true;
};

struct FadeEffect {
	bool isActive = false;
	float duration = 0.0f;
	float elapsed = 0.0f;
	ColorRGBA currentColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	ColorRGBA startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	ColorRGBA targetColor = { 1.0f, 1.0f, 1.0f, 0.0f };
	bool colorTransition = false;  // 色変化モードかどうか
};

struct ScaleEffect {
	bool isActive = false;
	float minScale = 1.0f;
	float maxScale = 1.0f;
	float speed = 0.0f;
	int maxCount = 0;
	int currentCount = 0;
	bool continuous = false;
	bool expanding = true;
	float currentScale = 1.0f;
};

// 縮小消滅エフェクト用の構造体
struct ScaleExtinctionEffect {
	bool isActive = false;
	float duration = 0.0f;           // 消滅にかかる時間
	float elapsed = 0.0f;            // 経過時間
	float startScale = 1.0f;         // 開始スケール
	float endScale = 0.0f;           // 終了スケール（通常0）
	int easingType = 0;              // イージングタイプ（0:Linear, 1:EaseOut, 2:EaseIn, 3:EaseInOut）
	bool fadeWithScale = true;       // スケールと同時にフェードアウトするか
	ColorRGBA startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float currentScale = 1.0f;
};

class DrawComponent2D {
public:
	// 基本プロパティ
	float width_ = 50.0f;
	float height_ = 50.0f;
	Vector2 scale_ = { 1.0f, 1.0f };
	Vector2 position_ = { 0.0f, 0.0f };
	float angle_ = 0.0f;
	Vector2 pivot_ = { 0.5f, 0.5f };

	Graph graph_;
	Vertex4Component vertex_;
	std::unique_ptr<Animation> animation_;

	// 視覚効果
	ShakeEffect shakeEffect_;
	RotationEffect rotationEffect_;
	SquashStretchEffect squashStretchEffect_;
	FadeEffect fadeEffect_;
	ScaleEffect scaleEffect_;
	ScaleExtinctionEffect scaleExtinctionEffect_;

	// コンストラクタ
	DrawComponent2D();
	~DrawComponent2D() = default;
	DrawComponent2D(const Vector2& center, const float width, const float height, Graph graph);

	/// <summary>
	/// アニメーション対応コンストラクタ
	/// </summary>
	/// <param name="center">描画中心座標</param>
	/// <param name="width">描画横幅</param>
	/// <param name="height">描画縦幅</param>
	/// <param name="grHandle">グラフハンドル</param>
	/// <param name="grWidth">画像の横幅</param>
	/// <param name="grHeight">画像の縦幅</param>
	/// <param name="totalFrames">総フレーム数</param>
	/// <param name="grSplit">一行のフレーム数</param>
	/// <param name="animeSpeed">アニメーションスピード 小さいほど早い</param>
	/// <param name="isLoop">ループするかどうか</param>
	/// <param name="color">描画の色 書かなくてもOK</param>
	DrawComponent2D(const Vector2& center, const float width, const float height,
		int grHandle, int grWidth, int grHeight, int totalFrames, int grSplit,
		float animeSpeed, bool isLoop, unsigned int color = 0xFFFFFFFF);

	// コピー・ムーブ
	DrawComponent2D(const DrawComponent2D& drawcomp);
	DrawComponent2D(DrawComponent2D&& drawcomp) noexcept;
	DrawComponent2D& operator=(const DrawComponent2D& drawcomp);
	DrawComponent2D& operator=(DrawComponent2D&& drawcomp) noexcept;

	// グラフ設定
	void SetGraph(int grHandle, unsigned int grDrawWidth, unsigned int grDrawHeight, unsigned int color = 0xFFFFFFFF);

	// アニメーション制御
	void SetAnimation(std::unique_ptr<Animation> animation);
	void PlayAnimation();
	void StopAnimation();
	void UpdateAnimation(float deltaTime);
	void UpdateAnimation(float deltaTime, const Vector2& center, const float angle, const Vector2& scale);

	// 更新
	void Update();
	void Update(const Vector2& center, const Vector2& scale, const float rotate, const float width, const float height, Graph graph);

	// エフェクト更新（Update内で呼ばれる）
	void UpdateEffects(float deltaTime);

	// ========== シェイク効果 ==========
	// 時間指定版

	/// <summary>
	/// シェイク効果開始
	/// </summary>
	/// <param name="range"></param>
	/// <param name="duration"></param>
	void StartShake(float range, float duration);
	// 継続フラグ版
	void StartShakeContinuous(float range);
	void StopShake();

	// ========== 回転効果 ==========
	// 時間指定版
	void StartRotation(float speed, float duration);
	// 継続フラグ版
	void StartRotationContinuous(float speed);
	void StopRotation();

	// ========== 潰しと伸ばし効果 ==========
	void StartSquashStretch(const Vector2& scaleAmount, float duration, int count);

	// ========== フェード効果 ==========
	// アルファ値のみフェード（透明度を下げる）
	void StartFade(float duration);
	// 指定色へフェード
	void StartFadeToColor(unsigned int targetColor, float duration);
	void StartFadeToColor(const ColorRGBA& targetColor, float duration);
	void StopFade();

	// ========== 拡大縮小効果 ==========
	// 回数指定版
	void StartScale(float minScale, float maxScale, float speed, int count);
	// 継続フラグ版
	void StartScaleContinuous(float minScale, float maxScale, float speed);
	void StopScale();

	/// <summary>
	/// 縮小しながら消滅するエフェクトを開始
	/// </summary>
	/// <param name="duration">消滅にかかる時間（秒）</param>
	/// <param name="easingType">イージングタイプ（0:Linear, 1:EaseOut, 2:EaseIn, 3:EaseInOut）</param>
	/// <param name="fadeWithScale">スケールと同時に透明度も下げるか</param>
	void StartScaleExtinction(float duration, int easingType = 1, bool fadeWithScale = true);

	/// <summary>
	/// 開始スケールを指定して縮小消滅
	/// </summary>
	void StartScaleExtinction(float startScale, float duration, int easingType = 1, bool fadeWithScale = true);

	void StopScaleExtinction();

	// エフェクト状態取得に追加
	bool IsScaleExtinctionActive() const { return scaleExtinctionEffect_.isActive; }
	bool IsScaleExtinctionFinished() const {
		return scaleExtinctionEffect_.isActive && scaleExtinctionEffect_.elapsed >= scaleExtinctionEffect_.duration;
	}

	// エフェクトリセット
	void ResetAllEffects();

	// 描画メソッド（既存）
	void DrawCamera(const Camera2D& camera);
	void DrawAnimationCamera(const Camera2D& camera);

	void DrawQuadWorld();
	void DrawAnimationWorld();
	void DrawAnimationScreen();

	// 描画メソッド（拡張）
	/*-------------------------------*/

	// 画像指定
	void DrawAnimationScreen(int grHandle);
	// 色指定
	void DrawAnimationScreen(unsigned int color);
	// オフセット指定
	void DrawAnimationScreenWorldToScreen(const Vector2& offset);
	// 画像＋オフセット指定
	void DrawAnimationScreenWorldToScreen(const Vector2& offset, int grHandle);
	// 反転描画＋オフセット指定
	void DrawAnimationScreenWorldToScreenReverse(const Vector2& offset);
	// 反転描画＋画像＋オフセット指定
	void DrawAnimationScreenWorldToScreenReverse(const Vector2& offset, int grHandle);
	// ワールド座標指定＋オフセット指定
	void DrawAnimationScreenWorldToScreen(const Vector2& worldPosition, const Vector2& offset);
	// オフセット指定
	void DrawAnimationScreen(const Vector2& offset);

	// アニメーション状態取得
	bool HasAnimation() const;
	bool IsAnimationPlaying() const;

	// エフェクト状態取得
	bool IsShakeActive() const { return shakeEffect_.isActive; }
	bool IsRotationActive() const { return rotationEffect_.isActive; }
	bool IsSquashStretchActive() const { return squashStretchEffect_.isActive; }
	bool IsFadeActive() const { return fadeEffect_.isActive; }
	bool IsScaleActive() const { return scaleEffect_.isActive; }

	// デバッグウィンドウ
	void DrawDebugWindow(const char* title);

private:
	// エフェクト更新用内部メソッド
	void UpdateShake(float deltaTime);
	void UpdateRotation(float deltaTime);
	void UpdateSquashStretch(float deltaTime);
	void UpdateFade(float deltaTime);
	void UpdateScale(float deltaTime);
	void UpdateScaleExtinction(float deltaTime);

	// エフェクト適用後の変換行列を取得
	Matrix3x3 GetEffectAppliedMatrix() const;
	Vector2 GetEffectAppliedPosition() const;
	Vector2 GetEffectAppliedScale() const;
	float GetEffectAppliedAngle() const;
	unsigned int GetEffectAppliedColor() const;

	// イージング関数適用
	float ApplyEasing(float t, int easingType) const;
};