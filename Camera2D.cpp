#include "Camera2D.h"
#include "Affine2D.h"
#include <Novice.h>
#include "WindowSize.h"

Camera2D::Camera2D()
	:position{ kWindowWidth / 2.0f, kWindowHeight / 2.0f }
	, rotation(0.0f)
	, scale{ 1.0f, 1.0f }
	, width(kWindowWidth)
	, height(kWindowHeight)
	, orthoLeft(-kWindowWidth / 2.0f)
	, orthoTop(kWindowHeight / 2.0f)
	, orthoRight(kWindowWidth / 2.0f)
	, orthoBottom(-kWindowHeight / 2.0f)
	, vpLeft(0.0f)
	, vpTop(0.0f)
	, vpWidth(kWindowWidth)
	, vpHeight(kWindowHeight)
{
	Update();
}

// カメラ操作: WASDで移動、Eで拡大、Qで縮小
void Camera2D::Move(const char* keys) {
	const float moveSpeed = 10.0f;
	const float scaleSpeed = 0.05f;

	if (keys[DIK_UP]) {
		position.y += moveSpeed;
	}

	if (keys[DIK_DOWN]) {
		position.y -= moveSpeed;
	}

	if (keys[DIK_LEFT]) {
		position.x -= moveSpeed;
	}

	if (keys[DIK_RIGHT]) {
		position.x += moveSpeed;
	}

	if (keys[DIK_E]) {
		scale.x += scaleSpeed;
		scale.y += scaleSpeed;
	}

	if (keys[DIK_Q]) {
		scale.x -= scaleSpeed;
		scale.y -= scaleSpeed;
	}

	if (keys[DIK_R]) {
		rotation += 0.02f;
	}

	if (keys[DIK_F]) {
		rotation -= 0.02f;
	}

	if (keys[DIK_Y]) {
		position = { kWindowWidth / 2.0f, kWindowHeight / 2.0f };
		rotation = 0.0f;
		scale = { 1.0f, 1.0f };
	}

	// 拡大縮小の下限を設定
	if (scale.x < 0.1f) scale.x = 0.1f;
	if (scale.y < 0.1f) scale.y = 0.1f;

	Update();
}

// 正射影行列を作成する
void Camera2D::MakeOrthoGraphicMatrix() {
	Matrix3x3 ortho;

	ortho.m[0][0] = 2.0f / (orthoRight - orthoLeft);
	ortho.m[0][1] = 0.0f;
	ortho.m[0][2] = 0.0f;

	ortho.m[1][0] = 0.0f;
	ortho.m[1][1] = 2.0f / (orthoTop - orthoBottom);
	ortho.m[1][2] = 0.0f;

	ortho.m[2][0] = (orthoLeft + orthoRight) / (orthoLeft - orthoRight);
	ortho.m[2][1] = (orthoTop + orthoBottom) / (orthoBottom - orthoTop);
	ortho.m[2][2] = 1.0f;

	projectionMatrix = ortho;
}

void Camera2D::MakeViewportMatrix() {
	Matrix3x3 viewport;

	viewport.m[0][0] = vpWidth / 2.0f;
	viewport.m[0][1] = 0.0f;
	viewport.m[0][2] = 0.0f;

	viewport.m[1][0] = 0.0f;
	viewport.m[1][1] = -(vpHeight / 2.0f);
	viewport.m[1][2] = 0.0f;

	viewport.m[2][0] = vpLeft + vpWidth / 2.0f;
	viewport.m[2][1] = vpTop + vpHeight / 2.0f;
	viewport.m[2][2] = 1.0f;

	viewportMatrix = viewport;
}

void Camera2D::UpdateVpVpMatrix() {
	Matrix3x3 cameraAffine = AffineMatrix2D::MakeAffine(scale, rotation, position);
	viewMatrix = Matrix3x3::Inverse(cameraAffine);
	MakeOrthoGraphicMatrix();
	MakeViewportMatrix();
	vpVpMatrix = Matrix3x3::Multiply(Matrix3x3::Multiply(viewMatrix, projectionMatrix), viewportMatrix);
}

// ビュー行列の更新
void Camera2D::UpdateViewMatrix() {
	// カメラのスケール・回転・位置からアフィン行列を作成し、逆行列をビュー行列とする
	Matrix3x3 cameraAffine = AffineMatrix2D::MakeAffine(scale, rotation, position);
	viewMatrix = Matrix3x3::Inverse(cameraAffine);
}

// 射影行列の更新
void Camera2D::UpdateProjectionMatrix() {
	// カメラの投影範囲から正射影行列を作成
	MakeOrthoGraphicMatrix();
}

// ビューポート行列の更新
void Camera2D::UpdateViewportMatrix() {
	// カメラのビューポート範囲からビューポート行列を作成
	MakeViewportMatrix();
}

void Camera2D::Update() {
	UpdateViewMatrix();
	UpdateProjectionMatrix();
	UpdateViewportMatrix();
	UpdateVpVpMatrix();
}
