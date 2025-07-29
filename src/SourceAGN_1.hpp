/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#ifndef CSOURCE_AGN_H
#define CSOURCE_AGN_H

#include "DustMixture.hpp"
#include "Vector3D.hpp"
#include "MathFunctions.hpp"
#include "Matrix2D.hpp"
#include "Parameters.hpp"
#include "SourceBasic.hpp"
#include "Stokes.hpp"
#include "Typedefs.hpp"

class CSourceAGN : public CSourceBasic
{
public:
    CSourceAGN()
    {
        pos = 0;
        source_id = SRC_AGN;
    }

    ~CSourceAGN()
    {}

    bool initSource(uint id, uint max, bool use_energy_density);

    void createNextRay(photon_package * pp, CRandomGenerator * rand_gen);
    void createDirectRay(photon_package * pp, CRandomGenerator * rand_gen, Vector3D dir_obs);

    bool setParameterFromFile(parameters & param, uint p);
    void setParameter(parameters & param, uint p);
};

#endif /* CSOURCE_AGN_H */