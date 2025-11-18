#include "Button.h"
#include "Easing.h"
#include <Novice.h>
#include <algorithm>

Button::Button(const Vector2& position, const Vector2& size, const std::string& label, std::function<void()> callback)
	: position_(position),
	size_(size),
	label_(label),
	callback_(callback) {

	// DrawComponent2Dの初期化
	drawComp_.position_ = position_;
	drawComp_.width_ = size_.x;
	drawComp_.height_ = size_.y;
	drawComp_.pivot_ = anchor_;
}

void Button::Update(float deltaTime, bool isSelected) {
	isSelected_ = isSelected;

	// イージングのターゲット値を設定
	float target = isSelected_ ? 1.0f : 0.0f;

	// イージングでスケールを変更
	easeT_ += (target - easeT_) * std::clamp(easeSpeed_ * deltaTime, 0.0f, 1.0f);

	// イージングを適用してスケールを計算
	float eased = Easing::EaseOutQuad(easeT_);
	float scale = std::lerp(scaleMin_, scaleMax_, eased);

	// DrawComponent2Dのスケールを更新
	drawComp_.scale_ = { scale, scale };
	drawComp_.position_ = position_;
}

void Button::Draw(int textureHandle, FontAtlas* font, TextRenderer* textRenderer) const {
	textureHandle; // 未使用パラメータ回避

	// ボタンの背景を描画
	uint32_t fillColor = isSelected_ ? colorSelected_ : colorNormal_;

	// 描画位置とサイズを計算（アンカーを考慮）
	Vector2 scale = drawComp_.scale_;
	float w = size_.x * scale.x;
	float h = size_.y * scale.y;

	// アンカーに基づいて左上の座標を計算
	float left = position_.x - w * anchor_.x;
	float top = position_.y - h * anchor_.y;

	// 背景の描画
	Novice::DrawBox(
		static_cast<int>(left),
		static_cast<int>(top),
		static_cast<int>(w),
		static_cast<int>(h),
		0.0f,
		fillColor,
		kFillModeSolid
	);

	// 枠線の描画
	uint32_t frameColor = isSelected_ ? colorFrameSelected_ : colorFrame_;
	Novice::DrawBox(
		static_cast<int>(left),
		static_cast<int>(top),
		static_cast<int>(w),
		static_cast<int>(h),
		0.0f,
		frameColor,
		kFillModeWireFrame
	);

	// テキストの描画
	if (font && textRenderer) {
		float labelScale = isSelected_ ? textScaleSelected_ : textScale_;
		uint32_t textColor = isSelected_ ? colorTextSelected_ : colorText_;

		// テキストの幅を計算して中央に配置
		int textWidth = textRenderer->MeasureWidth(label_.c_str(), labelScale);
		int textHeight = static_cast<int>(font->GetLineHeight() * labelScale);

		int textX = static_cast<int>(left + (w - textWidth) * 0.5f);
		int textY = static_cast<int>(top + (h - textHeight) * 0.5f);

		textRenderer->DrawTextLabel(textX, textY, label_.c_str(), textColor, labelScale);
	}
}

void Button::Execute() {
	if (callback_) {
		callback_();
	}
}