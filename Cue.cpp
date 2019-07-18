#include "Cue.h"
#include "Ball.h"
#include "TwoDVector.h"
#include <math.h>

const float Cue::length = 8.0;

Cue::Cue(): distanceToCue(5.0), angle(0.0), visible(false), shooting(false), velocity(-1.0) {}

void Cue::applyVelocity(float time){
    distanceToCue += velocity * time;
}

void Cue::processCollision( Ball& cueball){
    float overlap = Ball::radius + length - distanceToCue;
    if(overlap > 0.0)
    {
        visible = false;
        shooting = false;
        cueball.Position.add(std::cos(angle) * overlap, std::sin(angle) * overlap);    
        cueball.Velocity.add(-0.85 * std::cos(angle) * velocity, -0.85 * std::sin(angle) * velocity);
        velocity = 0.0;
    }
}

void Cue::updatePosition(const TwoDVector& ballpos, const TwoDVector& mousepos){
    TwoDVector angleVec = difference( ballpos, mousepos);
    float distance = angleVec.norm();
    angleVec.normalize();
    if(angleVec.x == 0)
    {
        angle = 3.1415/2.0 * angleVec.y / abs(angleVec.y);
    }
    else
    {
        angle = std::atan2( angleVec.y, angleVec.x);
    }
    distanceToCue = length + distance;
}
       
