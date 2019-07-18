#include "TwoDVector.h"
#include <math.h>

TwoDVector::TwoDVector(double coor1, double coor2): x(coor1), y(coor2) {}

double TwoDVector::norm(){
    double ret = sqrt(x*x + y*y);
    return ret;
}

void TwoDVector::normalize(){
    double length = norm();
    if (length != 0)
    {
        x /= length;
        y /= length;
    }
}

void TwoDVector::add(double addX, double addY){
    x += addX;
    y += addY;
}

TwoDVector TwoDVector::projectOntoUnitVector(const TwoDVector& unit){
    double dotProduct = x * unit.x + y* unit.y;
    TwoDVector ret(dotProduct * unit.x, dotProduct * unit.y);
    return ret;
}


TwoDVector difference(const TwoDVector& vec1, const TwoDVector& vec2){
    TwoDVector ret = TwoDVector( vec2.x - vec1.x, vec2.y - vec1.y);
    return ret;
}
    
