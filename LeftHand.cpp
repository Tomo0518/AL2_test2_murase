#include "LeftHand.h"

LeftHand::LeftHand() {
	gh_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
	offsetAngle_ = 180.0f;
}

void LeftHand::Initialize(Vector2 bossCenter, float offset, float bossAngle) {

	BossParts::Initialize(bossCenter, offset, bossAngle);
}

void LeftHand::Update(float dt, Vector2 bossCenter, float bossAngle, float offset) {
	orbitAngle_ -= orbitSpeed_;

	BossParts::RotateParts();

	BossParts::Update(dt, bossCenter, bossAngle, offset);
}

void LeftHand::Draw()const {
	BossParts::Draw();
}