#include "RightBottomHand.h"

RightBottomHand::RightBottomHand() {
	gh_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
	offsetAngle_ = 45.0f;
	orbitAngle_ = 180.0f;
}

void RightBottomHand::Initialize(Vector2 bossCenter, float offset, float bossAngle) {

	BossParts::Initialize(bossCenter, offset, bossAngle);
}

void RightBottomHand::Update(float dt, Vector2 bossCenter, float bossAngle, float offset) {
	orbitAngle_ += orbitSpeed_;

	BossParts::RotateParts();

	BossParts::Update(dt, bossCenter, bossAngle, offset);
}

void RightBottomHand::Draw()const {
	BossParts::Draw();
}