#include "ScrapManager.h"
#include <cmath>
#include <algorithm>
#include <chrono>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

ScrapManager::ScrapManager() {
	auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
	randomEngine_.seed(seed);
}

void ScrapManager::Initialize() {
	scraps_.clear();
	scraps_.reserve(kMaxScraps);
	heldWeight_ = 0.0f;
	heldCount_ = 0;
}

void ScrapManager::Update(float dt, const Vector2& vaccumPos, bool isSucking) {

	// 保持中のスクラップ数と重量を計算
	heldWeight_ = 0.0f;
	heldCount_ = 0;

	for (auto& scrap : scraps_) {
		if (!scrap->IsActive()) {
			continue;
		}

		scrap->Update(dt);

		// 保持中のスクラップを集計
		if (scrap->GetState() == ScrapState::Held) {
			heldWeight_ += scrap->GetWeight();
			heldCount_++;
		}
	}

	// 吸引中・保持中の衝突解決
	if (isSucking || heldCount_ > 0) {
		ResolveCollisions(vaccumPos);
	}

	// 保持中のスクラップを整列（vaccumPos周辺に配置）
	if (heldCount_ > 0) {
		ArrangeHeldScraps(vaccumPos);
	}

	// 画面外のスクラップを削除
	RemoveOutOfBoundsScraps({ 1280.0f, 720.0f }, kOutOfBoundsMargin);
}

void ScrapManager::Draw(const Vector2& scrollOffset) {
	for (auto& scrap : scraps_) {
		if (scrap->IsActive()) {
			scrap->Draw(scrollOffset);
		}
	}
}

// ========================================
// スクラップ生成系
// ========================================

Scrap* ScrapManager::CreateScrap(ScrapType type, ScrapTrait trait, const Vector2& position, const Vector2& velocity) {
	if (scraps_.size() >= kMaxScraps) {
		return nullptr;
	}

	auto scrap = std::make_unique<Scrap>();
	scrap->Initialize(type, trait, position, velocity);

	Scrap* ptr = scrap.get();
	scraps_.push_back(std::move(scrap));

	return ptr;
}

// 指定位置にスクラップを生成（ボスの供給ポイント用）
void ScrapManager::SpawnScrap(ScrapType type, const Vector2& position, const Vector2& initialVelocity) {
	// 非アクティブなスクラップを探す
	for (auto& scrap : scraps_) {
		if (!scrap->IsActive()) {
			// スクラップを初期化して使用
			scrap->Initialize(type, ScrapTrait::Normal, position, initialVelocity);
			scrap->SetState(ScrapState::Free);  // 自由落下状態で生成
			return;
		}
	}

	// 全て使用中の場合は新規作成
	auto newScrap = std::make_unique<Scrap>();
	newScrap->Initialize(type, ScrapTrait::Normal, position, initialVelocity);
	newScrap->SetState(ScrapState::Free);
	scraps_.push_back(std::move(newScrap));
}

// 円形にスクラップを生成
void ScrapManager::SpawnScrapCircle(const Vector2& center, int count, float radius, ScrapType type, float spreadSpeed) {
	std::vector<Vector2> positions;
	std::vector<float> radii;

	float scrapRadius = 16.0f;
	switch (type) {
	case ScrapType::Small:  scrapRadius = 16.0f; break;
	case ScrapType::Medium: scrapRadius = 24.0f; break;
	case ScrapType::Large:  scrapRadius = 32.0f; break;
	}

	for (int i = 0; i < count; ++i) {
		float angle = (2.0f * 3.14159265f * i) / count;
		Vector2 position = {
			center.x + std::cos(angle) * radius,
			center.y + std::sin(angle) * radius
		};

		position = FindNonOverlappingPosition(position, scrapRadius, radius * 0.5f, positions, radii);

		Vector2 direction = {
			std::cos(angle),
			std::sin(angle)
		};
		Vector2 velocity = {
			direction.x * spreadSpeed,
			direction.y * spreadSpeed
		};

		CreateScrap(type, ScrapTrait::Normal, position, velocity);
		positions.push_back(position);
		radii.push_back(scrapRadius);
	}
}

// ランダム位置にスクラップを生成
void ScrapManager::SpawnScrapRandom(const Vector2& center, int count, float minRadius, float maxRadius, ScrapType type) {
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
	std::uniform_real_distribution<float> radiusDist(minRadius, maxRadius);

	std::vector<Vector2> positions;
	std::vector<float> radii;

	float scrapRadius = 16.0f;
	switch (type) {
	case ScrapType::Small:  scrapRadius = 16.0f; break;
	case ScrapType::Medium: scrapRadius = 24.0f; break;
	case ScrapType::Large:  scrapRadius = 32.0f; break;
	}

	for (int i = 0; i < count; ++i) {
		float angle = angleDist(randomEngine_);
		float r = radiusDist(randomEngine_);

		Vector2 position = {
			center.x + std::cos(angle) * r,
			center.y + std::sin(angle) * r
		};

		position = FindNonOverlappingPosition(position, scrapRadius, maxRadius * 0.3f, positions, radii);

		CreateScrap(type, ScrapTrait::Normal, position, { 0.0f, 0.0f });
		positions.push_back(position);
		radii.push_back(scrapRadius);
	}
}

// 爆発的にスクラップを生成
void ScrapManager::SpawnScrapExplosion(const Vector2& center, int count, ScrapType type, float explosionForce) {
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
	std::uniform_real_distribution<float> forceDist(explosionForce * 0.7f, explosionForce * 1.3f);

	for (int i = 0; i < count; ++i) {
		float angle = angleDist(randomEngine_);
		float force = forceDist(randomEngine_);

		Vector2 direction = {
			std::cos(angle),
			std::sin(angle)
		};

		Vector2 velocity = {
			direction.x * force,
			direction.y * force
		};

		CreateScrap(type, ScrapTrait::Normal, center, velocity);
	}
}

// 大小混合スクラップ生成
void ScrapManager::SpawnScrapExplosionKinds(const Vector2& center, int maxCount, int bigSizeCount, ScrapGenerateSize generateSize, float explosionForce, int midSizeCount) {
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
	std::uniform_real_distribution<float> forceDist(explosionForce * 0.7f, explosionForce * 1.3f);

	switch (generateSize) {
	case ScrapGenerateSize::SmallAndMedium:

		for (int i = 0; i < maxCount - bigSizeCount; ++i) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);

			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};

			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};

			CreateScrap(ScrapType::Small, ScrapTrait::Normal, center, velocity);
		}

		for (int i = 0; i < bigSizeCount; ++i) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);
			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};
			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};
			CreateScrap(ScrapType::Medium, ScrapTrait::Normal, center, velocity);
		}

		break;
	case ScrapGenerateSize::SmallAndLarge:

		for (int i = 0; i < maxCount - bigSizeCount; i++) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);

			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};

			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};

			CreateScrap(ScrapType::Small, ScrapTrait::Normal, center, velocity);
		}

		for (int i = 0; i < bigSizeCount; ++i) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);
			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};
			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};
			CreateScrap(ScrapType::Large, ScrapTrait::Normal, center, velocity);
		}

		break;
	case ScrapGenerateSize::MediumAndLarge:
		for (int i = 0; i < maxCount - bigSizeCount; i++) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);
			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};
			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};
			CreateScrap(ScrapType::Medium, ScrapTrait::Normal, center, velocity);
		}

		for (int i = 0; i < bigSizeCount; ++i) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);
			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};
			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};
			CreateScrap(ScrapType::Large, ScrapTrait::Normal, center, velocity);
		}
		break;
	case ScrapGenerateSize::SmallAndMediumAndLarge:
		for (int i = 0; i < maxCount - bigSizeCount - midSizeCount; i++) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);
			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};
			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};
			CreateScrap(ScrapType::Small, ScrapTrait::Normal, center, velocity);
		}

		for (int i = 0; i < midSizeCount; ++i) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);
			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};
			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};
			CreateScrap(ScrapType::Medium, ScrapTrait::Normal, center, velocity);
		}

		for (int i = 0; i < bigSizeCount; ++i) {
			float angle = angleDist(randomEngine_);
			float force = forceDist(randomEngine_);
			Vector2 direction = {
				std::cos(angle),
				std::sin(angle)
			};
			Vector2 velocity = {
				direction.x * force,
				direction.y * force
			};
			CreateScrap(ScrapType::Large, ScrapTrait::Normal, center, velocity);
		}
		break;

	default:
		break;
	}
}

// ========================================
// 保持半径計算
// ========================================
float ScrapManager::CalculateMaxHeldRadius(int heldCount) const {
	if (heldCount <= 0) {
		return 0.0f;
	}

	if (heldCount == 1) {
		return 0.0f;  // 中心に配置
	}

	if (heldCount <= 3) {
		return kHeldOrbitRadiusBase * 0.3f;
	}

	// 4個以降は層状配置の最外層を計算
	const int scrapsPerLayer = 6;
	int remainingScraps = heldCount - 3;  // 中心の3個を除く
	int layer = 0;

	while (remainingScraps > 0) {
		int scrapsInThisLayer = std::min(scrapsPerLayer * (layer + 1), remainingScraps);
		remainingScraps -= scrapsInThisLayer;
		if (remainingScraps > 0) {
			layer++;
		}
	}

	return kHeldOrbitRadiusBase + (layer * kHeldOrbitRadiusStep);
}

// ========================================
// 吸引処理
// ========================================
void ScrapManager::ProcessSuction(const Vector2& vaccumPos, float vaccumRadius, float playerWeight, float maxWeight) {
	// 動的な保持移行判定距離を計算
	float holdTransitionRadius = holdTransitionMinRadius_;

	if (useAdvancedHoldTransition_) {
		// 現在の保持数に基づいて最大半径を計算
		float maxRadius = CalculateMaxHeldRadius(heldCount_);

		// 最外層の指定された割合を判定距離とする
		holdTransitionRadius = maxRadius * holdTransitionRadiusRatio_;

		// 上限・下限でクランプ
		holdTransitionRadius = std::max(holdTransitionMinRadius_, holdTransitionRadius);
		holdTransitionRadius = std::min(holdTransitionMaxRadius_, holdTransitionRadius);
	}

	// 吸引中のスクラップが範囲外に出た場合のチェック
	for (auto& scrap : scraps_) {
		if (!scrap->IsActive()) {
			continue;
		}

		if (scrap->GetState() == ScrapState::BeingSucked) {
			Vector2 toVaccum = vaccumPos - scrap->GetPosition();
			float distance = std::sqrt(toVaccum.x * toVaccum.x + toVaccum.y * toVaccum.y);

			// 保持移行判定（動的距離を使用）
			if (distance < holdTransitionRadius) {
				scrap->SetState(ScrapState::Held);
				scrap->SetVelocity({ 0.0f, 0.0f });
				continue;
			}

			// 吸引範囲外に出た場合、Free状態に戻す
			if (distance > vaccumRadius) {
				scrap->SetState(ScrapState::Free);
				// 速度を大幅に減衰させる
				Vector2 currentVel = scrap->GetVelocity();
				scrap->SetVelocity({ currentVel.x * 0.1f, currentVel.y * 0.1f });
			}
		}
	}

	// Free状態のスクラップを吸引範囲内に入れる
	for (auto& scrap : scraps_) {
		if (!scrap->IsActive()) {
			continue;
		}

		if (scrap->GetState() != ScrapState::Free) {
			continue;
		}

		if (playerWeight >= maxWeight) {
			break;
		}

		Vector2 toVaccum = vaccumPos - scrap->GetPosition();
		float distance = std::sqrt(toVaccum.x * toVaccum.x + toVaccum.y * toVaccum.y);

		if (distance <= vaccumRadius) {
			scrap->SetState(ScrapState::BeingSucked);
		}
	}

	// 吸引中のスクラップに吸引力を適用
	for (auto& scrap : scraps_) {
		if (scrap->GetState() == ScrapState::BeingSucked) {
			scrap->ApplySuction(vaccumPos, vaccumRadius, 1.0f / 60.0f);
		}
	}
}

// 吸引停止時の処理
void ScrapManager::ReleaseBeingSuckedScraps() {
	for (auto& scrap : scraps_) {
		if (!scrap->IsActive()) {
			continue;
		}

		// BeingSucked状態のスクラップをFreeに戻す
		if (scrap->GetState() == ScrapState::BeingSucked) {
			scrap->SetState(ScrapState::Free);
			// 速度を大幅に減衰させる
			Vector2 currentVel = scrap->GetVelocity();
			scrap->SetVelocity({ currentVel.x * 0.2f, currentVel.y * 0.2f });
		}
	}
}

std::vector<Scrap*> ScrapManager::GetHeldScraps() {
	std::vector<Scrap*> heldScraps;

	for (auto& scrap : scraps_) {
		if (scrap->IsActive() && scrap->GetState() == ScrapState::Held) {
			heldScraps.push_back(scrap.get());
		}
	}

	return heldScraps;
}


// ========================================
// 発射処理
// ========================================

void ScrapManager::FireAllHeldScraps(const Vector2& fireDirection, float fireSpeed, float spreadAngle) {
	if (heldCount_ == 0) {
		return;
	}

	std::uniform_real_distribution<float> angleDist(-spreadAngle / 2.0f, spreadAngle / 2.0f);

	for (auto& scrap : scraps_) {
		if (!scrap || !scrap->IsActive()) continue;

		if (scrap->GetState() == ScrapState::Held) {
			// 保持中のスクラップを発射
			float angleOffset = angleDist(randomEngine_);
			float rad = (std::atan2(fireDirection.y, fireDirection.x) + angleOffset * 3.14159f / 180.0f);

			scrap->SetVelocity({
				std::cos(rad) * fireSpeed,
				std::sin(rad) * fireSpeed
				});

			scrap->SetState(ScrapState::Fired);
		}
		else if (scrap->GetState() == ScrapState::BeingSucked) {
			// 吸引中だが保持されていないスクラップは通常状態に戻す
			scrap->SetState(ScrapState::Free);
			scrap->SetVelocity({ 0.0f, 0.0f });
		}
	}

	heldWeight_ = 0.0f;
	heldCount_ = 0;
}
// ========================================
// クリア
// ========================================
void ScrapManager::ClearAll() {
	scraps_.clear();
	heldWeight_ = 0.0f;
	heldCount_ = 0;
}

// ========================================
// 非アクティブなスクラップを配列から削除
// ========================================
void ScrapManager::ClearInactive() {
	scraps_.erase(
		std::remove_if(scraps_.begin(), scraps_.end(),
			[](const std::unique_ptr<Scrap>& scrap) {
				return !scrap->IsActive();
			}),
		scraps_.end()
	);
}

// ========================================
// 画面外のスクラップを削除
// ========================================
void ScrapManager::RemoveOutOfBoundsScraps(const Vector2& screenSize, float margin) {
	for (auto& scrap : scraps_) {
		if (!scrap->IsActive()) {
			continue;
		}

		// Held状態のスクラップは削除しない
		if (scrap->GetState() == ScrapState::Held || scrap->GetState() == ScrapState::BeingSucked) {
			continue;
		}

		Vector2 pos = scrap->GetPosition();

		// 画面外判定（マージン付き）
		bool outOfBounds =
			pos.x < -margin ||
			pos.x > screenSize.x + margin ||
			pos.y < -margin ||
			pos.y > screenSize.y + margin;

		if (outOfBounds) {
			scrap->SetActive(false);
		}
	}

	// ループ終了後にまとめて削除
	ClearInactive();
}

// ========================================
// デバッグ用
// ========================================
int ScrapManager::GetActiveScrapsCount() const {
	return static_cast<int>(std::count_if(scraps_.begin(), scraps_.end(),
		[](const std::unique_ptr<Scrap>& scrap) {
			return scrap->IsActive();
		}));
}

int ScrapManager::GetFreeScrapsCount() const {
	return static_cast<int>(std::count_if(scraps_.begin(), scraps_.end(),
		[](const std::unique_ptr<Scrap>& scrap) {
			return scrap->IsActive() && scrap->GetState() == ScrapState::Free;
		}));
}

// ========================================
// 重複回避処理
// ========================================
bool ScrapManager::IsOverlapping(const Vector2& position, float radius,
	const std::vector<Vector2>& existingPositions, const std::vector<float>& existingRadii) {

	for (size_t i = 0; i < existingPositions.size(); ++i) {
		Vector2 diff = { position.x - existingPositions[i].x, position.y - existingPositions[i].y };
		float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
		float minDistance = radius + existingRadii[i] + kMinSpawnDistance;

		if (distance < minDistance) {
			return true;
		}
	}

	return false;
}

// 重複しない位置を探索
Vector2 ScrapManager::FindNonOverlappingPosition(const Vector2& basePosition, float radius, float searchRadius,
	const std::vector<Vector2>& existingPositions, const std::vector<float>& existingRadii, int maxAttempts) {

	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
	std::uniform_real_distribution<float> distDist(0.0f, searchRadius);

	for (int attempt = 0; attempt < maxAttempts; ++attempt) {
		Vector2 candidate = basePosition;

		if (attempt > 0) {
			float angle = angleDist(randomEngine_);
			float dist = distDist(randomEngine_);
			candidate.x += std::cos(angle) * dist;
			candidate.y += std::sin(angle) * dist;
		}

		if (!IsOverlapping(candidate, radius, existingPositions, existingRadii)) {
			return candidate;
		}
	}

	return basePosition;
}

// ========================================
// 衝突解決
// ========================================

// 吸引中・保持中のスクラップ同士の衝突を解決
void ScrapManager::ResolveCollisions(const Vector2& vaccumPos) {
	vaccumPos; // 未使用パラメータ対策

	std::vector<Scrap*> activeScraps;

	// 吸引中・保持中のスクラップを収集
	for (auto& scrap : scraps_) {
		if (!scrap->IsActive()) {
			continue;
		}

		ScrapState state = scrap->GetState();
		if (state == ScrapState::BeingSucked || state == ScrapState::Held) {
			activeScraps.push_back(scrap.get());
		}
	}

	const int iterations = 3;
	for (int iter = 0; iter < iterations; ++iter) {
		for (size_t i = 0; i < activeScraps.size(); ++i) {
			for (size_t j = i + 1; j < activeScraps.size(); ++j) {
				Scrap* a = activeScraps[i];
				Scrap* b = activeScraps[j];

				// 両方とも吸引中の場合はスキップ
				if (a->GetState() == ScrapState::BeingSucked &&
					b->GetState() == ScrapState::BeingSucked) {
					continue;
				}

				Vector2 posA = a->GetPosition();
				Vector2 posB = b->GetPosition();

				Vector2 diff = { posB.x - posA.x, posB.y - posA.y };
				float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

				float radiusA = a->GetCollisionRadius();
				float radiusB = b->GetCollisionRadius();
				float minDistance = radiusA + radiusB;

				if (distance < minDistance && distance > 0.01f) {
					Vector2 normal = { diff.x / distance, diff.y / distance };

					float overlap = minDistance - distance;
					Vector2 push = { normal.x * overlap * 0.5f, normal.y * overlap * 0.5f };

					// 吸引中が含まれる場合は軽い反発
					bool hasBeingSucked = (a->GetState() == ScrapState::BeingSucked ||
						b->GetState() == ScrapState::BeingSucked);
					float pushScale = hasBeingSucked ? 0.3f : 1.0f;

					if (a->GetState() == ScrapState::Held) {
						a->SetPosition({ posA.x - push.x * pushScale, posA.y - push.y * pushScale });
					}
					else {
						a->AddVelocity({ -push.x * kCollisionPushForce * pushScale,
										 -push.y * kCollisionPushForce * pushScale });
					}

					if (b->GetState() == ScrapState::Held) {
						b->SetPosition({ posB.x + push.x * pushScale, posB.y + push.y * pushScale });
					}
					else {
						b->AddVelocity({ push.x * kCollisionPushForce * pushScale,
										 push.y * kCollisionPushForce * pushScale });
					}
				}
			}
		}
	}
}

// ========================================
// 保持中のスクラップ整列
// ========================================

// 保持中のスクラップをvaccumPos周辺に層状に整列配置
void ScrapManager::ArrangeHeldScraps(const Vector2& vaccumPos) {
	auto heldScraps = GetHeldScraps();
	int count = static_cast<int>(heldScraps.size());

	if (count == 0) {
		return;
	}

	int scrapIndex = 0;

	// 最初の1つは中心に配置
	if (count == 1) {
		heldScraps[0]->SetOrbitAngle(0.0f);
		heldScraps[0]->UpdateHeldPosition(vaccumPos, 0.0f, 1.0f / 60.0f);
		return;
	}

	// 中心に配置（最初の数個）
	const int centerCount = std::min(3, count);
	for (int i = 0; i < centerCount && scrapIndex < count; ++i) {
		float angle = (2.0f * 3.14159265f * i) / centerCount;
		heldScraps[scrapIndex]->SetOrbitAngle(angle);

		// 中心付近の小さい半径
		float centerRadius = kHeldOrbitRadiusBase * 0.3f;
		heldScraps[scrapIndex]->UpdateHeldPosition(vaccumPos, centerRadius, 1.0f / 60.0f);

		scrapIndex++;
	}

	// 残りは層状に配置
	int layer = 0;
	const int scrapsPerLayer = 6;

	while (scrapIndex < count) {
		int scrapsInThisLayer = std::min(scrapsPerLayer * (layer + 1), count - scrapIndex);

		// 層ごとの半径を計算（中心からの距離を調整）
		float layerRadius = kHeldOrbitRadiusBase + (layer * kHeldOrbitRadiusStep);

		for (int i = 0; i < scrapsInThisLayer && scrapIndex < count; ++i) {
			float angle = (2.0f * 3.14159265f * i) / scrapsInThisLayer;

			// 各層で少しずつ回転をずらして隙間を埋める
			float angleOffset = (layer % 2 == 0) ? 0.0f : (3.14159265f / scrapsInThisLayer);
			angle += angleOffset;

			heldScraps[scrapIndex]->SetOrbitAngle(angle);
			heldScraps[scrapIndex]->UpdateHeldPosition(vaccumPos, layerRadius, 1.0f / 60.0f);

			scrapIndex++;
		}

		layer++;
	}
}

void ScrapManager::HandleDebugInput(const Vector2& mousePos, const char* keys, const char* preKeys) {
	// 左クリックでスクラップ生成
	if (Novice::IsTriggerMouse(0)) {
		SpawnScrapCircle(
			mousePos,
			debugSpawnCount_,
			debugSpawnRadius_,
			debugScrapType_,
			debugSpreadSpeed_
		);
	}

	// 右クリックでランダム生成
	if (Novice::IsTriggerMouse(1)) {
		SpawnScrapRandom(
			mousePos,
			debugSpawnCount_,
			50.0f,  // 最小半径
			150.0f, // 最大半径
			debugScrapType_
		);
	}

	// 中クリックで爆発生成
	if (Novice::IsTriggerMouse(2)) {
		if (useNewExplosionMethod_) {
			// 新しい爆発生成メソッド（サイズ混合対応）
			SpawnScrapExplosionKinds(
				mousePos,
				debugMaxCount_,
				debugBigSizeCount_,
				debugGenerateSize_,
				debugExplosionForce_,
				debugMidSizeCount_
			);
		}
		else {
			// 従来の爆発生成メソッド（単一サイズ）
			SpawnScrapExplosion(
				mousePos,
				debugSpawnCount_,
				debugScrapType_,
				debugExplosionForce_
			);
		}
	}

	// Cキーで全クリア
	if (!preKeys[DIK_C] && keys[DIK_C]) {
		ClearAll();
	}

	// Xキーで非アクティブなスクラップをクリア
	if (!preKeys[DIK_X] && keys[DIK_X]) {
		ClearInactive();
	}
}

void ScrapManager::DrawDebugVisualization(const Vector2& mousePos, const Vector2& scrollOffset) {
	mousePos;
	scrollOffset;

#ifdef _DEBUG
	// マウスカーソル位置にマーカー表示
	Novice::DrawEllipse(
		static_cast<int>(mousePos.x - scrollOffset.x),
		static_cast<int>(mousePos.y - scrollOffset.y),
		5, 5, 0.0f, 0xFF0000FF, kFillModeSolid
	);

	// 爆発生成の範囲表示は新メソッド使用時は非表示
	if (!useNewExplosionMethod_) {
		Novice::DrawEllipse(
			static_cast<int>(mousePos.x - scrollOffset.x),
			static_cast<int>(mousePos.y - scrollOffset.y),
			static_cast<int>(debugSpawnRadius_),
			static_cast<int>(debugSpawnRadius_),
			0.0f, 0xFF000088, kFillModeWireFrame
		);
	}
#endif
}

// ========================================
// ボスのスクラップ管理用
// ========================================

/// <summary>
/// ボスの移動時にスクラップを生成（移動中に継続的に生成）
/// </summary>
void ScrapManager::SpawnBossScrapMove(
	bool isMoveGenerateScrap,
	const Vector2& bossCenter,
	float bossRadius,
	int spawnInterval,
	int spawnCountPerInterval,
	float outwardSpeed
) {
	if (!isMoveGenerateScrap) {
		// 移動していない場合はカウンターをリセット
		bossMoveSpawnFrameCounter_ = 0;
		return;
	}

	// フレームカウンターを更新
	bossMoveSpawnFrameCounter_++;

	// 指定間隔に達していなければ生成しない
	if (bossMoveSpawnFrameCounter_ < spawnInterval) {
		return;
	}

	// カウンターをリセット
	bossMoveSpawnFrameCounter_ = 0;

	// ランダム生成用の分布
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
	std::uniform_real_distribution<float> radiusDist(0.0f, bossRadius);

	// 指定個数のスクラップを生成
	for (int i = 0; i < spawnCountPerInterval; ++i) {
		// ボスの範囲内にランダム配置
		float angle = angleDist(randomEngine_);
		float radius = radiusDist(randomEngine_);

		Vector2 spawnPos = {
			bossCenter.x + std::cos(angle) * radius,
			bossCenter.y + std::sin(angle) * radius
		};

		// 中心から外側への方向を計算
		Vector2 direction = { spawnPos.x - bossCenter.x, spawnPos.y - bossCenter.y };
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

		if (length > 0.01f) {
			direction.x /= length;
			direction.y /= length;
		}
		else {
			// 中心の場合はランダムな方向
			direction = { std::cos(angle), std::sin(angle) };
		}

		// 外側への初速度を設定（少しランダム性を加える）
		std::uniform_real_distribution<float> speedVariation(0.8f, 1.2f);
		float speedMultiplier = speedVariation(randomEngine_);

		Vector2 velocity = {
			direction.x * outwardSpeed * speedMultiplier,
			direction.y * outwardSpeed * speedMultiplier
		};

		// 基本的にSmallサイズのみ生成
		CreateScrap(ScrapType::Small, ScrapTrait::Normal, spawnPos, velocity);
	}
}

/// <summary>
/// ボスのパンチ攻撃時にスクラップを爆発的に生成
/// </summary>
void ScrapManager::SpawnBossScrapPunch(
	const Vector2& bossPunchPos,
	int maxCount,
	int bigSizeCount,
	ScrapGenerateSize size,
	float explosionForce,
	int midSizeCount
) {
	// SpawnScrapExplosionKindsをそのまま使用
	SpawnScrapExplosionKinds(
		bossPunchPos,
		maxCount,
		bigSizeCount,
		size,
		explosionForce,
		midSizeCount
	);
}

/// <summary>
/// ボスのビーム軌道上にスクラップを生成
/// </summary>
void ScrapManager::SpawnBossScrapBeam(
	const Vector2& startPos,
	const Vector2& endPos,
	float width,
	int maxCount,
	ScrapGenerateSize size,
	float randomVelocityRange
) {
	if (maxCount <= 0) {
		return;
	}

	// ビームの方向と長さを計算
	Vector2 beamDir = { endPos.x - startPos.x, endPos.y - startPos.y };
	float beamLength = std::sqrt(beamDir.x * beamDir.x + beamDir.y * beamDir.y);

	if (beamLength < 0.01f) {
		return; // ビームが存在しない
	}

	// 方向を正規化
	beamDir.x /= beamLength;
	beamDir.y /= beamLength;

	// ビームの垂直方向ベクトル（90度回転）
	Vector2 perpDir = { -beamDir.y, beamDir.x };

	// 均等配置のための区画サイズ
	float segmentLength = beamLength / maxCount;

	// ランダム生成用の分布
	std::uniform_real_distribution<float> widthDist(-width * 0.5f, width * 0.5f);
	std::uniform_real_distribution<float> offsetDist(-segmentLength * 0.3f, segmentLength * 0.3f);
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265f);
	std::uniform_real_distribution<float> velocityDist(randomVelocityRange * 0.5f, randomVelocityRange);

	// サイズごとのスクラップ数を計算
	int smallCount = 0, mediumCount = 0, largeCount = 0;

	switch (size) {
	case ScrapGenerateSize::SmallAndMedium:
		mediumCount = maxCount / 4; // 約25%
		smallCount = maxCount - mediumCount;
		break;
	case ScrapGenerateSize::SmallAndLarge:
		largeCount = maxCount / 5; // 約20%
		smallCount = maxCount - largeCount;
		break;
	case ScrapGenerateSize::MediumAndLarge:
		largeCount = maxCount / 3; // 約33%
		mediumCount = maxCount - largeCount;
		break;
	case ScrapGenerateSize::SmallAndMediumAndLarge:
		largeCount = maxCount / 6; // 約17%
		mediumCount = maxCount / 3; // 約33%
		smallCount = maxCount - largeCount - mediumCount;
		break;
	}

	// スクラップタイプの配列を作成（シャッフル用）
	std::vector<ScrapType> scrapTypes;
	scrapTypes.reserve(maxCount);

	for (int i = 0; i < smallCount; ++i) scrapTypes.push_back(ScrapType::Small);
	for (int i = 0; i < mediumCount; ++i) scrapTypes.push_back(ScrapType::Medium);
	for (int i = 0; i < largeCount; ++i) scrapTypes.push_back(ScrapType::Large);

	// シャッフルしてランダムに配置
	std::shuffle(scrapTypes.begin(), scrapTypes.end(), randomEngine_);

	// 各区画にスクラップを配置
	for (int i = 0; i < maxCount; ++i) {
		// ビーム軌道上の基準位置（均等配置 + 少しのランダムオフセット）
		float t = (i + 0.5f) / maxCount; // 0.0～1.0
		float lengthOffset = offsetDist(randomEngine_);
		float distAlongBeam = (t * beamLength) + lengthOffset;

		// 幅方向のランダムオフセット
		float widthOffset = widthDist(randomEngine_);

		// 最終的な生成位置
		Vector2 spawnPos = {
			startPos.x + beamDir.x * distAlongBeam + perpDir.x * widthOffset,
			startPos.y + beamDir.y * distAlongBeam + perpDir.y * widthOffset
		};

		// ランダムな方向と速度
		float angle = angleDist(randomEngine_);
		float speed = velocityDist(randomEngine_);

		Vector2 velocity = {
			std::cos(angle) * speed,
			std::sin(angle) * speed
		};

		// スクラップを生成
		CreateScrap(scrapTypes[i], ScrapTrait::Normal, spawnPos, velocity);
	}
}

/// <summary>
/// ボスのタックル時にスクラップを大量生成（※仕様確定後に実装）
/// </summary>
void ScrapManager::SpawnBossScrapTackle(
	bool isTackleDone,
	const Vector2& bossCenterPos
) {
	// TODO: タックルの仕様が確定次第実装
	// 現在は空実装
	isTackleDone;
	bossCenterPos;
}