#include "LeftTopHand.h"

LeftTopHand::LeftTopHand() {
	gh_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
	offsetAngle_ = 225.0f;
}

void LeftTopHand::Initialize(Vector2 bossCenter, float offset, float bossAngle) {

	BossParts::Initialize(bossCenter, offset, bossAngle);
}

void LeftTopHand::Update(float dt, Vector2 bossCenter, float bossAngle, float offset) {
	orbitAngle_ -= orbitSpeed_;

	BossParts::RotateParts();

	BossParts::Update(dt, bossCenter, bossAngle, offset);
}

void LeftTopHand::Draw()const {
	BossParts::Draw();
}