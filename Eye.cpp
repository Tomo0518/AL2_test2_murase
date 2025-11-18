#include "Eye.h"
#include<math.h>

Eye::Eye() {
	gh_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
	offsetAngle_ = 45.0f;
	orbitAngle_ = 180.0f;
	orbitRadius_ = 60.0f;
	width_ = 80.0f;
	height_ = 80.0f;
}

void Eye::Initialize(Vector2 bossCenter, float offset, float bossAngle) {

	BossParts::Initialize(bossCenter, offset, bossAngle);
}

void Eye::Update(float dt, Vector2 bossCenter, float bossAngle, float offset) {

	BossParts::Update(dt, bossCenter, bossAngle, offset);
}

void Eye::Draw()const {
	BossParts::Draw();
}

void Eye::Follow(Vector2 playerCenter) {
	float angle = atan2f(playerCenter.y - orbitCenter_.y, playerCenter.x - orbitCenter_.x);

	center_ = { cosf(angle) * orbitRadius_  + orbitCenter_.x,sinf(angle) * orbitRadius_ + orbitCenter_.y };
}