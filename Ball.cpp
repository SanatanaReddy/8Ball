#include "Ball.h"
#include "TwoDVector.h"
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>
const double Ball::radius = 0.25;

Ball::Ball(TwoDVector vel, TwoDVector pos, int n, float r, float g, float b): Velocity(vel), Position(pos), ballNumber(n), red(r), green(g), blue(b), rotation(glm::quat(1.0,0.0,0.0,0.0)){
   float angle = ((float)(rand()%100)/100.0 * 6.283);
   TwoDVector axis = TwoDVector( (float)(rand()%100)/100.0, (float)(rand()%100)/100.0);
   axis.normalize();
   glm::quat random = glm::quat( std::cos(angle), axis.x * std::sin(angle), axis.y * std::sin(angle), 0.0);
   rotation = random * rotation;
}

void Ball::applyVelocity(double time){
    double deltaX =  Velocity.x * time;
    double deltaY =  Velocity.y * time;
    float distance = sqrt((deltaX * deltaX) + (deltaY * deltaY));
    float rotationAngle = (float) (distance/ radius);
    TwoDVector axis(-1.0 * deltaY, deltaX);
    axis.normalize();
    float sinAngle = std::sin( rotationAngle/2.0);
    glm::quat change = glm::quat( std::cos(rotationAngle/2.0), axis.x * sinAngle, axis.y * sinAngle, 0.0); 
    glm::quat rotation2 = change * rotation;
    rotation = rotation2;
    Position.x += deltaX;
    Position.y += deltaY;
    if( Velocity.x != 0)
    {
        double xdampen = fmin(abs(Velocity.x), 1.8 * time * abs(Velocity.x) / Velocity.norm());
        Velocity.x -=  Velocity.x / abs(Velocity.x) * xdampen;
    }
    if( Velocity.y != 0)
    {
        double ydampen = fmin(abs(Velocity.y), 1.8 * time * abs(Velocity.y) / Velocity.norm());
        Velocity.y -=  Velocity.y / abs(Velocity.y) * ydampen;
    }
   
}


void Ball::processCollisionWithBumpers( const std::vector<TwoDVector>& bumpers){
    if(Position.x > 2.0 && Position.x < 17.0 && Position.y > 2.0 && Position.y < 8.0) { return;}
    bool noCollision = true;
    for(std::vector<TwoDVector>::const_iterator iter = bumpers.begin(); noCollision && iter != bumpers.end(); iter+=2)
    {
        TwoDVector edge = difference( *iter, *(iter+1));
        double edgeLength = edge.norm();
        edge.normalize();
        TwoDVector normal = TwoDVector( -1.0* edge.y, edge.x);
        normal.normalize();
        TwoDVector relPos = difference( *iter, Position);
        double edgeDot = relPos.x * edge.x + relPos.y * edge.y;
        double normalDot = relPos.x * normal.x + relPos.y * normal.y;
        if( 0< normalDot && normalDot < radius && 0 < edgeDot && edgeDot < edgeLength)
        {
            double normalVelocity = Velocity.x * normal.x + Velocity.y * normal.y;
            double tangentVelocity = Velocity.x * edge.x + Velocity.y * edge.y;
            Velocity.x = 0.9 * (-1.0 * normalVelocity * normal.x + tangentVelocity * edge.x);
            Velocity.y = 0.9 * (-1.0 * normalVelocity * normal.y + tangentVelocity * edge.y);
            noCollision = false;
            Position.add( (radius - normalDot) * normal.x, (radius - normalDot) * normal.y);
        }
        
    }     
    if(noCollision)
    {
        for(std::vector<TwoDVector>::const_iterator iter = bumpers.begin(); iter != bumpers.end(); ++iter)
        {
            double distance = sqrt(  pow((Position.x - (*iter).x),2) + pow((Position.y - (*iter).y),2));
            if(distance < radius)
            {
                TwoDVector normal = difference( (*iter), Position);
                normal.normalize();
                double normalComp = Velocity.x * normal.x + Velocity.y * normal.y;
                Velocity.add( -1.8 * normalComp * normal.x, -1.8 * normalComp * normal.y);
                Position.add( (radius - distance) * normal.x, (radius - distance) * normal.y);
            }
        }
    }
}

bool Ball::inPocket( const std::vector<TwoDVector>& holeLocations, float holeRadius){
    bool sunken = false;
    if( Position.x > 1.0 && Position.x < 18.0 && Position.y > 1.0 && Position.y < 9.0) { return sunken;}
    for( std::vector<TwoDVector>::const_iterator it = holeLocations.begin(); it != holeLocations.end(); ++it)
    {
        if( sqrt(  pow( Position.x - it->x,2) + pow( Position.y - it->y, 2)) < holeRadius)
        {
            sunken = true;
        }
    }
    return sunken;
}
int processCollision(Ball& ball1, Ball& ball2){
    TwoDVector normal = difference(ball1.Position, ball2.Position);
    double distance = normal.norm();
    if (distance < ball1.radius*2){
        normal.normalize();
        double translationDistance = (2*ball1.radius - distance)/2;
        ball1.Position.add(-1 * translationDistance * normal.x, -1 * translationDistance * normal.y);
        ball2.Position.add(translationDistance * normal.x, translationDistance * normal.y); 
        TwoDVector normalVelocity1 = ball1.Velocity.projectOntoUnitVector(normal);
        TwoDVector normalVelocity2 = ball2.Velocity.projectOntoUnitVector(normal);
        TwoDVector velocityTransfer = difference(normalVelocity1, normalVelocity2);
        ball1.Velocity.add(0.9 * velocityTransfer.x, 0.9 * velocityTransfer.y);
        ball2.Velocity.add(-0.9* velocityTransfer.x, -0.9* velocityTransfer.y);
        if( ball1.ballNumber == 0)
        {
            return ball2.ballNumber;
        }
    }
    return -1;
}   

