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

    ~spline() {
	delete[] d;
	delete[] u;
	delete[] w;
	delete[] p;
	delete[] x;
	delete[] y;
    }

    uint size() const;

    void clear();

    void resize(uint size);

    /**
     * Indicates that memory of this.x and this.y should not
     * be managed by this spline. x_values and y_values must be
     * existing memory locations. This method must be paired with
     * unshare() to ensure correct freeing of memory. The
     * caller is still responsible for freeing x_values and y_values.
     */
    void resizeShared(size_t size, double* x_values, double* y_values);

    /**
     * Sets this.x and this.y to nullptr to ensure, memory will
     * not be freed by spline. This method call should be preceded
     * by resizeShared.
     */
    void unshare();

    double getAverageY();

    void setValue(uint pos, double _x, double _y);

    void addValue(uint pos, double _x, double _y);

    void setDynValue(double _x, double _y);
    void copyDynValue(const std::vector<double>& _x, std::vector<double>&& _y) {
	dx.resize(_x.size());
	std::copy(_x.begin(), _x.end(), dx.begin());
	dy = std::move(_y);
    }

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

    bool operator==(const spline& other) const {
	if (N != other.N)
	    return false;

	for (size_t i = 0; i < N; i++)
	    if (x[i] != other.x[i] || y[i] != other.y[i])
		return false;

	// TODO: After createDySpline not necessary...
	if (dx.size() != other.dx.size())
	    return false;

	for (size_t i = 0; i < dx.size(); i++)
	    if (dx[i] != other.dx[i] || dy[i] != other.dy[i])
		return false;
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
