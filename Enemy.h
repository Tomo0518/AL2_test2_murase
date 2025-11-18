#pragma once

class Enemy {
public:
	Enemy() = default;
	~Enemy() = default;
	void Initialize();
	void Update(float dt);
	void Draw();
};