#pragma once
#include <SFML/Graphics.hpp>

class Shooter;
class LIVES;

class power_up {
protected:
    float duration;
public:
    power_up(float d = 0.0f);
    power_up(const power_up& other); 

    virtual ~power_up();

    float getDuration() const;

    virtual void apply(Shooter& shooter, LIVES& lives) = 0;
    virtual void remove(Shooter& shooter, LIVES& lives) = 0;
};


class shield_powerup : public power_up {
public:
    shield_powerup(float d = 15.0f);

    void apply(Shooter& shooter, LIVES& lives) override;
    void remove(Shooter& shooter, LIVES& lives) override;

    shield_powerup operator+(const shield_powerup& rhs) const;
};

class doublefire_powerup : public power_up {
public:
    doublefire_powerup(float d = 10.0f);

    void apply(Shooter& shooter, LIVES& lives) override;
    void remove(Shooter& shooter, LIVES& lives) override;

    doublefire_powerup operator+(const doublefire_powerup& rhs) const;
};

class extrahealth_powerup : public power_up {
    int healAmount;
public:
    extrahealth_powerup(int heal = 1);

    void apply(Shooter& shooter, LIVES& lives) override;
    void remove(Shooter& shooter, LIVES& lives) override;

    extrahealth_powerup operator+(const extrahealth_powerup& rhs) const;
};

