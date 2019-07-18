#ifndef _TWODVECTOR_H_
#define _TWODVECTOR_H_

class TwoDVector{
    public:
        TwoDVector(double coor1, double coor2);
        double x;
        double y;
        double norm();
        void normalize();
        void add(double addX, double addY);
        TwoDVector projectOntoUnitVector(const TwoDVector& unit);
};
TwoDVector difference(const TwoDVector& vec1, const TwoDVector& vec2);
#endif
