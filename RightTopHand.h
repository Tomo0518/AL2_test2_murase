#pragma once
#include "BossParts.h"

class RightTopHand :public BossParts{

public:
	RightTopHand();
	void Initialize(Vector2 bossCenter, float offset, float bossAngle) override;
	void Update(float dt, Vector2 bossCenter, float bossAngle, float offset)override;
	void Draw()const override;
};

