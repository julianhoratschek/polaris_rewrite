/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef CSOURCE_ISRF_LEGACY_H
#define CSOURCE_ISRF_LEGACY_H

#include "DustMixture.hpp"
#include "Vector3D.hpp"
#include "MathFunctions.hpp"
#include "Matrix2D.hpp"
#include "Parameters.hpp"
#include "SourceBasic.hpp"
#include "Stokes.hpp"
#include "Typedefs.hpp"

namespace rewrite::legacy {

class CSourceISRF : public CSourceBasic
{
public:
    CSourceISRF()
    {
        init = false;
        kill_count = 0;
        radius = 0;
        g_zero = 0;

        grid = 0;

        c_w = 0;
        c_f = 0;
        c_q = 0;
        c_u = 0;
        c_v = 0;

        L = 0;

        source_id = SRC_ISRF;
    }

    ~CSourceISRF()
    {
        if(c_w != 0)
            delete[] c_w;
        if(c_f != 0)
            delete[] c_f;
    }

    bool initSource(uint id, uint max, bool use_energy_density);

    bool setParameterFromFile(parameters & param, uint p);

    void setParameter(parameters & param, uint p);

    void createNextRay(photon_package * pp, CRandomGenerator * rand_gen);

    void createDirectRay(photon_package * pp, CRandomGenerator * rand_gen, Vector3D dir_obs);

#ifdef TEST_REWRITE
public:
#else
private:
#endif
    Vector3D e, l;

    int kill_count;
    bool init;
    double c_q, c_u, c_v;
    double radius, g_zero;
    double *c_w, *c_f;
};

}

#endif /* CSOURCE_ISRF_H */
