#ifndef _BALL_H_
#define _BALL_H_
#include "TwoDVector.h"
#include <glm/gtc/quaternion.hpp>
#include <vector>
class Ball{

    public:

        TwoDVector Velocity;
        TwoDVector Position;
        glm::quat rotation;
        int ballNumber;
        float red, green, blue;
	static const double radius;
	
        Ball(TwoDVector vel, TwoDVector pos, int n, float r, float g, float b);
        void applyVelocity(double time);
        void processCollisionWithBumpers(const std::vector<TwoDVector>& bumpers);
        bool inPocket( const std::vector<TwoDVector>& holeLocations, float holeRadius);
};

int processCollision(Ball& ball1, Ball& ball2);

#endif
