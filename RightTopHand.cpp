#include "RightTopHand.h"

RightTopHand::RightTopHand() {
	gh_ = Novice::LoadTexture("./Resources/images/gamePlay/playerSpecial_ver1.png");
	offsetAngle_ = 315.0f;
	orbitAngle_ = 180.0f;
	//orbitRadius_ = 0.0f;
}

void RightTopHand::Initialize(Vector2 bossCenter, float offset, float bossAngle) {

	BossParts::Initialize(bossCenter, offset, bossAngle);
}

void RightTopHand::Update(float dt, Vector2 bossCenter, float bossAngle, float offset) {
	orbitAngle_ += orbitSpeed_;

	BossParts::RotateParts();

	BossParts::Update(dt, bossCenter, bossAngle, offset);
}

void RightTopHand::Draw()const {
	BossParts::Draw();
}