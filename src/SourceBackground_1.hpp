/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef CSOURCE_BACKGROUND_H
#define CSOURCE_BACKGROUND_H

#include "DustMixture.hpp"
#include "Vector3D.hpp"
#include "MathFunctions.hpp"
#include "Matrix2D.hpp"
#include "Parameters.hpp"
#include "SourceBasic.hpp"
#include "Stokes.hpp"
#include "Typedefs.hpp"

class CSourceBackground : public CSourceBasic
{
public:
    CSourceBackground()
    {
        init = false;
        constant = false;

        step_xy = 0;
        off_xy = 0;

        grid = 0;

        bins = 0;
        sidelength = 0;

        ex.set(1, 0, 0);
        ey.set(0, 1, 0);
        ez.set(0, 0, 1);

        c_temp = 0;
        c_f = 0;
        c_q = 0;
        c_u = 0;
        c_v = 0;

        max_len = 0;

        lam_pf = 0;
        L = 0;
        rot_angle1 = 0;
        rot_angle2 = 0;

        source_id = SRC_BACKGROUND;
    }

    ~CSourceBackground()
    {
        if(L != 0)
            delete[] L;
        if(lam_pf != 0)
            delete[] lam_pf;
    }

    bool initSource(uint id, uint max, bool use_energy_density);

    StokesVector getStokesVector(photon_package * pp);

    bool setParameterFromFile(parameters & param, uint p);

    void setParameter(parameters & param, uint p);

    ullong getNrOfPhotons();

    uint getBins();

    void setOrientation(Vector3D n1, Vector3D n2, double _rot_angle1, double _rot_angle2);

private:
    Matrix2D temp, f, q, u, v;
    Vector3D ex, ey, ez;
    uint bins;
    uint max_len;
    bool init;
    bool constant;
    double sidelength;

    double step_xy;
    double off_xy;

    double c_temp, c_f, c_q, c_u, c_v;
    double rot_angle1, rot_angle2;
    spline * lam_pf;
    double * L;
};

#endif /* CSOURCE_BACKGROUND_H */