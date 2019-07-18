#ifndef __CUE_H_
#define __CUE_H_
#include "TwoDVector.h"
#include "Ball.h"
class Cue{

    public:
        float distanceToCue, velocity, angle;
        bool shooting, visible;
        static const float length;
        
        Cue();
        void applyVelocity(float time);
        void processCollision( Ball& cueball);
        void updatePosition( const TwoDVector& ballpos, const TwoDVector& cursorpos);
};
#endif
