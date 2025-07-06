/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef CMATH_INTERP_H
#define CMATH_INTERP_H

#include "Matrix2D.hpp"
#include "Stokes.hpp"
#include "Typedefs.hpp"

class interp
{
public:
    interp() {
        N = 0;
	x = nullptr;
	y = nullptr;
    }

    interp(uint size) {
        N = size - 1;
	x = new double[size];
	y = new double[size];
    }

    uint size() const;

    void resize(uint size);

    void resizeShared(size_t size, double* x_values, double* y_values);

    void setValue(uint pos, double _x, double _y);

    // void addValue(double _x, double _y);

    double getLinear(uint i, double v) const;

    double getValue(double v, uint interpolation = LINEAR) const;

private:
    uint N;
    double *x;
    double *y;
};

#endif /* CMATH_INTERP_H */
