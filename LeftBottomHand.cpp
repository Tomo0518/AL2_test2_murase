#include "LeftBottomHand.h"

LeftBottomHand::LeftBottomHand() {
	gh_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
	offsetAngle_ = 135.0f;
}

void LeftBottomHand::Initialize(Vector2 bossCenter, float offset, float bossAngle) {

	BossParts::Initialize(bossCenter, offset, bossAngle);
}

void LeftBottomHand::Update(float dt, Vector2 bossCenter, float bossAngle, float offset) {
	orbitAngle_ -= orbitSpeed_;

	BossParts::RotateParts();

	BossParts::Update(dt, bossCenter, bossAngle, offset);
}

void LeftBottomHand::Draw()const {
	BossParts::Draw();
}