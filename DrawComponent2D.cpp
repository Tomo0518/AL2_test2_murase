#include "DrawComponent2D.h"
#include <cmath>
#include <random>

// ========== コンストラクタ ==========
DrawComponent2D::DrawComponent2D() {
	width_ = 50.0f;
	height_ = 50.0f;
	vertex_.SetBySize(width_, height_);
}

DrawComponent2D::DrawComponent2D(const Vector2& center, const float width, const float height, Graph graph)
	: position_(center), width_(width), height_(height), graph_(graph) {
	vertex_.SetBySize(width, height);
}

DrawComponent2D::DrawComponent2D(const Vector2& center, const float width, const float height,
	int grHandle, int grWidth, int grHeight, int totalFrames, int grSplit,
	float animeSpeed, bool isLoop, unsigned int color)
	: position_(center), width_(width), height_(height) {

	animation_ = std::make_unique<Animation>(grHandle, grWidth, grHeight, totalFrames, grSplit, animeSpeed, isLoop);
	graph_ = Graph(grHandle, grWidth, grHeight, color);
	vertex_.SetBySize(width, height);
	animation_->Play();
}

DrawComponent2D::DrawComponent2D(const DrawComponent2D& drawcomp)
	: width_(drawcomp.width_)
	, height_(drawcomp.height_)
	, scale_(drawcomp.scale_)
	, position_(drawcomp.position_)
	, angle_(drawcomp.angle_)
	, pivot_(drawcomp.pivot_)
	, graph_(drawcomp.graph_)
	, vertex_(drawcomp.vertex_)
	, animation_(nullptr)
	, shakeEffect_(drawcomp.shakeEffect_)
	, rotationEffect_(drawcomp.rotationEffect_)
	, squashStretchEffect_(drawcomp.squashStretchEffect_)
	, fadeEffect_(drawcomp.fadeEffect_)
	, scaleEffect_(drawcomp.scaleEffect_)
{
	if (drawcomp.animation_) {
		animation_ = std::make_unique<Animation>(*drawcomp.animation_);
		if (drawcomp.animation_->IsPlaying()) {
			animation_->Play();
		}
	}
}

DrawComponent2D::DrawComponent2D(DrawComponent2D&& drawcomp) noexcept
	: width_(drawcomp.width_)
	, height_(drawcomp.height_)
	, scale_(drawcomp.scale_)
	, position_(drawcomp.position_)
	, angle_(drawcomp.angle_)
	, pivot_(drawcomp.pivot_)
	, graph_(drawcomp.graph_)
	, vertex_(drawcomp.vertex_)
	, animation_(std::move(drawcomp.animation_))
	, shakeEffect_(drawcomp.shakeEffect_)
	, rotationEffect_(drawcomp.rotationEffect_)
	, squashStretchEffect_(drawcomp.squashStretchEffect_)
	, fadeEffect_(drawcomp.fadeEffect_)
	, scaleEffect_(drawcomp.scaleEffect_)
{
}

DrawComponent2D& DrawComponent2D::operator=(const DrawComponent2D& drawcomp) {
	if (this != &drawcomp) {
		width_ = drawcomp.width_;
		height_ = drawcomp.height_;
		scale_ = drawcomp.scale_;
		position_ = drawcomp.position_;
		angle_ = drawcomp.angle_;
		pivot_ = drawcomp.pivot_;
		graph_ = drawcomp.graph_;
		vertex_ = drawcomp.vertex_;
		shakeEffect_ = drawcomp.shakeEffect_;
		rotationEffect_ = drawcomp.rotationEffect_;
		squashStretchEffect_ = drawcomp.squashStretchEffect_;
		fadeEffect_ = drawcomp.fadeEffect_;
		scaleEffect_ = drawcomp.scaleEffect_;

		if (drawcomp.animation_) {
			animation_ = std::make_unique<Animation>(*drawcomp.animation_);
			if (drawcomp.animation_->IsPlaying()) {
				animation_->Play();
			}
		}
		else {
			animation_.reset();
		}
	}
	return *this;
}

DrawComponent2D& DrawComponent2D::operator=(DrawComponent2D&& drawcomp) noexcept {
	if (this != &drawcomp) {
		width_ = drawcomp.width_;
		height_ = drawcomp.height_;
		scale_ = drawcomp.scale_;
		position_ = drawcomp.position_;
		angle_ = drawcomp.angle_;
		pivot_ = drawcomp.pivot_;
		graph_ = drawcomp.graph_;
		vertex_ = drawcomp.vertex_;
		animation_ = std::move(drawcomp.animation_);
		shakeEffect_ = drawcomp.shakeEffect_;
		rotationEffect_ = drawcomp.rotationEffect_;
		squashStretchEffect_ = drawcomp.squashStretchEffect_;
		fadeEffect_ = drawcomp.fadeEffect_;
		scaleEffect_ = drawcomp.scaleEffect_;
	}
	return *this;
}

// ========== グラフ設定 ==========
void DrawComponent2D::SetGraph(int grHandle, unsigned int grDrawWidth, unsigned int grDrawHeight, unsigned int color) {
	graph_.handle = grHandle;
	graph_.grDrawWidth = grDrawWidth;
	graph_.grDrawHeight = grDrawHeight;
	graph_.color = color;
}

// ========== アニメーション制御 ==========
void DrawComponent2D::SetAnimation(std::unique_ptr<Animation> animation) {
	animation_ = std::move(animation);
}

void DrawComponent2D::PlayAnimation() {
	if (animation_) {
		animation_->Play();
	}
}

void DrawComponent2D::StopAnimation() {
	if (animation_) {
		animation_->Stop();
	}
}

void DrawComponent2D::UpdateAnimation(float deltaTime) {
	if (animation_) {
		animation_->Update(deltaTime);
	}
}

void DrawComponent2D::UpdateAnimation(float deltaTime, const Vector2& center, const float angle, const Vector2& scale) {
	position_ = center;
	angle_ = angle;
	scale_ = scale;

	if (animation_) {
		animation_->Update(deltaTime);
	}
}

// ========== 更新 ==========
void DrawComponent2D::Update() {
	vertex_.SetBySize(width_, height_);
}

void DrawComponent2D::Update(const Vector2& center, const Vector2& scale, const float rotate, const float width, const float height, Graph graph) {
	position_ = center;
	scale_ = scale;
	angle_ = rotate;
	width_ = width;
	height_ = height;
	graph_ = graph;
	vertex_.SetBySize(width, height);
}

void DrawComponent2D::UpdateEffects(float deltaTime) {
	UpdateShake(deltaTime);
	UpdateRotation(deltaTime);
	UpdateSquashStretch(deltaTime);
	UpdateFade(deltaTime);
	UpdateScale(deltaTime);
	UpdateScaleExtinction(deltaTime);
}

// ========== シェイク効果 ==========
void DrawComponent2D::StartShake(float range, float duration) {
	shakeEffect_.isActive = true;
	shakeEffect_.range = range;
	shakeEffect_.duration = duration;
	shakeEffect_.elapsed = 0.0f;
	shakeEffect_.continuous = false;
}

void DrawComponent2D::StartShakeContinuous(float range) {
	shakeEffect_.isActive = true;
	shakeEffect_.range = range;
	shakeEffect_.continuous = true;
	shakeEffect_.elapsed = 0.0f;
}

void DrawComponent2D::StopShake() {
	shakeEffect_.isActive = false;
	shakeEffect_.offset = { 0.0f, 0.0f };
}

void DrawComponent2D::UpdateShake(float deltaTime) {
	if (!shakeEffect_.isActive) {
		return;
	}

	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(-shakeEffect_.range, shakeEffect_.range);

	shakeEffect_.offset.x = dist(gen);
	shakeEffect_.offset.y = dist(gen);

	if (!shakeEffect_.continuous) {
		shakeEffect_.elapsed += deltaTime;
		if (shakeEffect_.elapsed >= shakeEffect_.duration) {
			StopShake();
		}
	}
}

// ========== 回転効果 ==========
void DrawComponent2D::StartRotation(float speed, float duration) {
	rotationEffect_.isActive = true;
	rotationEffect_.speed = speed;
	rotationEffect_.duration = duration;
	rotationEffect_.elapsed = 0.0f;
	rotationEffect_.continuous = false;
	rotationEffect_.accumulatedAngle = 0.0f;
}

void DrawComponent2D::StartRotationContinuous(float speed) {
	rotationEffect_.isActive = true;
	rotationEffect_.speed = speed;
	rotationEffect_.continuous = true;
	rotationEffect_.elapsed = 0.0f;
	rotationEffect_.accumulatedAngle = 0.0f;
}

void DrawComponent2D::StopRotation() {
	rotationEffect_.isActive = false;
	rotationEffect_.accumulatedAngle = 0.0f;
}

void DrawComponent2D::UpdateRotation(float deltaTime) {
	if (!rotationEffect_.isActive) {
		return;
	}

	rotationEffect_.accumulatedAngle += rotationEffect_.speed * deltaTime;

	if (!rotationEffect_.continuous) {
		rotationEffect_.elapsed += deltaTime;
		if (rotationEffect_.elapsed >= rotationEffect_.duration) {
			StopRotation();
		}
	}
}

// ========== 潰しと伸ばし効果 ==========
void DrawComponent2D::StartSquashStretch(const Vector2& scaleAmount, float duration, int count) {
	squashStretchEffect_.isActive = true;
	squashStretchEffect_.scale = scaleAmount;
	squashStretchEffect_.duration = duration;
	squashStretchEffect_.maxCount = count;
	squashStretchEffect_.currentCount = 0;
	squashStretchEffect_.elapsed = 0.0f;
	squashStretchEffect_.expanding = true;
}

void DrawComponent2D::UpdateSquashStretch(float deltaTime) {
	if (!squashStretchEffect_.isActive) {
		return;
	}

	squashStretchEffect_.elapsed += deltaTime;

	if (squashStretchEffect_.elapsed >= squashStretchEffect_.duration) {
		squashStretchEffect_.elapsed = 0.0f;
		squashStretchEffect_.expanding = !squashStretchEffect_.expanding;

		if (!squashStretchEffect_.expanding) {
			squashStretchEffect_.currentCount++;
		}

		if (squashStretchEffect_.currentCount >= squashStretchEffect_.maxCount) {
			squashStretchEffect_.isActive = false;
			squashStretchEffect_.scale = { 1.0f, 1.0f };
		}
	}
}

// ========== フェード効果（RGBA対応） ==========
void DrawComponent2D::StartFade(float duration) {
	fadeEffect_.isActive = true;
	fadeEffect_.duration = duration;
	fadeEffect_.elapsed = 0.0f;
	fadeEffect_.colorTransition = false;

	// 現在の色を取得してRGBAに変換
	fadeEffect_.startColor = ColorRGBA::FromUInt(graph_.color);
	fadeEffect_.currentColor = fadeEffect_.startColor;

	// ターゲットはアルファだけ0にする（透明にする）
	fadeEffect_.targetColor = fadeEffect_.startColor;
	fadeEffect_.targetColor.a = 0.0f;
}

void DrawComponent2D::StartFadeToColor(unsigned int targetColor, float duration) {
	fadeEffect_.isActive = true;
	fadeEffect_.duration = duration;
	fadeEffect_.elapsed = 0.0f;
	fadeEffect_.colorTransition = true;

	// 現在の色とターゲット色を設定
	fadeEffect_.startColor = ColorRGBA::FromUInt(graph_.color);
	fadeEffect_.targetColor = ColorRGBA::FromUInt(targetColor);
	fadeEffect_.currentColor = fadeEffect_.startColor;
}

void DrawComponent2D::StartFadeToColor(const ColorRGBA& targetColor, float duration) {
	fadeEffect_.isActive = true;
	fadeEffect_.duration = duration;
	fadeEffect_.elapsed = 0.0f;
	fadeEffect_.colorTransition = true;

	fadeEffect_.startColor = ColorRGBA::FromUInt(graph_.color);
	fadeEffect_.targetColor = targetColor;
	fadeEffect_.currentColor = fadeEffect_.startColor;
}

void DrawComponent2D::StopFade() {
	fadeEffect_.isActive = false;
	fadeEffect_.currentColor = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void DrawComponent2D::UpdateFade(float deltaTime) {
	if (!fadeEffect_.isActive) {
		return;
	}

	fadeEffect_.elapsed += deltaTime;
	float t = fadeEffect_.elapsed / fadeEffect_.duration;

	if (t >= 1.0f) {
		t = 1.0f;
		fadeEffect_.currentColor = fadeEffect_.targetColor;
		fadeEffect_.isActive = false;
	}
	else {
		// 線形補間で色を変化
		fadeEffect_.currentColor = ColorRGBA::Lerp(fadeEffect_.startColor, fadeEffect_.targetColor, t);
	}
}

// ========== 拡大縮小効果 ==========
void DrawComponent2D::StartScale(float minScale, float maxScale, float speed, int count) {
	scaleEffect_.isActive = true;
	scaleEffect_.minScale = minScale;
	scaleEffect_.maxScale = maxScale;
	scaleEffect_.speed = speed;
	scaleEffect_.maxCount = count;
	scaleEffect_.currentCount = 0;
	scaleEffect_.continuous = false;
	scaleEffect_.expanding = true;
	scaleEffect_.currentScale = minScale;
}

/// <summary>
/// 継続拡縮、StopScaleするまで継続
/// </summary>
/// <param name="minScale">最小の大きさ</param>
/// <param name="maxScale">最大の大きさ</param>
/// <param name="speed">拡縮のスピード 低いほど遅い 0.5くらいが標準くらい</param>
void DrawComponent2D::StartScaleContinuous(float minScale, float maxScale, float speed) {
	scaleEffect_.isActive = true;
	scaleEffect_.minScale = minScale;
	scaleEffect_.maxScale = maxScale;
	scaleEffect_.speed = speed;
	scaleEffect_.continuous = true;
	scaleEffect_.expanding = true;
	scaleEffect_.currentScale = minScale;
}

void DrawComponent2D::StopScale() {
	scaleEffect_.isActive = false;
	scaleEffect_.currentScale = 1.0f;
}

void DrawComponent2D::UpdateScale(float deltaTime) {
	if (!scaleEffect_.isActive) {
		return;
	}

	if (scaleEffect_.expanding) {
		scaleEffect_.currentScale += scaleEffect_.speed * deltaTime;
		if (scaleEffect_.currentScale >= scaleEffect_.maxScale) {
			scaleEffect_.currentScale = scaleEffect_.maxScale;
			scaleEffect_.expanding = false;
		}
	}
	else {
		scaleEffect_.currentScale -= scaleEffect_.speed * deltaTime;
		if (scaleEffect_.currentScale <= scaleEffect_.minScale) {
			scaleEffect_.currentScale = scaleEffect_.minScale;
			scaleEffect_.expanding = true;

			if (!scaleEffect_.continuous) {
				scaleEffect_.currentCount++;
				if (scaleEffect_.currentCount >= scaleEffect_.maxCount) {
					StopScale();
				}
			}
		}
	}
}

// 縮小消滅エフェクトの実装
void DrawComponent2D::StartScaleExtinction(float duration, int easingType, bool fadeWithScale) {
	StartScaleExtinction(1.0f, duration, easingType, fadeWithScale);
}

void DrawComponent2D::StartScaleExtinction(float startScale, float duration, int easingType, bool fadeWithScale) {
	scaleExtinctionEffect_.isActive = true;
	scaleExtinctionEffect_.duration = duration;
	scaleExtinctionEffect_.elapsed = 0.0f;
	scaleExtinctionEffect_.startScale = startScale;
	scaleExtinctionEffect_.endScale = 0.0f;
	scaleExtinctionEffect_.easingType = easingType;
	scaleExtinctionEffect_.fadeWithScale = fadeWithScale;
	scaleExtinctionEffect_.currentScale = startScale;

	// 開始色を保存
	scaleExtinctionEffect_.startColor = ColorRGBA::FromUInt(GetEffectAppliedColor());

	// スケールを初期化
	scale_ = { startScale, startScale };
}

void DrawComponent2D::StopScaleExtinction() {
	scaleExtinctionEffect_.isActive = false;
	scaleExtinctionEffect_.elapsed = 0.0f;
}

void DrawComponent2D::UpdateScaleExtinction(float deltaTime) {
	if (!scaleExtinctionEffect_.isActive) return;

	scaleExtinctionEffect_.elapsed += deltaTime;

	// 進行度を計算（0.0 ~ 1.0）
	float t = scaleExtinctionEffect_.elapsed / scaleExtinctionEffect_.duration;
	if (t > 1.0f) {
		t = 1.0f;
		// 完全に消滅したら非アクティブ化（後で削除できるように）
	}

	// イージングを適用
	float easedT = ApplyEasing(t, scaleExtinctionEffect_.easingType);

	// スケールを補間
	scaleExtinctionEffect_.currentScale =
		scaleExtinctionEffect_.startScale * (1.0f - easedT) +
		scaleExtinctionEffect_.endScale * easedT;

	// スケールを適用
	scale_.x = scaleExtinctionEffect_.currentScale;
	scale_.y = scaleExtinctionEffect_.currentScale;

	// フェードも同時に行う場合
	if (scaleExtinctionEffect_.fadeWithScale) {
		ColorRGBA targetColor = scaleExtinctionEffect_.startColor;
		targetColor.a = scaleExtinctionEffect_.startColor.a * (1.0f - easedT);

		// FadeEffectを使わずに直接色を設定
		graph_.color = targetColor.ToUInt();
	}
}

// イージング関数
float DrawComponent2D::ApplyEasing(float t, int easingType) const {
	switch (easingType) {
	case 0: // Linear
		return t;

	case 1: // EaseOut (減速)
		return 1.0f - (1.0f - t) * (1.0f - t);

	case 2: // EaseIn (加速)
		return t * t;

	case 3: // EaseInOut (加速→減速)
		if (t < 0.5f) {
			return 2.0f * t * t;
		}
		else {
			float t2 = 1.0f - t;
			return 1.0f - 2.0f * t2 * t2;
		}

	default:
		return t;
	}
}

// ========== エフェクトリセット ==========
void DrawComponent2D::ResetAllEffects() {
	StopShake();
	StopRotation();
	squashStretchEffect_.isActive = false;
	StopFade();
	StopScale();
}

// ========== エフェクト適用 ==========
Vector2 DrawComponent2D::GetEffectAppliedPosition() const {
	Vector2 pos = position_;
	if (shakeEffect_.isActive) {
		pos.x += shakeEffect_.offset.x;
		pos.y += shakeEffect_.offset.y;
	}
	return pos;
}

Vector2 DrawComponent2D::GetEffectAppliedScale() const {
	Vector2 scale = scale_;

	if (squashStretchEffect_.isActive) {
		float t = squashStretchEffect_.elapsed / squashStretchEffect_.duration;
		if (squashStretchEffect_.expanding) {
			scale.x *= 1.0f + (squashStretchEffect_.scale.x - 1.0f) * t;
			scale.y *= 1.0f + (squashStretchEffect_.scale.y - 1.0f) * t;
		}
		else {
			scale.x *= squashStretchEffect_.scale.x - (squashStretchEffect_.scale.x - 1.0f) * t;
			scale.y *= squashStretchEffect_.scale.y - (squashStretchEffect_.scale.y - 1.0f) * t;
		}
	}

	if (scaleEffect_.isActive) {
		scale.x *= scaleEffect_.currentScale;
		scale.y *= scaleEffect_.currentScale;
	}

	return scale;
}

float DrawComponent2D::GetEffectAppliedAngle() const {
	float angle = angle_;
	if (rotationEffect_.isActive) {
		angle += rotationEffect_.accumulatedAngle;
	}
	return angle;
}

unsigned int DrawComponent2D::GetEffectAppliedColor() const {
	unsigned int color = graph_.color;

	if (fadeEffect_.isActive) {
		// フェード効果が有効な場合、現在の補間色を使用
		color = fadeEffect_.currentColor.ToUInt();
	}

	return color;
}

Matrix3x3 DrawComponent2D::GetEffectAppliedMatrix() const {
	Vector2 effectPos = GetEffectAppliedPosition();
	Vector2 effectScale = GetEffectAppliedScale();
	float effectAngle = GetEffectAppliedAngle();

	return AffineMatrix2D::MakeAffine(effectScale, effectAngle, effectPos);
}

// ========== 描画メソッド（既存） ==========
void DrawComponent2D::DrawCamera(const Camera2D& camera) {
	Matrix3x3 affineMatrix = GetEffectAppliedMatrix();
	Matrix3x3 wvpVpMatrix = Matrix3x3::Multiply(affineMatrix, camera.vpVpMatrix);
	Vertex4 screenVertex = vertex_.Transform(vertex_.localVertex, wvpVpMatrix);
	vertex_.DrawVertexQuad(screenVertex, graph_.handle, graph_.grDrawWidth, graph_.grDrawHeight, GetEffectAppliedColor());
}

void DrawComponent2D::DrawAnimationCamera(const Camera2D& camera) {
	if (!animation_ || !animation_->IsPlaying()) {
		DrawCamera(camera);
		return;
	}

	Matrix3x3 affineMatrix = GetEffectAppliedMatrix();
	Matrix3x3 wvpVpMatrix = Matrix3x3::Multiply(affineMatrix, camera.vpVpMatrix);
	Vertex4 screenVertex = vertex_.Transform(vertex_.localVertex, wvpVpMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenVertex.leftTop.x), static_cast<int>(screenVertex.leftTop.y),
		static_cast<int>(screenVertex.rightTop.x), static_cast<int>(screenVertex.rightTop.y),
		static_cast<int>(screenVertex.leftBottom.x), static_cast<int>(screenVertex.leftBottom.y),
		static_cast<int>(screenVertex.rightBottom.x), static_cast<int>(screenVertex.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawQuadWorld() {
	Matrix3x3 affineMatrix = GetEffectAppliedMatrix();
	Vertex4 localV = vertex_.Transform(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(localV.leftTop.x), static_cast<int>(localV.leftTop.y),
		static_cast<int>(localV.rightTop.x), static_cast<int>(localV.rightTop.y),
		static_cast<int>(localV.leftBottom.x), static_cast<int>(localV.leftBottom.y),
		static_cast<int>(localV.rightBottom.x), static_cast<int>(localV.rightBottom.y),
		0, 0,
		graph_.grDrawWidth, graph_.grDrawHeight,
		graph_.handle,
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationWorld() {
	if (!animation_->IsPlaying()) {
		DrawQuadWorld();
		return;
	}

	Matrix3x3 affineMatrix = GetEffectAppliedMatrix();
	Vertex4 localV = vertex_.Transform(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(localV.leftTop.x), static_cast<int>(localV.leftTop.y),
		static_cast<int>(localV.rightTop.x), static_cast<int>(localV.rightTop.y),
		static_cast<int>(localV.leftBottom.x), static_cast<int>(localV.leftBottom.y),
		static_cast<int>(localV.rightBottom.x), static_cast<int>(localV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreen() {
	if (!animation_->IsPlaying()) {
		DrawQuadWorld();
		return;
	}

	Matrix3x3 affineMatrix = GetEffectAppliedMatrix();
	Vertex4 screenV = vertex_.TransformScreen(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreen(int grHandle) {
	if (!animation_->IsPlaying()) {
		DrawQuadWorld();
		return;
	}

	Matrix3x3 affineMatrix = GetEffectAppliedMatrix();
	Vertex4 screenV = vertex_.TransformScreen(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		grHandle,
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreen(unsigned int color) {
	if (!animation_->IsPlaying()) {
		DrawQuadWorld();
		return;
	}

	Matrix3x3 affineMatrix = GetEffectAppliedMatrix();
	Vertex4 screenV = vertex_.TransformScreen(vertex_.localVertex, affineMatrix);

	unsigned int effectColor = GetEffectAppliedColor();
	// RGB成分は引数の色、アルファ成分はエフェクト適用
	unsigned int finalColor = (color & 0x00FFFFFF) | (effectColor & 0xFF000000);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		finalColor
	);
}

void DrawComponent2D::DrawAnimationScreenWorldToScreen(const Vector2& offset) {
	if (!animation_ || !animation_->IsPlaying()) {
		return;
	}

	Vector2 effectPos = GetEffectAppliedPosition();
	float screenX = effectPos.x - offset.x;
	float screenY = kWindowHeight - (effectPos.y + height_ / 2.0f - offset.y);
	Vector2 screenPosition = { screenX, screenY };

	Vector2 effectScale = GetEffectAppliedScale();
	float effectAngle = GetEffectAppliedAngle();

	Matrix3x3 affineMatrix = AffineMatrix2D::MakeAffine(effectScale, effectAngle, screenPosition);
	Vertex4 screenV = vertex_.Transform(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreenWorldToScreen(const Vector2& offset, int grHandle) {
	if (!animation_ || !animation_->IsPlaying()) {
		return;
	}

	Vector2 effectPos = GetEffectAppliedPosition();
	float screenX = effectPos.x - offset.x;
	float screenY = kWindowHeight - (effectPos.y + height_ / 2.0f - offset.y);
	Vector2 screenPosition = { screenX, screenY };

	Vector2 effectScale = GetEffectAppliedScale();
	float effectAngle = GetEffectAppliedAngle();

	Matrix3x3 affineMatrix = AffineMatrix2D::MakeAffine(effectScale, effectAngle, screenPosition);
	Vertex4 screenV = vertex_.Transform(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		grHandle,
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreenWorldToScreenReverse(const Vector2& offset) {
	if (!animation_ || !animation_->IsPlaying()) {
		return;
	}

	Vector2 effectPos = GetEffectAppliedPosition();
	float screenX = effectPos.x - offset.x;
	float screenY = kWindowHeight - (effectPos.y + height_ / 2.0f - offset.y);
	Vector2 screenPosition = { screenX, screenY };

	Vector2 effectScale = GetEffectAppliedScale();
	float effectAngle = GetEffectAppliedAngle();

	Matrix3x3 affineMatrix = AffineMatrix2D::MakeAffine(effectScale, effectAngle, screenPosition);
	Vertex4 screenV = vertex_.Transform(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreenWorldToScreenReverse(const Vector2& offset, int grHandle) {
	if (!animation_ || !animation_->IsPlaying()) {
		return;
	}

	Vector2 effectPos = GetEffectAppliedPosition();
	float screenX = effectPos.x - offset.x;
	float screenY = kWindowHeight - (effectPos.y + height_ / 2.0f - offset.y);
	Vector2 screenPosition = { screenX, screenY };

	Vector2 effectScale = GetEffectAppliedScale();
	float effectAngle = GetEffectAppliedAngle();

	Matrix3x3 affineMatrix = AffineMatrix2D::MakeAffine(effectScale, effectAngle, screenPosition);
	Vertex4 screenV = vertex_.Transform(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		grHandle,
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreenWorldToScreen(const Vector2& worldPosition, const Vector2& offset) {
	if (!animation_ || !animation_->IsPlaying()) {
		return;
	}

	position_ = worldPosition;
	Vector2 effectPos = GetEffectAppliedPosition();

	float screenX = effectPos.x - offset.x;
	float screenY = kWindowHeight - (effectPos.y + height_ / 2.0f - offset.y);
	Vector2 screenPosition = { screenX, screenY };

	Vector2 effectScale = GetEffectAppliedScale();
	float effectAngle = GetEffectAppliedAngle();

	Matrix3x3 affineMatrix = AffineMatrix2D::MakeAffine(effectScale, effectAngle, screenPosition);
	Vertex4 screenV = vertex_.Transform(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		GetEffectAppliedColor()
	);
}

void DrawComponent2D::DrawAnimationScreen(const Vector2& offset) {
	if (!animation_ || !animation_->IsPlaying()) {
		DrawQuadWorld();
		return;
	}

	Vector2 effectPos = GetEffectAppliedPosition();
	Vector2 offsetPosition = Vector2::Add(effectPos, Vector2{ -offset.x, offset.y });

	Vector2 effectScale = GetEffectAppliedScale();
	float effectAngle = GetEffectAppliedAngle();

	Matrix3x3 affineMatrix = AffineMatrix2D::MakeAffine(effectScale, effectAngle, offsetPosition);
	Vertex4 screenV = vertex_.TransformScreen(vertex_.localVertex, affineMatrix);

	Novice::DrawQuad(
		static_cast<int>(screenV.leftTop.x), static_cast<int>(screenV.leftTop.y),
		static_cast<int>(screenV.rightTop.x), static_cast<int>(screenV.rightTop.y),
		static_cast<int>(screenV.leftBottom.x), static_cast<int>(screenV.leftBottom.y),
		static_cast<int>(screenV.rightBottom.x), static_cast<int>(screenV.rightBottom.y),
		animation_->GetSrcX(), animation_->GetSrcY(),
		animation_->GetSrcW(), animation_->GetSrcH(),
		animation_->GetGraphHandle(),
		GetEffectAppliedColor()
	);
}

// ========== アニメーション状態取得 ==========
bool DrawComponent2D::HasAnimation() const {
	return animation_ != nullptr;
}

bool DrawComponent2D::IsAnimationPlaying() const {
	return animation_->IsPlaying();
}

// Debug用 color関連
void DrawComponent2D::DrawDebugWindow(const char* title) {
	title;

#ifdef _DEBUG
	ImGui::Begin(title);
	ImGui::Text("Position: (%.2f, %.2f)", position_.x, position_.y);
	ImGui::Text("Scale: (%.2f, %.2f)", scale_.x, scale_.y);
	ImGui::Text("Angle: %.2f", angle_);
	ImGui::Text("Width: %.2f", width_);
	ImGui::Text("Height: %.2f", height_);
	ImGui::ColorEdit4("Graph Color", reinterpret_cast<float*>(&graph_.color));

	ImGui::Text("R :%.2f", &fadeEffect_.currentColor.r);
	ImGui::Text("G :%.2f", &fadeEffect_.currentColor.g);
	ImGui::Text("B :%.2f", &fadeEffect_.currentColor.b);
	ImGui::Text("A :%.2f", &fadeEffect_.currentColor.a);

	ImGui::End();
#endif
}