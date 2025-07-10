/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef MATRIX2D_H
#define MATRIX2D_H

#include "Typedefs.hpp"

// m x n
// row    -> m (zeile)
// column |  n (spalte)

typedef vector<vector<vector<double> > > Matrix3D;

class Matrix2D
{
public:
    Matrix2D()
    {
        m_n = 0;
        m_m = 0;
        m_size = 0;
        m_data = 0;
        m_pos = 0;
    }

    Matrix2D(uint m, uint n)
    {
        m_n = n;
        m_m = m;
        m_size = n * m;
        m_pos = 0;
        m_data = new double[m_size];
        for(uint i = 0; i < m_size; i++)
            m_data[i] = 0;
    }

    Matrix2D(uint m, uint n, double * data)
    {
        m_n = n;
        m_m = m;
        m_size = n * m;
        m_pos = 0;
        m_data = new double[m_size];
        for(uint i = 0; i < m_size; i++)
            m_data[i] = data[i];
    }

    Matrix2D(uint m, uint n, vector<double> data)
    {
        m_n = n;
        m_m = m;
        m_size = n * m;
        m_pos = 0;
        m_data = new double[m_size];
        for(uint i = 0; i < m_size; i++)
            m_data[i] = data[i];
    }

    Matrix2D(const Matrix2D & rhs)
    {
        m_n = rhs.col();
        m_m = rhs.row();
        m_size = m_n * m_m;
        m_pos = 0;
        m_data = new double[m_size];

        for(uint i = 0; i < m_size; i++)
            m_data[i] = rhs(i);
    }

    ~Matrix2D(void)
    {
        if(m_data != 0)
            delete[] m_data;
    }

    void clear();

    uint get_n();

    uint get_m();

    void resize(uint m, uint n);

    void replace(double o_val, double n_val);

    void resize(uint m, uint n, double val);

    void printMatrix();

    double minElement();

    double maxElement();

    void setValue(uint i, uint j, double val);

    void addValue(uint i, uint j, double val);

    void addValue(uint k, double val);

    void set(uint k, double val);

    void set(double * val, uint size);

    void fill(double val);

    void transpose(bool invert_values);

    void transpose();

    dlist get_n_list(uint j);

    dlist get_m_list(uint i);

    uint size() const;

    uint row() const;

    uint col() const;

    void unityMatrix();

    void unityMatrix(uint m, uint n);

    double sum();

    double & operator()(uint i, uint j);

    double & operator()(uint i);

    void operator*=(double val);

    void operator+=(double val);

    void operator+=(const Matrix2D & mat);

    void operator-=(const Matrix2D & mat);

    double operator()(uint i, uint j) const;

    double operator()(uint k) const;

    Matrix2D & operator=(const Matrix2D & rhs);

    Matrix2D & operator=(Matrix2D * rhs);

    bool operator==(const Matrix2D & rhs) const;

    friend Matrix2D operator*(const Matrix2D & lhs, const Matrix2D & rhs);

    friend Matrix2D operator*(double val, const Matrix2D & mat);

    friend Matrix2D operator*(const Matrix2D & mat, double val);

    friend Matrix2D operator+(const Matrix2D & mat, const Matrix2D & rhs);

    friend Matrix2D operator+(const Matrix2D & mat, double val);

    friend ostream & operator<<(ostream & out, const Matrix2D & mat);

private:
    uint m_n; // column
    uint m_m; // row
    uint m_size;
    uint m_pos;
    double * m_data;
};

#endif /* MATRIX2D_H */
