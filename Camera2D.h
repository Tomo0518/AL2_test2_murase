#pragma once
#include "Vector2.h"
#include "Matrix3x3.h"
#include "WindowSize.h"

class Camera2D {
public:
	Vector2 position;
	Vector2 translate;
	float rotation;
	Vector2 scale;
	float width, height;
	float orthoLeft, orthoTop, orthoRight, orthoBottom;
	float vpLeft, vpTop, vpWidth, vpHeight;

	Matrix3x3 viewMatrix;
	Matrix3x3 projectionMatrix;
	Matrix3x3 viewportMatrix;
	Matrix3x3 vpVpMatrix;

	Camera2D();

	void Move(const char* keys);

	void Update();
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
	void UpdateViewportMatrix();
	void UpdateVpVpMatrix();
	void MakeOrthoGraphicMatrix();
	void MakeViewportMatrix();

	Matrix3x3 GetVpVpMatrix() const { return vpVpMatrix; }
};