/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef CMATH_SPLINE_H
#define CMATH_SPLINE_H

#include "Matrix2D.hpp"
#include "Stokes.hpp"
#include "Typedefs.hpp"

class spline
{
public:
    spline()
    {
        N = 0;
        d = 0;
        u = 0;
        w = 0;
        p = 0;
        x = 0;
        y = 0;
    }

    spline(uint size)
    {
        N = size - 1;
        d = new double[size];
        u = new double[size];
        w = new double[size];
        p = new double[size];
        x = new double[size];
        y = new double[size];

        for(uint i = 0; i < size; i++)
        {
            d[i] = 0;
            u[i] = 0;
            w[i] = 0;
            p[i] = 0;
            x[i] = 0;
            y[i] = 0;
        }
    }

    ~spline()
    {
        if(d != 0)
            delete[] d;
        if(u != 0)
            delete[] u;
        if(w != 0)
            delete[] w;
        if(p != 0)
            delete[] p;
        if(x != 0)
            delete[] x;
        if(y != 0)
            delete[] y;
    }

    uint size() const;

    void clear();

    void resize(uint size);

    /*
     *
     */
    void resizeShared(size_t size, double* x_values, double* y_values);

    double getAverageY();

    void setValue(uint pos, double _x, double _y);

    void addValue(uint pos, double _x, double _y);

    void setDynValue(double _x, double _y);

    // void addYValueExt(uint pos, double _x, double _y);

    double f(double x) const;

    void printX();

    void createSpline();

    void createDynSpline();

    double getMinValue();

    double getMaxValue();

    double getLinear(uint i, double v) const;

    double getLogLinear(uint i, double v) const;

    double getLinearValue(double v);

    double getValue(double v, uint extrapolation = SPLINE) const;

    double getLimitedValue(double v, double limit);

    double getX(uint pos);

    double getY(uint pos);

    uint getXIndex(double v);

    uint getYIndex(double v) const;

    double getValue(uint i) const;

    void operator+=(spline spline2);

    bool compare(const spline& other) {
	if (N != other.N) {
	    cout << endl << "N: " << N << " " << other.N << endl;
	    return false;
	}

	for (size_t i = 0; i < N; i++) {
	     if (x[i] != other.x[i] || y[i] != other.y[i]) {
		cout << endl << "\t@ " << i << " " << x[i] << "|" << other.x[i] << "\t" <<
		    y[i] << "|" << other.y[i] << endl;
		return false;
	     }
	}
	return true;
    }

    friend spline operator*(double val, spline & spline2);

private:
    uint N;
    double * d;
    double * u;
    double * w;
    double * p;
    double * x;
    double * y;

    dlist dx, dy;
};

#endif /* CMATH_SPLINE_H */
