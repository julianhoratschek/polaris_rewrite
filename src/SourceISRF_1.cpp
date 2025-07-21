/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#include "SourceISRF.hpp"
#include "CommandParser.hpp"

bool CSourceISRF::initSource(uint id, uint max, bool use_energy_density)
{
    double * star_emi = new double[getNrOfWavelength()];

    cout << CLR_LINE << flush;
    cout << "-> Initiating interstellar radiation field          \r" << flush;

    for(uint w = 0; w < getNrOfWavelength(); w++)
    {
        double pl = sp_ext.getValue(wavelength_list[w]); //[W m^-2 m^-1 sr^-1]
        double sp_energy = pl * PI * PI * 3 * pow(radius * grid->getMaxLength(), 2);    //[W m^-1] energy per second and wavelength
        // if(g_zero > 0)
        //     sp_energy *= PIx2; //[W m^-1]
        star_emi[w] = sp_energy;
    }

    L = CMathFunctions::integ(wavelength_list, star_emi, 0, getNrOfWavelength() - 1);

    if(use_energy_density)
    {
        // For using energy density, only the photon number is required
        cout << "- Source (" << id + 1 << " of " << max << ") ISRF initiated with " << nr_of_photons
             << " photons FOR EACH wavelength"
             << "      " << endl;
    }
    else
    {
        double fr, sum = 0;
        lam_pf.resize(getNrOfWavelength());

        for(uint w = 0; w < getNrOfWavelength(); w++)
        {
            if(w > 0)
                sum += (wavelength_list[w] - wavelength_list[w - 1]) * star_emi[w - 1] +
                       0.5 * (wavelength_list[w] - wavelength_list[w - 1]) * (star_emi[w] - star_emi[w - 1]);

            fr = sum / L;
            lam_pf.setValue(w, fr, double(w));
        }
        cout << "- Source (" << id + 1 << " of " << max << ") ISRF initiated with " << nr_of_photons
             << " photons"
             << "      " << endl;
    }

    cout << "    Radius multiplier: " << radius << ", ";
    if(g_zero > 0)
        cout << "G_0: " << g_zero << " (see Mathis et al. 1983)" << endl;
    else
        cout << "luminosity: " << float(L / L_sun) << " [L_sun]" << endl;

    delete[] star_emi;

    return true;
}

bool CSourceISRF::setParameterFromFile(parameters & param, uint p)
{
    nr_of_photons = param.getNrOfISRFPhotons();
    string filename = param.getISRFPath();
    radius = param.getISRFRadius();

    spline sp_ext_wl;

    ifstream reader(filename.c_str());
    int line_counter = -2;
    string line;
    CCommandParser ps;

    double w_min = 1e300;
    double w_max = 0;

    cout << CLR_LINE << flush;
    cout << "-> Loading spectrum for source ISRF ...           \r" << flush;

    if(reader.fail())
    {
        cout << ERROR_LINE << "Cannot open file: " << filename << endl;
        return false;
    }

    while(getline(reader, line))
    {
        ps.formatLine(line);

        if(line.size() == 0)
            continue;

        dlist value = ps.parseValues(line);

        if(value.size() == 0)
            continue;

        line_counter++;

        if(line_counter == -1)
        {
            if(value.size() != 3)
            {
                cout << ERROR_LINE << "In ISRF file:\n" << filename << endl;
                cout << "Wrong amount of values in line " << line_counter + 1 << "!" << endl;
                return false;
            }

            c_q = value[0];
            c_u = value[1];
            c_v = value[2];
        }
        else if(line_counter >= 0)
        {
            if(value.size() == 2)
            {
                sp_ext_wl.setDynValue(value[0], value[1]);

                if(w_min > value[0])
                    w_min = value[0];

                if(w_max < value[0])
                    w_max = value[0];
            }
            else
            {
                cout << ERROR_LINE << "In ISRF file:\n" << filename << endl;
                cout << "Wrong amount of values in line " << line_counter + 1 << "!" << endl;
                return false;
            }
        }
    }

    sp_ext_wl.createDynSpline();
    reader.close();

    sp_ext.resize(getNrOfWavelength());
    for(uint w = 0; w < getNrOfWavelength(); w++)
    {
        double rad_field = 0;
        if(wavelength_list[w] > w_min && wavelength_list[w] < w_max)
        {
            // Get radiation field from dynamic spline
            // Divide by 4PI to get per steradian to match mathis_isrf
            rad_field = sp_ext_wl.getValue(wavelength_list[w]) / PIx4;
        }

        // Calculate final emission
        sp_ext.setValue(w, wavelength_list[w], rad_field * dust->getForegroundExtinction(wavelength_list[w]));
    }
    sp_ext.createSpline();

    return true;
}

void CSourceISRF::createNextRay(photon_package * pp, CRandomGenerator * rand_gen)
{
    double energy;
    StokesVector tmp_stokes_vector;
    uint wID;

    if(pp->getDustWavelengthID() != MAX_UINT)
    {
        wID = pp->getDustWavelengthID();
        double pl = sp_ext.getValue(wavelength_list[wID]); //[W m^-2 m^-1 sr^-1]
        energy = pl * PI * PI * 3 * pow(radius * grid->getMaxLength(), 2) / nr_of_photons; //[W m^-1] energy per second and wavelength
        // if(g_zero > 0)
        //     energy *= PIx2;
    }
    else
    {
        wID = lam_pf.getXIndex(rand_gen->getRND());
        energy = L / nr_of_photons;

        // Mol3D uses the upper value of the wavelength interval,
        // used for the selection of the emitting wavelengths from source!
        pp->setWavelength(wavelength_list[wID + 1], wID + 1);
    }

    tmp_stokes_vector = energy * StokesVector(1, c_q, c_u, c_v);

    // Get random direction for the postion (not the direction of travel) of the photon
    pp->setRandomDirection(rand_gen->getRND(), rand_gen->getRND());
    pp->initCoordSystem();

    // pos is center position of the sphere where the ISRF is emitted from
    pos = Vector3D(0,0,0);
    // set photon position to be on the surface of that sphere
    // negative sign, so that the direction of travel is inside of the cell
    pp->adjustPosition(pos, -sqrt(3) * radius * grid->getMaxLength() / 2);

    // the emission has to obey Lambert's cosine law
    // Thus, the square of cos(theta) of the direction of the photon
    // is given by a random number -> theta is only in (0,pi/2)
    // bias direction: theta -> 0, use pdf ~ cos^n(theta), n >= 1
    // int_{0}^{pi/2} cos^n(x) sin(x) dx = 1 / (n+1)
    // (n+1) int cos^n(x) sin(x) dx = -cos^{n+1}(x)
    // rnd = 1 - cos^{n+1}(theta) -> theta = arccos( (rnd)^{1/(n+1)} )
    // scale n with radius, 50 is arbitrary
    double bias_exp = 50.0 * radius;
    double theta_direction = acos( pow(rand_gen->getRND(), 1.0 / (bias_exp + 1.0)) );
    double phi_direction = PIx2 * rand_gen->getRND();

    // the direction was calculated in the coord system of the photon
    // now we have to update the coord system acoordingly
    pp->updateCoordSystem(phi_direction, theta_direction);

    // weight photon energy accordingly
    double exp_weight = 2.0 / (bias_exp + 1.0) * pow(cos(theta_direction), 1.0 - bias_exp);
    pp->setStokesVector(tmp_stokes_vector * exp_weight);
}

void CSourceISRF::createDirectRay(photon_package * pp, CRandomGenerator * rand_gen, Vector3D dir_obs)
{
    // THIS IS PROBABLY STILL WRONG
    // actually, this function should never be used
    // because it puts all the energy of the isrf
    // into a single photon

    double energy;
    StokesVector tmp_stokes_vector;
    uint wID;

    if(pp->getDustWavelengthID() != MAX_UINT)
    {
        wID = pp->getDustWavelengthID();
        double pl = sp_ext.getValue(wavelength_list[wID]); //[W m^-2 m^-1 sr^-1]
        energy = pl * PI * 3 * pow(radius * grid->getMaxLength(), 2) / PIx2;
        if(g_zero > 0)
            energy *= PIx2; //[W m^-1] energy per second an wavelength
    }
    else
    {
        wID = lam_pf.getXIndex(rand_gen->getRND());
        energy = L / PIx2;

        // Mol3D uses the upper value of the wavelength interval,
        // used for the selection of the emitting wavelengths from source!
        pp->setWavelength(wavelength_list[wID + 1], wID + 1);
    }

    tmp_stokes_vector = energy * StokesVector(1, c_q, c_u, c_v);

    e.rndDir(rand_gen->getRND(), rand_gen->getRND());
    l.setX(e.X() * sqrt(3) * radius * grid->getMaxLength() / 2);
    l.setY(e.Y() * sqrt(3) * radius * grid->getMaxLength() / 2);
    l.setZ(e.Z() * sqrt(3) * radius * grid->getMaxLength() / 2);

    // Set direction of the photon package to the observer
    if(dir_obs.length() > 0)
    {
        pp->setDirection(dir_obs);
        pp->initCoordSystem();
    }

    pos.set(l.X(), l.Y(), l.Z());

    pp->setPosition(pos);
    pp->setStokesVector(tmp_stokes_vector);
}

void CSourceISRF::setParameter(parameters & param, uint p)
{
    nr_of_photons = param.getNrOfISRFPhotons();
    g_zero = param.getISRFGZero();
    radius = param.getISRFRadius();

    sp_ext.resize(getNrOfWavelength());
    for(uint w = 0; w < getNrOfWavelength(); w++)
    {
        // Get Mathis radiation field
        double rad_field = CMathFunctions::mathis_isrf(wavelength_list[w]);

        // Calculate final emission
        sp_ext.setValue(w,
                        wavelength_list[w],
                        g_zero * rad_field * dust->getForegroundExtinction(wavelength_list[w]));
    }

    sp_ext.createSpline();
}
