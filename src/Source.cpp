#include <SFML/Graphics.hpp> 
#include <SFML/Audio.hpp> 
#include <iostream>          
#include <optional>          // used for C++17 setup
#include <string>
#include <cstdlib>
#include <ctime>
#include "PowerUp.h"
#include "Menus.h"
#include<fstream>

using namespace std;
using namespace sf;

class LIVES;

void saveGameBinary(const string& filename, int currentLevel, int score, const LIVES& lives);
bool loadGameBinary(const string& filename, int& currentLevel, int& score, LIVES& lives);
void saveGameBinary(const string& filename, int currentLevel, int score, const LIVES& lives);
bool loadGameBinary(const string& filename, int& currentLevel, int& score, LIVES& lives);


struct Bullet {
    Sprite bulletsprite;
    bool bulletActive = false;
    float bulletSpeed = 650.0f;

    void updatebSpeed(float fr) {
        if (bulletActive) {
            bulletsprite.move(0, -bulletSpeed * fr);
            if (bulletsprite.getPosition().y < -20) {
                bulletActive = false;
            }
        }
    }
};

class Shooter {
private:
    Sprite shooterSprite;
    Texture shooterTexture;

    Texture bulletTexture;
    static const int MAX_BULLETS = 2;
    Bullet bullets[MAX_BULLETS];

    float minboundX = 0.0f;
    float maxboundX = 800.0f;

    static int totalShooters;

    bool shieldActive = false;
    float shieldTimer = 0.0f;

    bool doubleFireActive = false;
    float doubleFireTimer = 0.0f;


public:
    Shooter() { totalShooters++; }

    bool spritesLoaded(const string& shooterFilename, const string& bulletFilename) {
        // Try to load shooter texture
        if (!shooterTexture.loadFromFile(shooterFilename)) {
            cout << "Error: Could not load shooter texture from: " << shooterFilename << endl;
            return false;
        }
        shooterSprite.setTexture(shooterTexture);

        // Try to load bullet texture
        if (!bulletTexture.loadFromFile(bulletFilename)) {
            cout << "Error: Could not load bullet texture from: " << bulletFilename << endl;
            return false;
        }
        for (int i = 0; i < MAX_BULLETS; ++i) {
            bullets[i].bulletsprite.setTexture(bulletTexture);
        }
        return true;
    }

    void bulletPosition(float x, float y) {
        shooterSprite.setPosition(x, y);
    }

    void moveBullet(float dx, float dy) {
        shooterSprite.move(dx, dy);

        float x = shooterSprite.getPosition().x;
        float width = shooterTexture.getSize().x * shooterSprite.getScale().x;

        if (x < minboundX)
            shooterSprite.setPosition(minboundX, shooterSprite.getPosition().y);
        else if (x + width > maxboundX)
            shooterSprite.setPosition(maxboundX - width, shooterSprite.getPosition().y);
    }

    void shoot() {
        float px = shooterSprite.getPosition().x;
        float py = shooterSprite.getPosition().y;
        float pw = shooterSprite.getGlobalBounds().width;

        if (doubleFireActive) {
            float bw = bullets[0].bulletsprite.getGlobalBounds().width;

            if (!bullets[0].bulletActive) {
                bullets[0].bulletActive = true;
                bullets[0].bulletsprite.setPosition(px + pw * 0.3f - bw / 2.0f, py - 10.0f);
            }
            if (!bullets[1].bulletActive) {
                bullets[1].bulletActive = true;
                bullets[1].bulletsprite.setPosition(px + pw * 0.7f - bw / 2.0f, py - 10.0f);
            }
        }
        else {
            if (!bullets[0].bulletActive) {
                bullets[0].bulletActive = true;
                float bw = bullets[0].bulletsprite.getGlobalBounds().width;
                bullets[0].bulletsprite.setPosition(px + pw / 2.0f - bw / 2.0f, py - 10.0f);
            }
        }
    }

    void updatesSpeed(float dt) {
        for (int i = 0; i < MAX_BULLETS; ++i) {
            bullets[i].updatebSpeed(dt);
        }
    }

    void draw(RenderWindow& window) {
        window.draw(shooterSprite);
        for (int i = 0; i < MAX_BULLETS; ++i) {
            if (bullets[i].bulletActive)
                window.draw(bullets[i].bulletsprite);
        }
    }

    void setScale(float shooterScaleX, float shooterScaleY, float bulletScaleX, float bulletScaleY) {
        shooterSprite.setScale(shooterScaleX, shooterScaleY);
        for (int i = 0; i < MAX_BULLETS; ++i) {
            bullets[i].bulletsprite.setScale(bulletScaleX, bulletScaleY);
        }
    }

    int getMaxBullets() const { return MAX_BULLETS; }

    bool isBulletActive(int index) const {
        if (index < 0 || index >= MAX_BULLETS) return false;
        return bullets[index].bulletActive;
    }

    void deactivateBullet(int index) {
        if (index < 0 || index >= MAX_BULLETS) return;
        bullets[index].bulletActive = false;
    }

    bool isBulletActive() const {
        for (int i = 0; i < MAX_BULLETS; ++i)
            if (bullets[i].bulletActive) return true;
        return false;
    }

    FloatRect getBulletBounds(int index) const {
        return bullets[index].bulletsprite.getGlobalBounds();
    }

    FloatRect getBounds() const {
        return shooterSprite.getGlobalBounds();
    }

    const Sprite& getSprite() const { return shooterSprite; }

    void enableShield(float seconds) {
        shieldActive = true;
        shieldTimer = seconds;
    }

    void enableDoubleFire(float seconds) {
        doubleFireActive = true;
        doubleFireTimer = seconds;
    }

    bool isShieldActive() const { return shieldActive; }
    bool isDoubleFireActive() const { return doubleFireActive; }

    void updatePowerUps(float dt) {
        if (shieldActive) {
            shieldTimer -= dt;
            if (shieldTimer <= 0.0f) {
                shieldActive = false;
                shieldTimer = 0.0f;
            }
        }
        if (doubleFireActive) {
            doubleFireTimer -= dt;
            if (doubleFireTimer <= 0.0f) {
                doubleFireActive = false;
                doubleFireTimer = 0.0f;
            }
        }
    }

    void resetPowerUps() {
        shieldActive = false;
        shieldTimer = 0.0f;
        doubleFireActive = false;
        doubleFireTimer = 0.0f;
    }
};
int Shooter::totalShooters = 0;

struct Asteroid {
    Sprite asteroidSprite;
    bool asteroidActive = false;
    float asteroidSpeed = 80.0f;

    void initA(Texture& asteroidTexture, float diffmul = 1.0f) {
        asteroidSprite.setTexture(asteroidTexture);
        asteroidSprite.setScale(0.02f, 0.02f);
        asteroidActive = false;
        asteroidSpeed = 80.0f * diffmul;
    }

    void updateAsteroid(float fr) {
        if (asteroidActive) {
            asteroidSprite.move(0, asteroidSpeed * fr);
        }
        if (asteroidSprite.getPosition().y > 620) {
            asteroidActive = false;
        }
    }
};

class EnemyBase {
protected:
    Sprite sprite;
    Asteroid asteroid;
    float speedX;
    float speedY;
    bool active;
    float asteroidShootTimer;
    float asteroidTimerInterval;
    float difficultyMultiplier;

public:
    EnemyBase() : speedX(0.0f), speedY(0.0f), active(false), asteroidShootTimer(0.0f), asteroidTimerInterval(2.0f), difficultyMultiplier(1.0f) {}
    virtual ~EnemyBase() {}

    virtual float getBaseSpeed() const = 0;

    virtual void applyDamage() {
        deactivate();
    }

    bool enemyActive() const {
        return active;
    }

    void setDifficulty(int level) {
        if (level == 1) {
            difficultyMultiplier = 0.7f;
            asteroidTimerInterval = 3.0f;
        }
        else if (level == 2) {
            difficultyMultiplier = 1.2f;
            asteroidTimerInterval = 1.8f;
        }
        else if (level == 3) {
            difficultyMultiplier = 1.2f;
            asteroidTimerInterval = 3.0f;
        }
    }

    void init(Texture& texture, Texture& asteroidTexture, float x, float y, float dir, float Vspeed) {
        sprite.setTexture(texture);
        sprite.setScale(0.35f, 0.35f);
        sprite.setPosition(x, y);
        speedX = 0.0f;
        speedY = Vspeed * difficultyMultiplier;
        active = true;
        asteroidShootTimer = (rand() % 200) / 100.0f;
        asteroidTimerInterval = 3.0f / difficultyMultiplier;

        asteroid.initA(asteroidTexture, difficultyMultiplier);
    }

    void deactivate() {
        active = false;
        asteroid.asteroidActive = false;
    }

    bool isActive() const { return active; }

    Sprite& getSprite() { return sprite; }
    Asteroid& getAsteroid() { return asteroid; }

    virtual void shootAsteroid() {
        if (!asteroid.asteroidActive) {
            asteroid.asteroidActive = true;
            float px = sprite.getPosition().x;
            float py = sprite.getPosition().y;
            float pw = sprite.getGlobalBounds().width;
            float ph = sprite.getGlobalBounds().height;
            float asteroidWidth = asteroid.asteroidSprite.getGlobalBounds().width;

            asteroid.asteroidSprite.setPosition(px + pw / 2 - asteroidWidth / 2, py + ph);
        }
    }

    virtual void update(float dt, float minX, float maxX, float minY = -1.0f, float maxY = -1.0f) {
        if (!active) return;

        sprite.move(0, speedY * dt);

        float x = sprite.getPosition().x;
        float w = sprite.getGlobalBounds().width;

        if (x < minX) {
            sprite.setPosition(minX, sprite.getPosition().y);
            speedX = -speedX;
        }
        else if (x + w > maxX) {
            sprite.setPosition(maxX - w, sprite.getPosition().y);
            speedX = -speedX;
        }

        if (asteroid.asteroidActive) {
            asteroid.updateAsteroid(dt);
        }

        asteroidShootTimer += dt;
        if (asteroidShootTimer > asteroidTimerInterval) {
            shootAsteroid();
            asteroidShootTimer = 0.0f;
            asteroidTimerInterval = (3.0f + (rand() % 200) / 100.0f) / difficultyMultiplier;
        }
    }
    virtual bool isBoss() const { return false; }
    virtual bool isShielded() const { return false; }
};



class EasyEnemy : public EnemyBase {
public:
    virtual float getBaseSpeed() const override {
        return 50.0f;
    }
};

class MediumEnemy : public EnemyBase {
public:
    virtual float getBaseSpeed() const override {
        return 70.0f;
    }
};

class HardEnemy : public EnemyBase {
public:
    virtual float getBaseSpeed() const override {
        return 90.0f;
    }
};

class Level {
protected:
    float spawnTimer;
    float spawnInterval;
    int totalEnemies;
    int enemiesSpawned;
    bool hasSpawned;

public:
    Level() : spawnTimer(0.0f), spawnInterval(1.5f), totalEnemies(15), enemiesSpawned(0), hasSpawned(false) {}

    virtual ~Level() {}

    virtual void levelSetting(EnemyBase* enemies[15], int& enemyCount, Texture& easyTexture,
        Texture& mediumTexture, Texture& hardTexture, Texture& bossTexture,
        Texture& asteroidTexture) {
    }

    virtual void initializeEnemies(EnemyBase* enemies[15]) {
        for (int i = 0; i < 15; i++) {
            enemies[i] = nullptr;
        }
        enemiesSpawned = 0;
        spawnTimer = 0.0f;
        hasSpawned = false;
    }

    virtual bool updateSpawning(float dt, EnemyBase* enemies[15], Texture& easyTexture,
        Texture& mediumTexture, Texture& hardTexture, Texture& bossTexture,
        Texture& asteroidTexture) {
        if (hasSpawned) return false;

        spawnTimer += dt;
        if (spawnTimer >= spawnInterval && enemiesSpawned < totalEnemies) {
            spawnTimer = 0.0f;
            return true;
        }
        return false;
    }

    bool has_Spawned() const { return hasSpawned; }
    int remainingEnemiesGetter() const { return totalEnemies - enemiesSpawned; }
};

class Easylevel : public Level {
public:
    Easylevel() {
        spawnInterval = 2.5f;
        totalEnemies = 15;
    }

    virtual void levelSetting(EnemyBase* enemies[15], int& enemyCount, Texture& easyTexture, Texture& mediumTexture, Texture& hardTexture, Texture& bossTexture, Texture& asteroidTexture) override {
        enemyCount = 15;
        initializeEnemies(enemies);
    }

    bool spawnEnemy(EnemyBase* enemies[15], Texture& easyTexture, Texture& asteroidTexture, int currentLevel) {
        if (enemiesSpawned >= totalEnemies) {
            hasSpawned = true;
            return false;
        }
        for (int i = 0; i < 15; i++) {
            if (enemies[i] == nullptr) {
                enemies[i] = new EasyEnemy();
                enemies[i]->setDifficulty(currentLevel);
                float xPos = 50.0f + (rand() % 700);
                enemies[i]->init(easyTexture, asteroidTexture, xPos, -50.0f, 0.0f, 40.0f);
                enemies[i]->getSprite().setScale(0.15f, 0.15f);

                enemiesSpawned++;
                return true;
            }
        }
        return false;
    }
};

class WaveMove : public MediumEnemy {
protected:
    float waveSpeed;
    float waveTimer;
    float waveInterval;
    bool movingRight;

public:
    WaveMove() : waveSpeed(120.0f), waveTimer(0.0f), waveInterval(0.5f), movingRight(true) {}

    void init(Texture& texture, Texture& asteroidTexture, float x, float y, float dir, float vertSpeed = 35.0f) {
        MediumEnemy::init(texture, asteroidTexture, x, y, dir, vertSpeed);
        waveSpeed = 150.0f * difficultyMultiplier;
        waveTimer = 0.0f;
        waveInterval = 0.8f / difficultyMultiplier;
        movingRight = (rand() % 2) == 0;
    }

    void update(float dt, float minX, float maxX, float minY = -1.0f, float maxY = -1.0f) override {
        if (!isActive()) return;

        waveTimer += dt;
        if (waveTimer >= waveInterval) {
            movingRight = !movingRight;
            waveTimer = 0.0f;
            waveInterval = (0.8f + (rand() % 100) / 100.0f) / difficultyMultiplier;
        }

        float waveFrequency = waveSpeed * dt;
        Sprite& sprite = getSprite();

        if (movingRight) {
            sprite.move(waveFrequency, 0);
            if (sprite.getPosition().x + sprite.getGlobalBounds().width > maxX) {
                sprite.setPosition(maxX - sprite.getGlobalBounds().width, sprite.getPosition().y);
                movingRight = false;
                waveTimer = 0.0f;
            }
        }
        else {
            sprite.move(-waveFrequency, 0);
            if (sprite.getPosition().x < minX) {
                sprite.setPosition(minX, sprite.getPosition().y);
                movingRight = true;
                waveTimer = 0.0f;
            }
        }

        MediumEnemy::update(dt, minX, maxX, minY, maxY);
    }
};

class Mediumlevel : public Level {
public:
    Mediumlevel() {
        spawnInterval = 1.8f;
        totalEnemies = 15;
    }

    virtual void levelSetting(EnemyBase* enemies[15], int& enemyCount, Texture& easyTexture, Texture& mediumTexture, Texture& hardTexture, Texture& bossTexture, Texture& asteroidTexture) override {
        enemyCount = 15;
        initializeEnemies(enemies);
    }

    bool spawnEnemy(EnemyBase* enemies[15], Texture& mediumTexture, Texture& asteroidTexture, int currentLevel) {
        if (enemiesSpawned >= totalEnemies) {
            hasSpawned = true;
            return false;
        }

        for (int i = 0; i < 15; i++) {
            if (enemies[i] == nullptr) {
                if (enemiesSpawned % 2 == 0) {
                    WaveMove* waveEnemy = new WaveMove();
                    waveEnemy->setDifficulty(currentLevel);
                    float xPos = 50.0f + (rand() % 700);
                    waveEnemy->init(mediumTexture, asteroidTexture, xPos, -50.0f, 0.0f, 40.0f);
                    waveEnemy->getSprite().setScale(0.15f, 0.15f);
                    enemies[i] = waveEnemy;
                }
                else {
                    enemies[i] = new MediumEnemy();
                    enemies[i]->setDifficulty(currentLevel);
                    float xPos = 50.0f + (rand() % 700);
                    enemies[i]->init(mediumTexture, asteroidTexture, xPos, -50.0f, 0.0f, 25.0f);
                    enemies[i]->getSprite().setScale(0.15f, 0.15f);
                }

                enemiesSpawned++;
                return true;
            }
        }
        return false;
    }
};

class DiagonalEnemy : public HardEnemy {
protected:
    float diagonalSpeedX;
    float diagonalSpeedY;
    bool movingRight;
    bool isDiagonalType;

public:
    DiagonalEnemy() : diagonalSpeedX(100.0f), diagonalSpeedY(60.0f), movingRight(true), isDiagonalType(true) {}

    void init(Texture& texture, Texture& asteroidTexture, float x, float y, float dir, float vertSpeed = 40.0f) {
        HardEnemy::init(texture, asteroidTexture, x, y, dir, vertSpeed);
        diagonalSpeedX = 90.0f * difficultyMultiplier;
        diagonalSpeedY = 75.0f * difficultyMultiplier;
        movingRight = (rand() % 2) == 0;
        isDiagonalType = true;
    }

    bool isDiagonal() const { return isDiagonalType; }

    void update(float dt, float minX, float maxX, float minY = 50.0f, float maxY = 300.0f) override {
        if (!isActive()) return;

        HardEnemy::update(dt, minX, maxX, minY, maxY);

        if (!isActive()) return;

        Sprite& sprite = getSprite();
        float moveX = diagonalSpeedX * dt;
        float moveY = diagonalSpeedY * dt * 0.5f;

        if (movingRight) {
            sprite.move(moveX, moveY);
        }
        else {
            sprite.move(-moveX, moveY);
        }

        float x = sprite.getPosition().x;
        float y = sprite.getPosition().y;
        float w = sprite.getGlobalBounds().width;

        if (x < minX) {
            sprite.setPosition(minX, y);
            movingRight = true;
        }
        else if (x + w > maxX) {
            sprite.setPosition(maxX - w, y);
            movingRight = false;
        }

        if (y > 600) {
            deactivate();
        }
    }
    virtual bool isShielded() const override { return false; }
};

class ShieldedEnemy : public HardEnemy {
protected:
    bool hasShield;
    bool isShieldedType;
    Color originalColor;
    int hitCount;

public:
    ShieldedEnemy() : hasShield(true), isShieldedType(true), hitCount(2) {}

    void init(Texture& texture, Texture& asteroidTexture, float x, float y, float dir, float vertSpeed = 40.0f) {
        HardEnemy::init(texture, asteroidTexture, x, y, dir, vertSpeed);
        hasShield = true;
        isShieldedType = true;
        originalColor = sprite.getColor();
        hitCount = 2;
        speedY *= 0.8f;
    }

    void deactivate() {
        if (hasShield) {
            hasShield = false;
            sprite.setColor(Color(200, 100, 100));
        }
        else {
            HardEnemy::deactivate();
        }
    }

    virtual bool isShielded() const override { return hasShield; }
    bool isShieldedEnemyType() const { return isShieldedType; }
    bool getShieldDestroyed() const { return !hasShield; }

    void update(float dt, float minX, float maxX, float minY = 50.0f, float maxY = 300.0f) override {
        if (!isActive()) return;

        float time = sprite.getPosition().y * 0.01f;
        float waveOffset = sin(time) * 30.0f;
        float newX = sprite.getPosition().x + waveOffset * dt;

        float w = sprite.getGlobalBounds().width;
        if (newX < minX) newX = minX;
        if (newX + w > maxX) newX = maxX - w;

        sprite.setPosition(newX, sprite.getPosition().y);

        HardEnemy::update(dt, minX, maxX, minY, maxY);
    }
    void applyDamage() override {
        if (!isActive()) return;

        if (hasShield) {
            hasShield = false;
            sprite.setColor(Color(200, 100, 100));
        }
        else {
            hitCount--;
            if (hitCount <= 0) {
                HardEnemy::deactivate();
            }
        }
    }
    virtual bool isBoss() const override { return true; }
};

class Hardlevel : public Level {
private:
    int enemyTypes[15]; // 0 = Diagonal, 1 = Shielded

public:
    Hardlevel() {
        spawnInterval = 1.5f;
        totalEnemies = 15;


        for (int i = 0; i < 15; i++) {
            if (i < 6) {
                enemyTypes[i] = 0; // Diagonal enemies for first 6
            }
            else {
                enemyTypes[i] = 1; // Shielded enemies for the rest
            }
        }

        for (int i = 0; i < 15; i++) {
            int swapIndex = rand() % 15;
            int temp = enemyTypes[i];
            enemyTypes[i] = enemyTypes[swapIndex];
            enemyTypes[swapIndex] = temp;
        }
    }

    virtual void levelSetting(EnemyBase* enemies[15], int& enemyCount, Texture& easyTexture, Texture& mediumTexture, Texture& hardTexture, Texture& bossTexture, Texture& asteroidTexture) override {
        enemyCount = 15;
        initializeEnemies(enemies);
    }

    bool spawnEnemy(EnemyBase* enemies[15], Texture& hardTexture, Texture& bossTexture, Texture& asteroidTexture, int currentLevel) {
        if (enemiesSpawned >= totalEnemies) {
            hasSpawned = true;
            return false;
        }

        for (int i = 0; i < 15; i++) {
            if (enemies[i] == nullptr) {
                float xPos = 50.0f + (rand() % 700);

                if (enemyTypes[enemiesSpawned] == 0) {
                    enemies[i] = new DiagonalEnemy();
                    enemies[i]->setDifficulty(currentLevel);
                    enemies[i]->init(hardTexture, asteroidTexture, xPos, -50.0f, 0.0f, 70.0f);
                    enemies[i]->getSprite().setScale(0.15f, 0.15f);
                    enemies[i]->getSprite().setColor(Color::Cyan);
                }
                else {
                    enemies[i] = new ShieldedEnemy();
                    enemies[i]->setDifficulty(currentLevel);
                    enemies[i]->init(bossTexture, asteroidTexture, xPos, -50.0f, 0.0f, 50.0f);
                    enemies[i]->getSprite().setScale(0.10f, 0.10f);
                }

                enemiesSpawned++;
                return true;
            }
        }
        return false;
    }
};

class LIVES;

class Collision {
public:
    virtual ~Collision() {}
    virtual void handle(Shooter& shooter, EnemyBase* enemies[15], int activeEnemyCount, LIVES& lives, float dt) = 0;
};

class enemybulletcollide : public Collision {
public:
    void handle(Shooter& shooter, EnemyBase* enemies[15], int activeEnemyCount, LIVES& lives, float dt) override;

    void handle(Shooter& shooter, EnemyBase* enemies[15], int activeEnemyCount, LIVES& lives, float dt, int currentLevel, int& score);
};

class shooterasteroidcollide : public Collision {
public:
    void handle(Shooter& shooter, EnemyBase* enemies[15], int activeEnemyCount, LIVES& lives, float dt) override;
};

class shooterenemycollide : public Collision {
public:
    void handle(Shooter& shooter, EnemyBase* enemies[15], int activeEnemyCount, LIVES& lives, float dt) override;
};

class LIVES {
    int count;
    int max_count;
    Sprite hearts[4];
    bool invulnerable;
    float invul_timer;
    float invul_duration;
    Font& font;

public:
    LIVES(int start, int max, Texture& heartTexture, Font& hudFont, float invulDur = 1.0f);

    void reset(int x);
    void loseLife();
    void update(float dt);

    bool isInvulnerable() const;
    bool isGameOver() const;
    int get() const;

    void draw(RenderWindow& window, Text& label);
};

void enemybulletcollide::handle(Shooter& shooter, EnemyBase* enemies[15], int active_enemycount, LIVES& lives, float ab)
{
    static int dummyScore = 0;
    int dummyLevel = 0;
    handle(shooter, enemies, active_enemycount, lives, ab, dummyLevel, dummyScore);
}

void enemybulletcollide::handle(Shooter& shooter, EnemyBase* enemies[15], int active_enemycount, LIVES& lives, float dt, int currentLevel, int& score)
{
    bool anyActive = false;
    for (int b = 0; b < shooter.getMaxBullets(); ++b) {
        if (shooter.isBulletActive(b)) {
            anyActive = true;
            break;
        }
    }
    if (!anyActive) return;

    for (int b = 0; b < shooter.getMaxBullets(); ++b)
    {
        if (!shooter.isBulletActive(b)) continue;

        FloatRect bullet = shooter.getBulletBounds(b);

        for (int i = 0; i < active_enemycount; i++)
        {
            if (!enemies[i] || !enemies[i]->isActive())
                continue;

            Asteroid& ast = enemies[i]->getAsteroid();
            if (ast.asteroidActive &&
                ast.asteroidSprite.getGlobalBounds().intersects(bullet))
            {
                ast.asteroidActive = false;
                shooter.deactivateBullet(b);

                if (currentLevel == 1)      score += 100;
                else if (currentLevel == 2) score += 200;
                else if (currentLevel == 3) score += 300;

                return;
            }

            if (bullet.intersects(enemies[i]->getSprite().getGlobalBounds()))
            {
                shooter.deactivateBullet(b);

                bool wasActive = enemies[i]->isActive();
                bool wasBoss = enemies[i]->isBoss();

                enemies[i]->applyDamage();

                if (wasActive && !enemies[i]->isActive())
                {
                    if (currentLevel == 1) {
                        score += 200;
                    }
                    else if (currentLevel == 2) {
                        score += 400;
                    }
                    else if (currentLevel == 3) {
                        if (wasBoss) score += 1200;
                        else         score += 600;
                    }
                }

                return;
            }
        }
    }
}


void shooterasteroidcollide::handle(Shooter& shooter, EnemyBase* enemies[15], int activeEnemyCount, LIVES& lives, float dt)
{
    if (lives.isInvulnerable() || shooter.isShieldActive()) return;

    FloatRect player = shooter.getBounds();
    for (int i = 0; i < activeEnemyCount; i++)
    {
        if (enemies[i]) {
            Asteroid& ast = enemies[i]->getAsteroid();
            if (ast.asteroidActive)
            {
                if (ast.asteroidSprite.getGlobalBounds().intersects(player)) {
                    ast.asteroidActive = false;
                    lives.loseLife();
                    return;
                }
            }
        }
    }
}

void shooterenemycollide::handle(Shooter& shooter, EnemyBase* enemies[15], int activeenemycount, LIVES& lives, float dt)
{
    if (lives.isInvulnerable() || shooter.isShieldActive()) return;

    FloatRect player = shooter.getBounds();
    for (int i = 0; i < activeenemycount; i++)
    {
        if (enemies[i] && enemies[i]->isActive())
        {
            if (enemies[i]->getSprite().getGlobalBounds().intersects(player))
            {
                enemies[i]->deactivate();
                lives.loseLife();
                return;
            }
        }
    }
}

LIVES::LIVES(int start, int max, Texture& tex, Font& f, float invDur)
    : count(start), max_count(max), font(f), invulnerable(false),
    invul_timer(0.0f), invul_duration(invDur)
{
    for (int i = 0; i < max_count; i++) {
        hearts[i].setTexture(tex);
        hearts[i].setScale(32.0f / tex.getSize().x, 32.0f / tex.getSize().y);
        hearts[i].setPosition(70 + i * 40, 10);
    }
}

void LIVES::reset(int x) {
    if (x > max_count) x = max_count;
    if (x < 0) x = 0;
    count = x;
    invulnerable = false;
    invul_timer = 0.0f;
}

bool LIVES::isInvulnerable() const {
    return invulnerable;
}

bool LIVES::isGameOver() const {
    return count <= 0;
}

int LIVES::get() const {
    return count;
}

void LIVES::loseLife() {
    if (invulnerable) return;
    count--;
    invulnerable = true;
    invul_timer = invul_duration;
}

void LIVES::update(float dt) {
    if (!invulnerable)
        return;
    invul_timer -= dt;
    if (invul_timer <= 0) {
        invulnerable = false;
        invul_timer = 0;
    }
}

void LIVES::draw(RenderWindow& win, Text& label) {
    win.draw(label);
    for (int i = 0; i < count && i < max_count; i++)
        win.draw(hearts[i]);
}

// Power up has separate header so fuctions are defined below
power_up::power_up(float d) : duration(d) {}

//copy constructor, deep copy is for duration)
power_up::power_up(const power_up& other)
    : duration(other.duration) {
}

power_up::~power_up() {}
float power_up::getDuration() const { return duration; }

shield_powerup::shield_powerup(float d) : power_up(d) {}

void shield_powerup::apply(Shooter& shooter, LIVES& lives) {
    shooter.enableShield(duration);
}

void shield_powerup::remove(Shooter& shooter, LIVES& lives) {
}

shield_powerup shield_powerup::operator+(const shield_powerup& rhs) const {
    return shield_powerup(this->duration + rhs.duration);
}

doublefire_powerup::doublefire_powerup(float d) : power_up(d) {}

void doublefire_powerup::apply(Shooter& shooter, LIVES& lives) {
    shooter.enableDoubleFire(duration);
}

void doublefire_powerup::remove(Shooter& shooter, LIVES& lives) {

}

doublefire_powerup doublefire_powerup::operator+(const doublefire_powerup& rhs) const {
    return doublefire_powerup(this->duration + rhs.duration);
}

extrahealth_powerup::extrahealth_powerup(int heal)
    : power_up(0.0f), healAmount(heal) {
}

void extrahealth_powerup::apply(Shooter& shooter, LIVES& lives) {
    int current = lives.get();
    int target = current + healAmount;
    if (target < 0) target = 0;
    lives.reset(target);
}

void extrahealth_powerup::remove(Shooter& shooter, LIVES& lives) {

}

extrahealth_powerup extrahealth_powerup::operator+(const extrahealth_powerup& rhs) const {
    return extrahealth_powerup(this->healAmount + rhs.healAmount);
}


void saveGameBinary(const string& filename, int currentLevel, int score, const LIVES& lives) {
    GameState state;
    state.level = currentLevel;
    state.score = score;
    state.lives = lives.get();

    ofstream out(filename.c_str(), ios::binary);
    if (!out) {
        cout << "Error: Failed to open save file for writing: " << filename << endl;
        return;
    }

    out.write(reinterpret_cast<const char*>(&state), sizeof(state));

    if (!out) {
        cout << "Error: Failed to write data to save file: " << filename << endl;
        out.close();
        return;
    }

    out.close();

    if (state.level < 1 || state.level > 3) {
        cout << "Warning: Invalid level data in save" << endl;
    }
    if (state.score < 0) {
        cout << "Warning: Invalid score data in save" << endl;
    }
    if (state.lives < 0 || state.lives > 4) {
        cout << "Warning: Invalid lives data in save" << endl;
    }

    cout << "Game saved to " << filename << endl;
}

bool loadGameBinary(const string& filename, int& currentLevel, int& score, LIVES& lives) {
    GameState state;

    ifstream in(filename.c_str(), ios::binary);
    if (!in) {
        cout << "No saved game found: " << filename << endl;
        return false;
    }

    in.read(reinterpret_cast<char*>(&state), sizeof(state));

    if (!in) {
        cout << "Error: Failed to read data from save file: " << filename << endl;
        in.close();
        return false;
    }

    in.close();

    // checking for data validity 
    if (state.level < 1 || state.level > 3) {
        cout << "Error: Invalid level data in save file" << endl;
        return false;
    }
    if (state.score < 0) {
        cout << "Error: Invalid score data in save file" << endl;
        return false;
    }
    if (state.lives < 0 || state.lives > 4) {
        cout << "Error: Invalid lives data in save file" << endl;
        return false;
    }

    currentLevel = state.level;
    score = state.score;
    lives.reset(state.lives);

    cout << "Game loaded from " << filename << endl;
    return true;
}



int main()
{
    const int easythreshold = 2500;
    const int mediumthreshold = 8200;
    const int hardthreshold = 18000;

    srand(time(NULL));
    RenderWindow window(VideoMode(800, 600), "Galaxy Wars");

    // Create necessary files required 
    ofstream createFile1("TopScores.txt", ios::app);
    if (!createFile1) {
        cout << "Warning: Could not create or open TopScores.txt" << endl;
    }
    createFile1.close();

    ofstream createFile2("PlayerInformation.txt", ios::app);
    if (!createFile2) {
        cout << "Warning: Could not create or open PlayerInformation.txt" << endl;
    }
    createFile2.close();

    Music easyMusic;
    Music mediumMusic;
    Music hardMusic;

    SoundBuffer bulletBuffer;
    Sound bulletSound;

    // Load music files
    if (!easyMusic.openFromFile("easy.wav")) {
        cout << "Warning: Could not load easy.wav music file" << endl;
    }
    if (!mediumMusic.openFromFile("medium.wav")) {
        cout << "Warning: Could not load medium.wav music file" << endl;
    }
    if (!hardMusic.openFromFile("hard.wav")) {
        cout << "Warning: Could not load hard.wav music file" << endl;
    }

    easyMusic.setLoop(true);
    mediumMusic.setLoop(true);
    hardMusic.setLoop(true);

    Font scoreFont;
    if (!scoreFont.loadFromFile("RobotoCondensed-Regular.ttf")) {
        cout << "Warning: Could not load font: RobotoCondensed-Regular.ttf" << endl;
    }

    HighScores h1;

    string name = h1.enterName(window, scoreFont);
    if (name.empty()) {
        window.close();
        return 0;
    }

    h1.setName(name);

    Menu menu;
    if (!menu.menuloaded("RobotoCondensed-Regular.ttf", "background.jpg", "title.png")) {
        cout << "Error: Could not load menu resources\n Unable to start the Game" << endl;
        window.close();
        return 1;
    }

    Shooter shooter;
    if (!shooter.spritesLoaded("shooter.png", "bullet.png")) {
        cout << "Error: Could not load shooter and bullet textures\n Unable to start the Game" << endl;
        window.close();
        return 1;
    }

    bool levelchosen = false;
    int currentLevel = 0;
    bool levelComplete = false;
    bool won = false;

    // Menu loop
    while (window.isOpen() && !levelchosen) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }

            menu.handleInput(event);
        }

        if (menu.levelSelected()) {
            int choice = menu.getLevel();

            if (choice == 1 || choice == 2 || choice == 3) {

                currentLevel = choice;
                levelchosen = true;
                cout << "Selected Level: " << currentLevel << endl;
            }
            else if (choice == -1) {
                menu.resetSelection();
                h1.displayHighScores(window, scoreFont);
            }
            else if (choice == -2) {
                window.close();
                break;
            }
        }

        if (!window.isOpen()) break;

        window.clear();
        menu.draw(window);
        window.display();
    }

    if (!window.isOpen()) {
        return 0;
    }

    easyMusic.stop();
    mediumMusic.stop();
    hardMusic.stop();

    if (currentLevel == 1)      easyMusic.play();
    else if (currentLevel == 2) mediumMusic.play();
    else if (currentLevel == 3) hardMusic.play();

    shooter.bulletPosition(400, 520);
    shooter.setScale(0.2f, 0.2f, 0.05f, 0.05f);

    float speed = 300.0;
    Clock clock;

    Texture easyEnemyTexture;
    Texture mediumEnemyTexture;
    Texture hardEnemyTexture;
    Texture bossEnemyTexture;
    Texture asteroidTexture;
    Texture shieldTexture;
    Texture gemTexture;

    // Load enemy textures
    if (!easyEnemyTexture.loadFromFile("bluealien.png")) {
        cout << "Error: Could not load bluealien.png" << endl;
        window.close();
        return 1;
    }

    if (!mediumEnemyTexture.loadFromFile("brownalien.png")) {
        cout << "Error: Could not load brownalien.png" << endl;
        window.close();
        return 1;
    }

    if (!hardEnemyTexture.loadFromFile("dbluealien.png")) {
        cout << "Error: Could not load dbluealien.png" << endl;
        window.close();
        return 1;
    }

    if (!bossEnemyTexture.loadFromFile("pinkalien.png")) {
        cout << "Error: Could not load pinkalien.png" << endl;
        window.close();
        return 1;
    }

    if (!asteroidTexture.loadFromFile("asteroid.png")) {
        cout << "Error: Could not load asteroid.png" << endl;
        window.close();
        return 1;
    }

    if (!shieldTexture.loadFromFile("shield.png")) {
        cout << "Warning: Could not load shield.png" << endl;
    }

    if (!gemTexture.loadFromFile("gem.png")) {
        cout << "Warning: Could not load gem.png" << endl;
    }

    Texture heartTexture;
    Font hudFont;

    if (!heartTexture.loadFromFile("heart.png")) {
        cout << "Error: Could not load heart.png" << endl;
        window.close();
        return 1;
    }

    if (!hudFont.loadFromFile("RobotoCondensed-Regular.ttf")) {
        cout << "Warning: Could not load HUD font. Using default font." << endl;
        hudFont = scoreFont;
    }


    if (!bulletBuffer.loadFromFile("bullet.wav")) {
        cout << "Warning: Could not load bullet.wav sound" << endl;
    }
    bulletSound.setBuffer(bulletBuffer);

    EnemyBase* enemies[15] = { nullptr };
    int activeEnemyCount = 15;

    Easylevel* easyLevel = nullptr;
    Mediumlevel* mediumLevel = nullptr;
    Hardlevel* hardLevel = nullptr;
    Level* currentLevelObj = nullptr;

    if (currentLevel == 1) {
        easyLevel = new Easylevel();
        easyLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
        currentLevelObj = easyLevel;
    }
    else if (currentLevel == 2) {
        mediumLevel = new Mediumlevel();
        mediumLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
        currentLevelObj = mediumLevel;
    }
    else if (currentLevel == 3) {
        hardLevel = new Hardlevel();
        hardLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
        currentLevelObj = hardLevel;
    }

    const int startLives = 3;
    const int maxLives = 4;

    LIVES livesObj(startLives, maxLives, heartTexture, hudFont, 1.0f);

    Text livesLabel;
    livesLabel.setFont(hudFont);
    livesLabel.setString("Lives:");
    livesLabel.setCharacterSize(20);
    livesLabel.setFillColor(Color::White);
    livesLabel.setPosition(10, 8);

    int score = 0;

    Text scoreText;
    scoreText.setFont(hudFont);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(4, 45);
    scoreText.setString("Score: 0");

    Text highScoreText;
    highScoreText.setFont(hudFont);
    highScoreText.setString("High: 0");
    highScoreText.setCharacterSize(20);
    highScoreText.setFillColor(Color::Yellow);
    highScoreText.setPosition(600, 10);

    Text spawnText;
    spawnText.setFont(hudFont);
    spawnText.setCharacterSize(16);
    spawnText.setFillColor(Color::Yellow);
    spawnText.setPosition(10, 550);
    spawnText.setString("Enemies remaining: 15");

    Sprite shield;
    Sprite gem;
    Sprite lifePowerUp;

    bool shieldSpawned = false;
    bool shieldCollected = false;

    bool gemSpawned = false;
    bool gemCollected = false;

    bool lifeSpawned = false;
    bool lifeCollected = false;

    float elapsedGameTime = 0.0f;

    Color currentBackground = Color::Black;
    float powerUpFlashTimer = 0.0f;
    float powerUpFlashDuration = 0.5f;
    bool isFlashing = false;


    enemybulletcollide bulletEnemyColl;
    shooterasteroidcollide asteroidPlayerColl;
    shooterenemycollide enemyPlayerColl;

    gameover_menu gameOverMenu(hudFont);
    PauseMenu pauseMenu(hudFont);
    bool paused = false;

    bool gameOver = false;

    // Game loop
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (gameOver) {
                char action = gameOverMenu.processKey(event);
                if (action == 'R') {
                    h1.saveInfo(currentLevel);
                    for (int i = 0; i < 15; i++) {
                        delete enemies[i];
                        enemies[i] = nullptr;
                    }

                    if (currentLevel == 1) {
                        easyLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                    }
                    else if (currentLevel == 2) {
                        mediumLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                    }
                    else if (currentLevel == 3) {
                        hardLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                    }

                    livesObj.reset(startLives);

                    easyMusic.stop();
                    mediumMusic.stop();
                    hardMusic.stop();

                    if (currentLevel == 1)      easyMusic.play();
                    else if (currentLevel == 2) mediumMusic.play();
                    else if (currentLevel == 3) hardMusic.play();

                    score = 0;
                    lifeSpawned = false;
                    lifeCollected = false;
                    shieldSpawned = false;
                    shieldCollected = false;
                    gemSpawned = false;
                    gemCollected = false;
                    elapsedGameTime = 0.0f;
                    gameOver = false;
                    paused = false;
                    isFlashing = false;
                }
                else if (action == 'Q') {
                    window.close();
                }
                else if (action == 'M') {
                    delete easyLevel;
                    delete mediumLevel;
                    delete hardLevel;

                    Menu menu2;
                    if (!menu2.menuloaded("RobotoCondensed-Regular.ttf", "background.jpg", "title.png")) {
                        cout << "Error: Cannot load menu resources for return-to-menu" << endl;
                    }
                    else {
                        bool returned = false;
                        while (window.isOpen() && !returned) {
                            Event me;
                            while (window.pollEvent(me)) {
                                if (me.type == Event::Closed) { window.close(); returned = true; break; }
                                menu2.handleInput(me);
                            }
                            if (!window.isOpen())
                                break;
                            if (menu2.levelSelected()) {
                                returned = true;
                                currentLevel = menu2.getLevel();
                                cout << "Selected Level from menu (M): " << currentLevel << endl;

                                for (int i = 0; i < 15; i++) {
                                    delete enemies[i];
                                    enemies[i] = nullptr;
                                }

                                if (currentLevel == 1) {
                                    easyLevel = new Easylevel();
                                    easyLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                    currentLevelObj = easyLevel;
                                }
                                else if (currentLevel == 2) {
                                    mediumLevel = new Mediumlevel();
                                    mediumLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                    currentLevelObj = mediumLevel;
                                }
                                else if (currentLevel == 3) {
                                    hardLevel = new Hardlevel();
                                    hardLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                    currentLevelObj = hardLevel;
                                }
                                lifeSpawned = false;
                                lifeCollected = false;
                                shieldSpawned = false;
                                shieldCollected = false;
                                gemSpawned = false;
                                gemCollected = false;
                                elapsedGameTime = 0.0f;

                                easyMusic.stop();
                                mediumMusic.stop();
                                hardMusic.stop();

                                if (currentLevel == 1)      easyMusic.play();
                                else if (currentLevel == 2) mediumMusic.play();
                                else if (currentLevel == 3) hardMusic.play();
                                livesObj.reset(startLives);
                                score = 0;
                                gameOver = false;
                                paused = false;
                                break;
                            }

                            window.clear();
                            menu2.draw(window);
                            window.display();
                        }
                    }
                }
                continue;
            }

            if (paused) {


                if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::K) {
                        saveGameBinary("savegame.dat", currentLevel, score, livesObj); // saving gamesate in binary 
                    }
                    else if (event.key.code == Keyboard::L) {
                        int loadedLevel = currentLevel;
                        int loadedScore = score;
                        if (loadGameBinary("savegame.dat", loadedLevel, loadedScore, livesObj)) {    //loading gamestate from binary 

                            for (int i = 0; i < 15; i++) {
                                delete enemies[i];
                                enemies[i] = nullptr;
                            }

                            currentLevel = loadedLevel;
                            score = loadedScore;

                            if (easyLevel) { delete easyLevel; easyLevel = nullptr; }
                            if (mediumLevel) { delete mediumLevel; mediumLevel = nullptr; }
                            if (hardLevel) { delete hardLevel; hardLevel = nullptr; }
                            currentLevelObj = nullptr;

                            if (currentLevel == 1) {
                                easyLevel = new Easylevel();
                                easyLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                currentLevelObj = easyLevel;
                            }
                            else if (currentLevel == 2) {
                                mediumLevel = new Mediumlevel();
                                mediumLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                currentLevelObj = mediumLevel;
                            }
                            else if (currentLevel == 3) {
                                hardLevel = new Hardlevel();
                                hardLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                currentLevelObj = hardLevel;
                            }
                            lifeSpawned = false;
                            lifeCollected = false;
                            shieldSpawned = false;
                            shieldCollected = false;
                            gemSpawned = false;
                            gemCollected = false;
                            elapsedGameTime = 0.0f;
                            gameOver = false;
                            levelComplete = false;
                            won = false;
                            isFlashing = false;
                            shooter.resetPowerUps();
                            easyMusic.stop();
                            mediumMusic.stop();
                            hardMusic.stop();
                            if (currentLevel == 1)      easyMusic.play();
                            else if (currentLevel == 2) mediumMusic.play();
                            else if (currentLevel == 3) hardMusic.play();

                            paused = false;
                        }
                    }
                }

                char act = pauseMenu.handleEvent(event);

                if (act == 'R') {
                    paused = false;
                }
                else if (act == 'S') {
                    for (int i = 0; i < 15; i++) {
                        delete enemies[i];
                        enemies[i] = nullptr;
                    }

                    if (currentLevel == 1) {
                        easyLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                    }
                    else if (currentLevel == 2) {
                        mediumLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                    }
                    else if (currentLevel == 3) {
                        hardLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                    }

                    livesObj.reset(startLives);
                    easyMusic.stop();
                    mediumMusic.stop();
                    hardMusic.stop();
                    if (currentLevel == 1)      easyMusic.play();
                    else if (currentLevel == 2) mediumMusic.play();
                    else if (currentLevel == 3) hardMusic.play();
                    score = 0;
                    lifeSpawned = false;
                    lifeCollected = false;
                    shieldSpawned = false;
                    shieldCollected = false;
                    gemSpawned = false;
                    gemCollected = false;
                    elapsedGameTime = 0.0f;
                    paused = false;
                    isFlashing = false;
                }
                else if (act == 'Q') {
                    window.close();
                }
                continue;
            }


            if (levelComplete || won) {
                if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::N && levelComplete) {
                        int nextLevel = currentLevel + 1;
                        if (nextLevel <= 3) {
                            for (int i = 0; i < 15; i++) {
                                delete enemies[i];
                                enemies[i] = nullptr;
                            }
                            currentLevel = nextLevel;
                            levelComplete = false;
                            delete easyLevel;
                            delete mediumLevel;
                            delete hardLevel;
                            easyLevel = nullptr;
                            mediumLevel = nullptr;
                            hardLevel = nullptr;

                            if (currentLevel == 1) {
                                easyLevel = new Easylevel();
                                easyLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                currentLevelObj = easyLevel;
                            }
                            else if (currentLevel == 2) {
                                mediumLevel = new Mediumlevel();
                                mediumLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                currentLevelObj = mediumLevel;
                            }
                            else if (currentLevel == 3) {
                                hardLevel = new Hardlevel();
                                hardLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                currentLevelObj = hardLevel;
                            }

                            easyMusic.stop();
                            mediumMusic.stop();
                            hardMusic.stop();

                            if (currentLevel == 1)      easyMusic.play();
                            else if (currentLevel == 2) mediumMusic.play();
                            else if (currentLevel == 3) hardMusic.play();

                            livesObj.reset(startLives);

                            lifeSpawned = false;
                            lifeCollected = false;
                            shieldSpawned = false;
                            shieldCollected = false;
                            gemSpawned = false;
                            gemCollected = false;
                            elapsedGameTime = 0.0f;
                            shooter.resetPowerUps();
                            isFlashing = false;

                        }
                    }
                    else if (event.key.code == Keyboard::M) {
                        delete easyLevel;
                        delete mediumLevel;
                        delete hardLevel;


                        for (int i = 0; i < 15; i++) {
                            delete enemies[i];
                            enemies[i] = nullptr;
                        }

                        levelchosen = false;
                        won = false;
                        levelComplete = false;
                        score = 0;
                        currentLevel = 0;
                        paused = false;
                        isFlashing = false;
                        lifeSpawned = false;
                        lifeCollected = false;
                        shieldSpawned = false;
                        shieldCollected = false;
                        gemSpawned = false;
                        gemCollected = false;
                        elapsedGameTime = 0.0f;
                        shooter.resetPowerUps();


                        Menu menu3;
                        if (!menu3.menuloaded("RobotoCondensed-Regular.ttf", "background.jpg", "title.png")) {
                            cout << "Error: Cannot load menu resources for return-to-menu" << endl;
                        }
                        else {
                            bool returned = false;
                            while (window.isOpen() && !returned) {
                                Event me;
                                while (window.pollEvent(me)) {
                                    if (me.type == Event::Closed) { window.close(); returned = true; break; }
                                    menu3.handleInput(me);
                                }
                                if (!window.isOpen())
                                    break;
                                if (menu3.levelSelected()) {
                                    returned = true;
                                    currentLevel = menu3.getLevel();
                                    cout << "Selected Level from menu (M): " << currentLevel << endl;

                                    if (currentLevel == 1) {
                                        easyLevel = new Easylevel();
                                        easyLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                        currentLevelObj = easyLevel;
                                    }
                                    else if (currentLevel == 2) {
                                        mediumLevel = new Mediumlevel();
                                        mediumLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                        currentLevelObj = mediumLevel;
                                    }
                                    else if (currentLevel == 3) {
                                        hardLevel = new Hardlevel();
                                        hardLevel->levelSetting(enemies, activeEnemyCount, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture);
                                        currentLevelObj = hardLevel;
                                    }
                                    lifeSpawned = false;
                                    lifeCollected = false;
                                    shieldSpawned = false;
                                    shieldCollected = false;
                                    gemSpawned = false;
                                    gemCollected = false;
                                    elapsedGameTime = 0.0f;

                                    easyMusic.stop();
                                    mediumMusic.stop();
                                    hardMusic.stop();

                                    if (currentLevel == 1)      easyMusic.play();
                                    else if (currentLevel == 2) mediumMusic.play();
                                    else if (currentLevel == 3) hardMusic.play();

                                    livesObj.reset(startLives);
                                    score = 0;
                                    gameOver = false;
                                    paused = false;
                                    break;
                                }

                                window.clear();
                                menu3.draw(window);
                                window.display();
                            }
                        }
                    }

                    else if (event.key.code == Keyboard::Q && won) {
                        window.close();
                    }
                }
                continue;
            }

            if (event.type == Event::KeyPressed &&
                event.key.code == Keyboard::P) {
                paused = true;
            }

            if (event.type == Event::KeyPressed &&
                event.key.code == Keyboard::Space)
            {
                shooter.shoot();
                bulletSound.play();
            }
        }

        float dt = clock.restart().asSeconds();

        elapsedGameTime += dt;

        if (!lifeSpawned && !lifeCollected && elapsedGameTime >= 10.0f) {
            lifeSpawned = true;
            lifePowerUp.setTexture(heartTexture);
            lifePowerUp.setScale(0.05f, 0.05f);
            float x = 50.0f + (rand() % 700);
            lifePowerUp.setPosition(x, 120.0f);
        }

        if (!shieldSpawned && !shieldCollected && elapsedGameTime >= 15.0f) {
            shieldSpawned = true;
            shield.setTexture(shieldTexture);
            shield.setScale(0.08f, 0.08f);
            float x = 50.0f + (rand() % 700);
            shield.setPosition(x, 80.0f);
        }

        if (!gemSpawned && !gemCollected && elapsedGameTime >= 20.0f) {
            gemSpawned = true;
            gem.setTexture(gemTexture);
            gem.setScale(0.08f, 0.08f);
            float x = 50.0f + (rand() % 700);
            gem.setPosition(x, 100.0f);
        }

        if (!paused && !gameOver && !levelComplete && !won) {

            if (Keyboard::isKeyPressed(Keyboard::Left))
                shooter.moveBullet(-speed * dt, 0);

            if (Keyboard::isKeyPressed(Keyboard::Right))
                shooter.moveBullet(speed * dt, 0);

            shooter.updatesSpeed(dt);
            shooter.updatePowerUps(dt);

            if (currentLevelObj && !currentLevelObj->has_Spawned()) {
                if (currentLevelObj->updateSpawning(dt, enemies, easyEnemyTexture, mediumEnemyTexture, hardEnemyTexture, bossEnemyTexture, asteroidTexture)) {
                    if (currentLevel == 1) {
                        easyLevel->spawnEnemy(enemies, easyEnemyTexture, asteroidTexture, currentLevel);
                    }
                    else if (currentLevel == 2) {
                        mediumLevel->spawnEnemy(enemies, mediumEnemyTexture, asteroidTexture, currentLevel);
                    }
                    else if (currentLevel == 3) {
                        hardLevel->spawnEnemy(enemies, hardEnemyTexture, bossEnemyTexture, asteroidTexture, currentLevel);
                    }
                }
            }

            for (int i = 0; i < 15; i++) {
                if (enemies[i] && enemies[i]->isActive()) {
                    enemies[i]->update(dt, 0.0f, 800.0f, 50.0f, 1000.0f);
                }
            }

            float powerUpSpeed = 80.0f;

            if (lifeSpawned && !lifeCollected) {
                lifePowerUp.move(0.f, powerUpSpeed * dt);
                if (lifePowerUp.getPosition().y > 600.f) {
                    lifeSpawned = false;
                }
            }

            if (shieldSpawned && !shieldCollected) {
                shield.move(0.f, powerUpSpeed * dt);
                if (shield.getPosition().y > 600.f) {
                    shieldSpawned = false;
                }
            }

            if (gemSpawned && !gemCollected) {
                gem.move(0.f, powerUpSpeed * dt);
                if (gem.getPosition().y > 600.f) {
                    gemSpawned = false;
                }
            }
            livesObj.update(dt);

            if (isFlashing) {
                powerUpFlashTimer -= dt;
                if (powerUpFlashTimer <= 0.0f) {
                    isFlashing = false;
                    powerUpFlashTimer = 0.0f;
                    currentBackground = Color::Black;
                }
            }

            bulletEnemyColl.handle(shooter, enemies, 15, livesObj, dt, currentLevel, score);
            asteroidPlayerColl.handle(shooter, enemies, 15, livesObj, dt);
            enemyPlayerColl.handle(shooter, enemies, 15, livesObj, dt);

            for (int b = 0; b < shooter.getMaxBullets(); ++b) {   //check for bullet collision with powerups
                if (!shooter.isBulletActive(b)) continue;
                FloatRect bRect = shooter.getBulletBounds(b);

                if (lifeSpawned && !lifeCollected &&
                    lifePowerUp.getGlobalBounds().intersects(bRect)) {
                    shooter.deactivateBullet(b);
                    lifeCollected = true;
                    lifeSpawned = false;

                    currentBackground = Color(180, 80, 120, 25);
                    isFlashing = true;
                    powerUpFlashTimer = powerUpFlashDuration;

                    extrahealth_powerup h1(1);
                    extrahealth_powerup h2(0);
                    extrahealth_powerup combined = h1 + h2;
                    combined.apply(shooter, livesObj);

                    break;
                }

                if (shieldSpawned && !shieldCollected &&
                    shield.getGlobalBounds().intersects(bRect)) {
                    shooter.deactivateBullet(b);
                    shieldCollected = true;
                    shieldSpawned = false;

                    currentBackground = Color(40, 60, 100, 30);
                    isFlashing = true;
                    powerUpFlashTimer = powerUpFlashDuration;

                    shield_powerup baseShield(15.0f);
                    shield_powerup bonusShield(0.0f);
                    shield_powerup combinedShield = baseShield + bonusShield;
                    combinedShield.apply(shooter, livesObj);

                    break;
                }

                if (gemSpawned && !gemCollected &&
                    gem.getGlobalBounds().intersects(bRect)) {
                    shooter.deactivateBullet(b);
                    gemCollected = true;
                    gemSpawned = false;

                    currentBackground = Color(70, 50, 90, 35);
                    isFlashing = true;
                    powerUpFlashTimer = powerUpFlashDuration;

                    doublefire_powerup baseDF(10.0f);

                    if (shooter.isDoubleFireActive()) {
                        doublefire_powerup bonusDF(5.0f);
                        doublefire_powerup combined = baseDF + bonusDF;
                        combined.apply(shooter, livesObj);
                    }
                    else {
                        baseDF.apply(shooter, livesObj);
                    }
                    break;
                }
            }


            if (livesObj.isGameOver()) {
                gameOver = true;
                paused = false;
            }

            scoreText.setString("Score: " + to_string(score));
            h1.setScore(score);
            int totalHigh = 0;

            ifstream file("TopScores.txt");
            if (file.is_open()) {
                string tempName;
                int tempScore, tempLevel;
                while (file >> tempName >> tempScore >> tempLevel) {
                    if (tempScore > totalHigh) {
                        totalHigh = tempScore;
                    }
                }
                file.close();
            }

            if (score > totalHigh) {
                highScoreText.setString("HIGH: " + to_string(score) + " (NEW!)");
                highScoreText.setFillColor(Color::Green);
            }
            else {
                highScoreText.setString("HIGH: " + to_string(h1.getHighScore()));
                highScoreText.setFillColor(Color::Yellow);
            }

            if (!levelComplete && !won) {
                if (currentLevel == 1 && score >= easythreshold) {
                    levelComplete = true;
                    h1.saveInfo(currentLevel);
                    int totalHighEasy = 0;

                    ifstream file("TopScores.txt");
                    if (file.is_open()) {
                        string tempName;
                        int tempScore, tempLevel;
                        while (file >> tempName >> tempScore >> tempLevel) {
                            if (tempScore > totalHighEasy) {
                                totalHighEasy = tempScore;
                            }
                        }
                        file.close();
                    }

                    if (score > totalHighEasy) {
                        spawnText.setString("EASY LEVEL COMPLETE! NEW HIGHSCORE! Score: " + to_string(score) + " Press N for next level or M for menu");
                    }
                    else {
                        spawnText.setString("EASY LEVEL COMPLETE! Score: " + to_string(score) + " Press N for next level or M for menu");
                    }
                }
                else if (currentLevel == 2 && score >= mediumthreshold) {
                    levelComplete = true;
                    h1.saveInfo(currentLevel);
                    int totalHighMedium = 0;

                    ifstream file("TopScores.txt");
                    if (file.is_open()) {
                        string tempName;
                        int tempScore, tempLevel;
                        while (file >> tempName >> tempScore >> tempLevel) {
                            if (tempScore > totalHighMedium) {
                                totalHighMedium = tempScore;
                            }
                        }
                        file.close();
                    }

                    if (score > totalHighMedium) {
                        spawnText.setString("MEDIUM LEVEL COMPLETE! NEW HIGHSCORE! Score: " + to_string(score) + " Press N for next level or M for menu");
                    }
                    else {
                        spawnText.setString("MEDIUM LEVEL COMPLETE! Score: " + to_string(score) + " Press N for next level or M for menu");
                    }
                }
                else if (currentLevel == 3 && score >= hardthreshold) {
                    levelComplete = true;
                    won = true;
                    h1.saveInfo(currentLevel);
                    int totalHighHard = 0;

                    ifstream file("TopScores.txt");
                    if (file.is_open()) {
                        string tempName;
                        int tempScore, tempLevel;
                        while (file >> tempName >> tempScore >> tempLevel) {
                            if (tempScore > totalHighHard) {
                                totalHighHard = tempScore;
                            }
                        }
                        file.close();
                    }

                    if (score > totalHighHard) {
                        spawnText.setString("HARD LEVEL COMPLETE! NEW HIGHSCORE! Score: " + to_string(score) + " Press M for menu or Q to Quit");
                    }
                    else {
                        spawnText.setString("HARD LEVEL COMPLETE! YOU HAVE WON! Score: " + to_string(score) + " Press M for menu or Q to Quit");
                    }
                }

            }
            if (!levelComplete && !won) {
                if (currentLevelObj) {
                    if (!currentLevelObj->has_Spawned()) {
                        spawnText.setString("Enemies remaining: " + to_string(currentLevelObj->remainingEnemiesGetter()));
                    }
                }
            }
            else {
                bool allDefeated = true;
                for (int i = 0; i < 15; i++) {
                    if (enemies[i] && enemies[i]->isActive()) {
                        allDefeated = false;
                        break;
                    }
                }
            }
        }


        if (isFlashing) {
            window.clear(currentBackground);
        }
        else {
            window.clear(Color::Black);
        }


        shooter.draw(window);


        if (lifeSpawned && !lifeCollected) {
            window.draw(lifePowerUp);
        }
        if (shieldSpawned && !shieldCollected) {
            window.draw(shield);
        }
        if (gemSpawned && !gemCollected) {
            window.draw(gem);
        }

        if (shooter.isShieldActive()) {
            CircleShape playerShield(50.0f);
            playerShield.setFillColor(Color(100, 100, 255, 40));
            playerShield.setOutlineColor(Color::Blue);
            playerShield.setOutlineThickness(2.0f);

            FloatRect pb = shooter.getBounds();
            playerShield.setPosition(
                pb.left + pb.width / 2.0f - playerShield.getRadius(),
                pb.top + pb.height / 2.0f - playerShield.getRadius()
            );
            window.draw(playerShield);
        }

        for (int i = 0; i < 15; i++) {
            if (enemies[i] && enemies[i]->isActive()) {
                window.draw(enemies[i]->getSprite());

                if (currentLevel == 3 && enemies[i]->isShielded() && enemies[i]->isActive()) {
                    CircleShape shield(63);
                    shield.setFillColor(Color(100, 100, 255, 50));
                    shield.setOutlineColor(Color::Blue);
                    shield.setOutlineThickness(2);
                    shield.setOrigin(-1, -5);

                    float enemyX = enemies[i]->getSprite().getPosition().x;
                    float enemyY = enemies[i]->getSprite().getPosition().y;

                    shield.setPosition(enemyX - 5, enemyY - 5);
                    window.draw(shield);
                }

                if (enemies[i]->getAsteroid().asteroidActive) {
                    if (currentLevel == 3 && enemies[i]->isShielded()) {
                        enemies[i]->getAsteroid().asteroidSprite.setColor(Color::Red);
                    }
                    window.draw(enemies[i]->getAsteroid().asteroidSprite);
                }
            }
        }

        window.draw(scoreText);
        window.draw(highScoreText);
        window.draw(spawnText);
        livesObj.draw(window, livesLabel);

        if (gameOver) {
            gameOverMenu.draw(window);

            Text goScore;
            goScore.setFont(hudFont);
            goScore.setString("Final Score: " + to_string(score));
            goScore.setCharacterSize(32);
            goScore.setFillColor(Color::Yellow);
            goScore.setPosition(290, 360);
            window.draw(goScore);
        }
        else if (paused) {
            pauseMenu.draw(window);
        }

        if (won) {
            RectangleShape overlay;
            overlay.setSize({ 800.f, 600.f });
            overlay.setFillColor(Color(0, 0, 0, 180));
            window.draw(overlay);

            Text winText;
            winText.setFont(hudFont);
            winText.setString("VICTORY!");
            winText.setCharacterSize(80);
            winText.setFillColor(Color::Green);
            winText.setPosition(280, 200);
            window.draw(winText);

            Text scoreText2;
            scoreText2.setFont(hudFont);
            scoreText2.setString("Final Score: " + to_string(score));
            scoreText2.setCharacterSize(40);
            scoreText2.setFillColor(Color::Yellow);
            scoreText2.setPosition(260, 300);
            window.draw(scoreText2);

            Text infoLine1;
            infoLine1.setFont(hudFont);
            infoLine1.setCharacterSize(24);
            infoLine1.setFillColor(Color::White);
            infoLine1.setString("K - Save game    L - Load game");
            infoLine1.setPosition(250, 380);
            window.draw(infoLine1);

            Text infoLine2;
            infoLine2.setFont(hudFont);
            infoLine2.setCharacterSize(24);
            infoLine2.setFillColor(Color::White);
            infoLine2.setString("M - Main Menu    Q - Quit");
            infoLine2.setPosition(250, 420);
            window.draw(infoLine2);
        }

        window.display();

    }


    for (int i = 0; i < 15; i++) {
        delete enemies[i];
    }
    delete easyLevel;
    delete mediumLevel;
    delete hardLevel;

    return 0;
}