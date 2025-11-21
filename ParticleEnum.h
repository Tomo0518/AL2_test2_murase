#pragma once

/// <summary>
/// パーティクルの「挙動」の種類
/// マネージャー側で「爆発」などの種類を選んだ際、内部的にこれが設定される
/// </summary>
enum class ParticleBehavior {
	Physics,    // 物理挙動（速度・重力・回転あり：デブリなど）
	Stationary, // その場に留まる（アニメーションのみ：爆発など）
	Ghost       // 残像（動きなし、その場でフェードアウト：ダッシュ残像）
};

// エフェクトの種類
enum class ParticleType {
	Explosion,
	Debris,
	Hit,
	Dust,
	MuzzleFlash,
};
