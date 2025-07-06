/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#include "MathInterp.hpp"

uint interp::size() const {
    return N + 1;
}

void interp::resize(const uint size) {
    delete[] x;
    delete[] y;

    N = size - 1;

    x = new double[size];
    y = new double[size];
}

void interp::resizeShared(const size_t size, double* x_values, double* y_values) {
    delete[] x;
    delete[] y;

    N = size - 1;
    x = x_values;
    y = y_values;

}

void interp::setValue(const uint pos, const double _x, const double _y) {
#ifdef DEBUG
    if(x == 0)
    {
        cout << ERROR_LINE << "Linear interpolation was not initiated!" << endl;
        return;
    }
#endif
    x[pos] = _x;
    y[pos] = _y;
}

// void interp::addValue(double _x, double _y) {
//     x.push_back(_x);
//     y.push_back(_y);
// }

double interp::getLinear(const uint i, const double v) const {
    const double t = v - x[i];
    return y[i] + t * (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
}

double interp::getValue(const double v, const uint interpolation) const {
    if(N == 0)
        return y[0];

    if(v < x[0])
        switch(interpolation)
        {
            case CONST:
                return y[0];
                break;

            case LINEAR:
                return getLinear(0, v);
                break;
        }
    else if(v > x[N])
        switch(interpolation)
        {
            case CONST:
                return y[N];
                break;

            case LINEAR:
                return getLinear(N - 1, v);
                break;
        }
    else
    {
        uint min = 0;

        if(v != x[0])
            min = lower_bound(x, x + N + 1, v) - x - 1;

        switch(interpolation)
        {
            case CONST:
                return y[min+1];
                break;

            case LINEAR:
                return getLinear(min, v);
                break;
        }
    }

    return 0;
}
