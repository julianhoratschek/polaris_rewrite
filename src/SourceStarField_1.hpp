/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef CSOURCE_STARFIELD_H
#define CSOURCE_STARFIELD_H

#include "DustMixture.hpp"
#include "Vector3D.hpp"
#include "MathFunctions.hpp"
#include "Matrix2D.hpp"
#include "Parameters.hpp"
#include "SourceBasic.hpp"
#include "Stokes.hpp"
#include "Typedefs.hpp"

class CSourceStarField : public CSourceBasic
{
public:
    CSourceStarField(void)
    {
        pos = 0;
        var = 0;
        source_id = SRC_SFIELD;
    }

    ~CSourceStarField(void)
    {}

    bool initSource(uint id, uint max, bool use_energy_density);

    void createNextRay(photon_package * pp, CRandomGenerator * rand_gen);
    void createDirectRay(photon_package * pp, CRandomGenerator * rand_gen, Vector3D dir_obs);

    bool setParameterFromFile(parameters & param, uint p);
    void setParameter(parameters & param, uint p);

private:
    double var;
};

#endif /* CSOURCE_STARFIELD_H */