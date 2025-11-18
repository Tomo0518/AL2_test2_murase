#include "RightHand.h"

RightHand::RightHand() {
	gh_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
	offsetAngle_ = 0.0f;
	orbitAngle_ = 180.0f;
}

void RightHand::Initialize(Vector2 bossCenter, float offset, float bossAngle) {

	BossParts::Initialize(bossCenter, offset, bossAngle);
}

void RightHand::Update(float dt, Vector2 bossCenter, float bossAngle, float offset) {
	orbitAngle_ += orbitSpeed_;

	BossParts::RotateParts();

	BossParts::Update(dt, bossCenter, bossAngle, offset);
}

void RightHand::Draw()const {
	BossParts::Draw();
}