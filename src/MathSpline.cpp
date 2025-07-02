/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#include "MathSpline.hpp"

uint spline::size() const
{
    return N + 1;
}

void spline::clear()
{
    delete[] d;
    d = nullptr;
    delete[] u;
    u = nullptr;
    delete[] w;
    w = nullptr;
    delete[] p;
    p = nullptr;
    delete[] x;
    x = nullptr;
    delete[] y;
    y = nullptr;
}

void spline::resize(uint size)
{
    delete[] d;
    delete[] u;
    delete[] w;
    delete[] p;
    delete[] x;
    delete[] y;

    N = size - 1;
    d = new double[size]{0};
    u = new double[size]{0};
    w = new double[size]{0};
    p = new double[size]{0};
    x = new double[size]{0};
    y = new double[size]{0};
}


void spline::resizeShared(size_t size, double* x_values, double* y_values) {
    delete[] d;
    delete[] u;
    delete[] w;
    delete[] p;
    delete[] x;
    delete[] y;

    N = size - 1;
    d = new double[size]{0};
    u = new double[size]{0};
    w = new double[size]{0};
    p = new double[size]{0};
    x = x_values;
    y = y_values;
}

double spline::getAverageY()
{
    uint size = N + 1;
    double res = 0;

    for(uint i = 0; i < size; i++)
        res += y[i];

    res /= size;
    return res;
}

void spline::setValue(uint pos, double _x, double _y)
{
#ifdef DEBUG
    if(x == 0)
    {
        cout << ERROR_LINE << "Spline was not initiated!" << endl;
        return;
    }
#endif
    x[pos] = _x;
    y[pos] = _y;
}

void spline::addValue(uint pos, double _x, double _y)
{
    // TODO: is this correct?
    x[pos] = _x;
    y[pos] += _y;
}

void spline::setDynValue(double _x, double _y)
{
    dx.push_back(_x);
    dy.push_back(_y);
}

/*
void spline::addYValueExt(uint pos, double _x, double _y)
{
#ifdef DEBUG
    if(x == 0)
    {
        cout << ERROR_LINE << "Spline was not initiated!" << endl;
        return;
    }
#endif

    x[pos] = _x;
    y[pos] += _y;
}
*/

double spline::f(double k) const
{
    return k * k * k - k;
}

void spline::printX()
{
    cout << "spline" << endl;

    for(uint i = 0; i < N + 1; i++)
        cout << "   " << i << "  " << x[i] << endl; /**/
}

void spline::createSpline()
{
    if(N == 0)
        return;

    for(uint i = 1; i < N; i++)
        d[i] = 2 * (x[i + 1] - x[i - 1]);

    for(uint i = 0; i < N; i++)
    {
        // if((x[i + 1] - x[i]) == 0.0)
        //    cout << ERROR_LINE << "Spline broken!" << endl;
        u[i] = x[i + 1] - x[i];
    }

    for(uint i = 1; i < N; i++)
    {
        w[i] = 6.0 * ((y[i + 1] - y[i]) / u[i] - (y[i] - y[i - 1]) / u[i - 1]);
    }

    for(uint i = 1; i < N - 1; i++)
    {
        w[i + 1] -= w[i] * u[i] / d[i];
        d[i + 1] -= u[i] * u[i] / d[i];
    }

    for(uint i = N - 1; i > 0; i--)
    {
        p[i] = (w[i] - u[i] * p[i + 1]) / d[i];
    }
}

void spline::createDynSpline()
{
    if(dx.size() <= 4)
        return;

    uint size = uint(dx.size());

    if(x != 0)
    {
        delete[] d;
        delete[] u;
        delete[] w;
        delete[] p;
        delete[] x;
        delete[] y;
    }

    N = size - 1;
    d = new double[size];
    u = new double[size];
    w = new double[size];
    p = new double[size];
    x = new double[size];
    y = new double[size];

    d[0] = 0.0;
    d[1] = 0.0;
    d[N] = 0.0;

    u[0] = dx[1] - dx[0];
    u[1] = 0.0;
    u[N] = 0.0;

    w[0] = 0.0;
    w[1] = 0.0;
    w[N] = 0.0;

    p[0] = 0.0;
    p[1] = 0.0;
    p[N] = 0.0;

    x[0] = dx[0];
    x[1] = dx[1];

    y[0] = dy[0];
    y[1] = dy[1];

    for(uint i = 1; i < N; i++)
    {
        x[i] = dx[i];
        y[i] = dy[i];

        x[i + 1] = dx[i + 1];
        y[i + 1] = dy[i + 1];

        d[i] = 2 * (x[i + 1] - x[i - 1]);

        u[i] = x[i + 1] - x[i];

        if(u[i] == 0)
        {
            cout << ERROR_LINE << "Identical values in x[" << i << "] and x[" << i + 1
                    << "]!\n                        ";
            cout << u[i]
                    << "        Try smaller step sizes in Spline. \n                    "
                    "    ";
            u[i] = 1;
        }

        w[i] = 6.0 * ((y[i + 1] - y[i]) / u[i] - (y[i] - y[i - 1]) / u[i - 1]);
        // cout << i << " x:" << x[i] << " y:" << y[i]<< " d:" << d[i] << " u:" <<
        // u[i] << " w:" << w[i] << endl;
    }

    for(uint i = 1; i < N - 1; i++)
    {
        w[i + 1] -= w[i] * u[i] / d[i];
        d[i + 1] -= u[i] * u[i] / d[i];

        // cout << i << " " << w[i] << " " << d[i] << endl;
    }

    for(uint i = N - 1; i > 0; i--)
        p[i] = (w[i] - u[i] * p[i + 1]) / d[i];
}

double spline::getMinValue()
{
    return y[0];
}

double spline::getMaxValue()
{
    return y[N];
}

double spline::getLinear(uint i, double v) const
{
    double t = v - x[i];
    return y[i] + t * (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
}

double spline::getLogLinear(uint i, double v) const
{
    double x1 = log10(x[i]);
    double x2 = log10(x[i+1]);

    double y1 = log10(y[i]);
    double y2 = log10(y[i+1]);

    v = log10(v);

    double res = y1 + ((y2-y1)/(x2-x1)) * (v-x1);
    res = pow(10.0, res);

    return res;
}

double spline::getLinearValue(double v)
{
    uint min = 0, max = N;

//        for(int i=0;i<=max;i++)
//            cout << x[i] << "\t" << y[i]<< endl;

    if(x == 0)
        return 0;

    if(v <= x[0])
        return getLinear(0, v);

    if(v >= x[N])
        return getLinear(N - 1, v);

    while(max - min > 1)
    {
        uint i = min + (max - min) / 2;
        if(x[i] >= v)
            max = i;
        else
            min = i;
    }

    double x1 = log10(x[min]);
    double x2 = log10(x[min+1]);

    double y1 = log10(y[min]);
    double y2 = log10(y[min+1]);

    v = log10(v);

    double res = y1 + ((y2-y1)/(x2-x1)) * (v-x1);
    res = pow(10.0, res);
    return res;
}

double spline::getValue(double v, uint extrapolation) const
{
    uint min = 0;

    if(x == 0)
        return 0;

    if(N == 0)
        return y[0];

    // Extrapolation
    if(v < x[0])
        switch(extrapolation)
        {
            case CONST:
                return y[0];
                break;

            case LOGLINEAR:
                return getLogLinear(0, v);
                break;

            case LINEAR:
                return getLinear(0, v);
                break;

            case SPLINE:
                min = 0;
                break;
        }
    else if(v > x[N])
        switch(extrapolation)
        {
            case CONST:
                return y[N];
                break;

            case LOGLINEAR:
                return getLogLinear(N - 1, v);
                break;

            case LINEAR:
                return getLinear(N - 1, v);
                break;

            case SPLINE:
                min = N - 1;
                break;
        }
    else if(v == x[0])
        min = 0;
    else
        min = lower_bound(x, x+N+1, v) - x - 1;

    double t = (v - x[min]) / (u[min]);

    return t * y[min + 1] + (1 - t) * y[min] +
            u[min] * u[min] * (f(t) * p[min + 1] + f(1 - t) * p[min]) / 6.0;
}

double spline::getLimitedValue(double v, double limit)
{
    double t; // int i=1;
    unsigned int min_ID = 0, max_ID = N;

    if(v < x[0] || v > x[N])
        return 0;

    while(max_ID - min_ID > 1)
    {
        uint i = min_ID + (max_ID - min_ID) / 2;
        if(x[i] > v)
            max_ID = i;
        else
            min_ID = i;
    }

    double min_val, max_val;

    if(min_ID == 0)
    {
        min_val = min(y[0], y[1]);
        max_val = max(y[0], y[1]);
    }
    else if(min_ID == N)
    {
        min_val = min(y[N - 1], y[N]);
        max_val = max(y[N - 1], y[N]);
    }
    else
    {
        min_val = min(y[min_ID], y[min_ID + 1]);
        max_val = max(y[min_ID], y[min_ID + 1]);
    }

    // double lim_min = limit*min_val;
    double lim_max = (1 + limit) * max_val;

    t = (v - x[min_ID]) / (u[min_ID]);

    double res = t * y[min_ID + 1] + (1 - t) * y[min_ID] +
                    u[min_ID] * u[min_ID] * (f(t) * p[min_ID + 1] + f(1 - t) * p[min_ID]) / 6.0;

    double diff, lim_f;

    if(res < min_val)
    {
        res = min_val;
    }

    if(res > max_val)
    {
        diff = res - max_val;
        lim_f = diff / lim_max;

        if(lim_f > 1)
            lim_f = 1;

        diff *= lim_f;

        res = max_val - diff;
    }

    return res;
}

double spline::getX(uint pos)
{
    return x[pos];
}

double spline::getY(uint pos)
{
    return y[pos];
}

uint spline::getXIndex(double v)
{
    if(v < x[0] || v > x[N] || N==1)
        return 0;

    uint min = upper_bound(x, x+N+1, v) - x - 1;

    return min;
}

uint spline::getYIndex(double v) const
{
    if(v < y[0] || v > y[N] || N==1)
        return 0;

    uint min = upper_bound(y, y+N+1, v) - y - 1;

    return min;
}

double spline::getValue(uint i) const
{
    if(i < 0 || i > N)
        return 0;

    return y[i];
}

void spline::operator+=(spline spline2)
{
    if(N + 1 == spline2.size())
        for(uint i = 0; i <= N; i++)
            y[i] += spline2.getValue(x[i]);
}

spline operator*(double val, spline & spline2)
{
    spline res_spline(spline2.size());

    for(uint i = 0; i < spline2.size(); i++)
        res_spline.setValue(i, spline2.getX(i), spline2.getY(i) * val);
    return res_spline;
}
