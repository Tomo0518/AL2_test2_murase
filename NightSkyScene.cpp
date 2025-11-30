#include "NightSkyScene.h"
#include <Novice.h>
#include <cstdlib>
#include <cmath>

// 乱数ユーティリティ
static float RandomRange(float min, float max) {
	return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

NightSkyScene::NightSkyScene() {
	Initialize();
}

void NightSkyScene::Initialize() {
	camera_ = Camera2D({ 640.0f, 360.0f }, { 1280.0f, 720.0f });

	// 画像読み込み
	starTextureHandle_ = Novice::LoadTexture("./Resources/images/effect/particle_white.png");
	// パーティクルも同じ画像でOK（あるいは別のキラキラ画像があれば変えてください）
	particleTextureHandle_ = starTextureHandle_;

	// パーティクル描画用の「はんこ」を用意
	particleDrawer_ = std::make_unique<DrawComponent2D>(particleTextureHandle_);
	particleDrawer_->SetAnchorPoint({ 0.5f, 0.5f }); // 中心基準

	// 星を生成
	SpawnStars(400);
}

void NightSkyScene::SpawnStars(int count) {
	stars_.clear();
	stars_.reserve(count);

	for (int i = 0; i < count; ++i) {
		Star star;
		float posX = RandomRange(-200.0f, 1480.0f);
		float posY = RandomRange(-200.0f, 920.0f);
		star.originalPosition = { posX, posY };
		star.originalScale = RandomRange(0.0f, 0.06f);

		star.drawComp = std::make_unique<DrawComponent2D>(starTextureHandle_);
		star.drawComp->SetPosition(star.originalPosition);

		// 色のバリエーション
		int type = rand() % 10;
		if (type < 6) star.drawComp->SetBaseColor(0xFFFFFFFF); // 白
		else if (type < 8) star.drawComp->SetBaseColor(0xAADDFFFF); // 青白
		else star.drawComp->SetBaseColor(0xFFFFAA00); // 金色

		star.originalScale = RandomRange(0.04f, 0.08f);
		star.drawComp->SetScale(star.originalScale, star.originalScale);

		bool isLarge = star.originalScale >= 0.05f;
		if (isLarge) {
			// 大きい星用のパルス（強め・遅め）
			star.drawComp->StartPulse(star.originalScale * 0.6f, star.originalScale * 1.8f, 2.5f, true);
		}
		else {
			// 小さい星用のパルス（弱め・速め）
			star.drawComp->StartPulse(star.originalScale * 0.5f, star.originalScale * 1.4f, 3.5f, true);
		}

		stars_.push_back(std::move(star));
	}
}

void NightSkyScene::SpawnShootingStar() {
	ShootingStar ss;
	ss.drawComp = std::make_unique<DrawComponent2D>(starTextureHandle_);

	// 画面外（右上や左上）からスタート
	float startX = RandomRange(0.0f, 1280.0f);
	ss.position = { startX, -50.0f }; // 画面上部から

	// 斜め下に落ちる速度
	float speed = RandomRange(600.0f, 1000.0f); // かなり速く
	float angle = RandomRange(45.0f, 135.0f) * (3.14159f / 180.0f); // 下方向への角度
	ss.velocity = { cosf(angle) * speed, sinf(angle) * speed };

	ss.isActive = true;
	ss.drawComp->SetScale(0.01f, 0.01f); // 本体は少し大きめ
	ss.drawComp->SetBaseColor(0xFFFFDDFF); // 少し黄色がかった白

	shootingStars_.push_back(std::move(ss));
}

void NightSkyScene::AddTrail(const Vector2& pos) {
	TrailParticle tp;
	tp.position = pos;

	// 拡散するような動きを入れると綺麗
	tp.velocity = { RandomRange(-20.0f, 20.0f), RandomRange(-20.0f, 20.0f) };

	tp.scale = RandomRange(0.03f, 0.06f);
	tp.life = 1.0f;
	tp.decayRate = RandomRange(1.5f, 3.0f); // 消える速さ

	// ランダムカラー（シアン、ピンク、白）
	int colType = rand() % 3;
	if (colType == 0) tp.color = 0x00FFFFFF; // Cyan
	else if (colType == 1) tp.color = 0xFF55FFFF; // Pink
	else tp.color = 0xFFFFFFFF; // White

	trails_.push_back(tp);
}

void NightSkyScene::Update(float deltaTime, const char* keys, const char* preKeys) {
	// マウス位置更新
	int mx, my;
	Novice::GetMousePosition(&mx, &my);
	lensPosition_ = { (float)mx, (float)my };

	camera_.Update(deltaTime);

	// 1. 背景星の更新
	for (auto& star : stars_) {
		star.drawComp->Update(deltaTime);
	}

	// 2. 流れ星の発生制御
	shootingStarTimer_ -= deltaTime;
	if (shootingStarTimer_ <= 0.0f) {
		SpawnShootingStar();
		shootingStarTimer_ = RandomRange(1.0f, 3.0f); // 1〜3秒に1回発生
	}

	// デバッグ：Spaceキーで強制発生
	if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
		SpawnShootingStar();
	}

	// 3. 流れ星の更新
	auto itSS = shootingStars_.begin();
	while (itSS != shootingStars_.end()) {
		// 移動
		itSS->position.x += itSS->velocity.x * deltaTime;
		itSS->position.y += itSS->velocity.y * deltaTime;
		itSS->drawComp->SetPosition(itSS->position); // コンポーネントに反映
		itSS->drawComp->Update(deltaTime); // 回転などを更新

		// 軌跡（パーティクル）の発生
		// フレームレートに依存せず一定間隔で出す
		itSS->spawnTimer -= deltaTime;
		if (itSS->spawnTimer <= 0.0f) {
			// 一度に複数個出すとリッチになる
			for (int i = 0; i < 3; ++i) AddTrail(itSS->position);
			itSS->spawnTimer = 0.016f; // かなり高頻度
		}

		// 画面外判定
		if (itSS->position.y > 800.0f || itSS->position.x < -200.0f || itSS->position.x > 1480.0f) {
			itSS = shootingStars_.erase(itSS);
		}
		else {
			++itSS;
		}
	}

	// 4. トレイルパーティクルの更新
	auto itTrail = trails_.begin();
	while (itTrail != trails_.end()) {
		itTrail->position.x += itTrail->velocity.x * deltaTime;
		itTrail->position.y += itTrail->velocity.y * deltaTime;
		itTrail->life -= itTrail->decayRate * deltaTime;

		if (itTrail->life <= 0.0f) {
			itTrail = trails_.erase(itTrail);
		}
		else {
			++itTrail;
		}
	}
}

// ---------------------------------------------------------
// 描画処理
// ---------------------------------------------------------

void NightSkyScene::Draw() {
	// 背景クリア
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, bgColor_, kFillModeSolid);

	// === パス1：通常描画（等倍・全画面：レンズ内は除外） ===
	DrawWorldElements(false);

	// === パス2：レンズ効果描画（拡大・レンズ内のみ） ===
	// ※ステンシルがないので、距離判定で疑似クリッピングします
	DrawWorldElements(true);

	// === レンズの枠描画 ===
	for (int li = 0; li < 30; li++) {
		Novice::DrawEllipse(
			(int)lensPosition_.x, (int)lensPosition_.y,
			(int)lensRadius_ + (int)li, (int)lensRadius_ + (int)li,
			0.0f, 0xFFFFFFFF, kFillModeWireFrame
		);
	}

	// 装飾：十字線
	Novice::DrawLine(
		(int)lensPosition_.x - 20, (int)lensPosition_.y,
		(int)lensPosition_.x + 20, (int)lensPosition_.y,
		0xFFFFFF55
	);
	Novice::DrawLine(
		(int)lensPosition_.x, (int)lensPosition_.y - 20,
		(int)lensPosition_.x, (int)lensPosition_.y + 20,
		0xFFFFFF55
	);
}

// すべての要素を描画する関数
// isLensEffect = true なら、「レンズの中心からの距離に応じて位置をずらし、拡大して」描画する
void NightSkyScene::DrawWorldElements(bool isLensEffect) {
	float r2 = lensRadius_ * lensRadius_; // 半径の2乗（計算軽量化）

	// ===================================
	// A. 背景の星
	// ===================================
	for (auto& star : stars_) {
		Vector2 drawPos = star.originalPosition;
		float drawScale = star.originalScale;

		// 通常描画パスではレンズ内を除外して二重表示を防ぐ
		if (!isLensEffect) {
			float dx = star.originalPosition.x - lensPosition_.x;
			float dy = star.originalPosition.y - lensPosition_.y;
			if (dx * dx + dy * dy < r2) continue;
		}

		if (isLensEffect) {
			// レンズ中心からのベクトル
			float dx = star.originalPosition.x - lensPosition_.x;
			float dy = star.originalPosition.y - lensPosition_.y;
			float distSq = dx * dx + dy * dy;

			// レンズ範囲外ならスキップ
			if (distSq > r2) continue;

			// レンズ拡大計算：中心から放射状に位置を広げる
			drawPos.x = lensPosition_.x + dx * lensMagnification_;
			drawPos.y = lensPosition_.y + dy * lensMagnification_;
			drawScale *= lensMagnification_;

			// 拡大後の位置がレンズ外にはみ出たら描画しない（疑似クリッピング）
			float dxZoom = drawPos.x - lensPosition_.x;
			float dyZoom = drawPos.y - lensPosition_.y;
			if (dxZoom * dxZoom + dyZoom * dyZoom > r2) continue;
		}

		star.drawComp->SetPosition(drawPos);
		star.drawComp->SetScale(drawScale, drawScale);
		star.drawComp->Draw(camera_);

		if (isLensEffect) {
			star.drawComp->SetPosition(star.originalPosition);
			star.drawComp->SetScale(star.originalScale, star.originalScale);
		}
	}

	// ===================================
	// B. 流れ星本体
	// ===================================
	for (auto& ss : shootingStars_) {
		Vector2 originalPos = ss.position;
		Vector2 drawPos = originalPos;
		float baseScale = 0.02f; // 初期設定値
		float drawScale = baseScale;

		// 通常描画パスでレンズ内除外
		if (!isLensEffect) {
			float dx0 = originalPos.x - lensPosition_.x;
			float dy0 = originalPos.y - lensPosition_.y;
			if (dx0 * dx0 + dy0 * dy0 < r2) continue;
		}

		if (isLensEffect) {
			float dx = originalPos.x - lensPosition_.x;
			float dy = originalPos.y - lensPosition_.y;
			float distSq = dx * dx + dy * dy;

			if (distSq > r2) continue;

			drawPos.x = lensPosition_.x + dx * lensMagnification_;
			drawPos.y = lensPosition_.y + dy * lensMagnification_;
			drawScale *= lensMagnification_;

			float dxZoom = drawPos.x - lensPosition_.x;
			float dyZoom = drawPos.y - lensPosition_.y;
			if (dxZoom * dxZoom + dyZoom * dyZoom > r2) continue;
		}

		ss.drawComp->SetPosition(drawPos);
		ss.drawComp->SetScale(drawScale, drawScale);
		ss.drawComp->Draw(camera_);

		if (isLensEffect) {
			ss.drawComp->SetPosition(originalPos);
			ss.drawComp->SetScale(baseScale, baseScale);
		}
	}

	// ===================================
	// C. 軌跡パーティクル
	// ===================================
	for (auto& trail : trails_) {
		Vector2 drawPos = trail.position;
		float drawScale = trail.scale * trail.life; // 寿命で小さくなる

		// 通常描画パスでレンズ内除外
		if (!isLensEffect) {
			float dx0 = trail.position.x - lensPosition_.x;
			float dy0 = trail.position.y - lensPosition_.y;
			if (dx0 * dx0 + dy0 * dy0 < r2) continue;
		}

		if (isLensEffect) {
			float dx = trail.position.x - lensPosition_.x;
			float dy = trail.position.y - lensPosition_.y;
			float distSq = dx * dx + dy * dy;

			if (distSq > r2) continue;

			drawPos.x = lensPosition_.x + dx * lensMagnification_;
			drawPos.y = lensPosition_.y + dy * lensMagnification_;
			drawScale *= lensMagnification_;

			float dxZoom = drawPos.x - lensPosition_.x;
			float dyZoom = drawPos.y - lensPosition_.y;
			if (dxZoom * dxZoom + dyZoom * dyZoom > r2) continue;
		}

		particleDrawer_->SetPosition(drawPos);
		particleDrawer_->SetScale(drawScale, drawScale);

		// アルファ値を寿命に合わせる（だんだん透明に）
		unsigned int color = trail.color;
		unsigned int alpha = (unsigned int)(255.0f * trail.life);
		color = (color & 0xFFFFFF00) | alpha;
		particleDrawer_->SetBaseColor(color);

		particleDrawer_->Draw(camera_);
	}
}