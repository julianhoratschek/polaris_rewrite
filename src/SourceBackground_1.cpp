/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#include "SourceBackground.hpp"
#include "CommandParser.hpp"

bool CSourceBackground::initSource(uint id, uint max, bool use_energy_density)
{
    lam_pf = new spline[max_len];
    L = new double[max_len];
    double * star_emi = new double[getNrOfWavelength()];
    sidelength = grid->getMaxLength();
    step_xy = sidelength / double(bins);
    off_xy = step_xy / 2.0;

    cout << CLR_LINE;

    if(constant)
    {
        cout << "Initiating constant background source             \r";

        for(uint w = 0; w < getNrOfWavelength(); w++)
        {
            double pl = 0.0;
            double sp_energy = 0.0;

            if(c_f>=0)
            {
                pl=CMathFunctions::planck(wavelength_list[w], c_temp); //[W m^-2 m^-1]
                sp_energy = abs(c_f) * pl; //[W m^-1] energy per second an wavelength
            }
            else
            {
                sp_energy = abs(c_f);
            }

            star_emi[w] = sp_energy;
        }

        L[0] = CMathFunctions::integ(wavelength_list, star_emi, 0, getNrOfWavelength() - 1);

        double fr, sum = 0;
        lam_pf[0].resize(getNrOfWavelength());

        for(uint w = 0; w < getNrOfWavelength(); w++)
        {
            if(w > 0)
                sum += (wavelength_list[w] - wavelength_list[w - 1]) * star_emi[w - 1] +
                       0.5 * (wavelength_list[w] - wavelength_list[w - 1]) * (star_emi[w] - star_emi[w - 1]);

            fr = sum / L[0];
            lam_pf[0].setValue(w, fr, double(w));
        }

        cout << CLR_LINE;
        if(nr_of_photons==0)
        {
            cout << "Source (" << id + 1 << " of " << max << ") BACKGROUND (const.) initiated \n";
        }
        else
        {
            if(use_energy_density)
                cout << "Source (" << id + 1 << " of " << max << ") BACKGROUND (const.) initiated \n"
                     << "with " << float(nr_of_photons) << " photons per cell and wavelength" << endl;
            else
                cout << "Source (" << id + 1 << " of " << max << ") BACKGROUND (const.) initiated \n"
                     << "with " << float(nr_of_photons) << " photons per cell" << endl;
        }

    }
    else
    {
        cout << CLR_LINE;
        cout << "Initiating variable background source: 0.0 [%]          \r";

        for(uint p = 0; p < max_len; p++)
        {
            double F = f(p);
            double T = temp(p);

            double fr, sum = 0;
            lam_pf[p].resize(getNrOfWavelength());

            for(uint w = 0; w < getNrOfWavelength(); w++)
            {
                double pl = CMathFunctions::planck(wavelength_list[w], T); //[W m^-2 m^-1]
                double sp_energy = F * pl; //[W m^-1] energy per second an wavelength
                star_emi[w] = sp_energy;
            }

            L[p] = CMathFunctions::integ(wavelength_list, star_emi, 0, getNrOfWavelength() - 1);

            for(uint w = 0; w < getNrOfWavelength(); w++)
            {
                if(w > 0)
                    sum +=
                        (wavelength_list[w] - wavelength_list[w - 1]) * star_emi[w - 1] +
                        0.5 * (wavelength_list[w] - wavelength_list[w - 1]) * (star_emi[w] - star_emi[w - 1]);

                fr = sum / L[p];
                lam_pf[p].setValue(w, fr, double(w));
            }

            if(p % 500 == 0)
                cout << "Initiating variable background source: " << 100 * float(p) / float(max_len)
                     << " [%]       \r";
        }

        if(use_energy_density)
            cout << "Source (" << id + 1 << " of " << max << ") BACKGROUND (var.) initiated \n"
                 << "with " << float(nr_of_photons) << " photons per cell and wavelength" << endl;
        else
            cout << "Source (" << id + 1 << " of " << max << ") BACKGROUND (var.) initiated \n"
                 << "with " << float(nr_of_photons) << " photons per cell" << endl;
    }

    // temp.clear();
    // f.clear();

    delete[] star_emi;
    return true;
}

bool CSourceBackground::setParameterFromFile(parameters & param, uint p)
{
    dlist values = param.getDiffuseSources();
    string filename = param.getBackgroundSourceString(p / NR_OF_BG_SOURCES);

    ifstream reader(filename.c_str());
    int line_counter = -2;
    string line;
    CCommandParser ps;

    rot_angle1 = values[p + 5];
    rot_angle2 = values[p + 6];
    nr_of_photons = ullong(values[p + 7]);

    if(reader.fail())
    {
        cout << ERROR_LINE << "Cannot open file:\n" << filename << endl;
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
            if(value.size() != 1)
            {
                cout << ERROR_LINE << "Wrong amount of values in:\n " << filename << endl;
                cout << "1 value expected in line " << line_counter + 5 << " !" << endl;
                return false;
            }

            bins = uint(value[0]);
            max_len = bins * bins;

            temp.resize(bins, bins);
            f.resize(bins, bins);
            q.resize(bins, bins);
            u.resize(bins, bins);
            v.resize(bins, bins);
        }
        else
        {
            if(line_counter > (int)max_len)
            {
                cout << ERROR_LINE << "To many background values in : " << filename << endl;
                cout << max_len << " lines expected!" << endl;
                return false;
            }

            if(value.size() == 5)
            {
                f.set(uint(line_counter), value[0]);
                temp.set(uint(line_counter), value[1]);
                q.set(uint(line_counter), value[2]);
                u.set(uint(line_counter), value[3]);
                v.set(uint(line_counter), value[4]);
            }
            else
            {
                cout << ERROR_LINE << "File : " << filename << endl;
                cout << " 5 values in line " << line_counter + 1 << " expected!" << endl;
                return false;
            }
        }
    }

    if(line_counter + 1 < (int)max_len)
    {
        cout << ERROR_LINE << "Not enough background values in : " << filename << endl;
        cout << max_len << " lines expected!" << endl;
        return false;
    }

    constant = false;
    reader.close();

    return true;
}

StokesVector CSourceBackground::getStokesVector(photon_package * pp)
{
    double F, T, I, Q, U, V, pl;
    StokesVector res;

    uint wID = pp->getDustWavelengthID();

    if(constant)
    {
        if(c_f>=0)
        {
            F = c_f;
            T = c_temp;
            Q = c_q;
            U = c_u;
            V = c_v;

            pl = CMathFunctions::planck(wavelength_list[wID], T); //[W m^-2 m^-1]
            I = F * pl;                                           //[W m^-1] energy per second and wavelength
            Q *= I;
            U *= I;
            V *= I;
        }
        else
        {
            I = abs(c_f); //[W m^-1]
            Q = c_q*I;
            U = c_u*I;
            V = c_v*I;
        }
    }
    else
    {
	Vector3D pos = pp->getPosition();

	uint x = uint((pos.X() + 0.5 * sidelength) / sidelength * double(bins));
	uint y = uint((pos.Y() + 0.5 * sidelength) / sidelength * double(bins));

        F = f(x, y);
        T = temp(x, y);
        Q = q(x, y);
        U = u(x, y);
        V = v(x, y);

        pl = CMathFunctions::planck(wavelength_list[wID], T); //[W m^-2 m^-1]
        I = F * pl;                                           //[W m^-1] energy per second and wavelength
        Q *= I;
        U *= I;
        V *= I;
    }

    res.set(I, Q, U, V);

    return res;
}

void CSourceBackground::setParameter(parameters & param, uint p)
{
    dlist values;
    if(param.getNrOfBackgroundSources() == 0)
        values.resize(10, 0);
    else
        values = param.getBackgroundSources();

    bins = 1;
    max_len = 1;

    c_f = values[p + 0];
    c_temp = values[p + 1];
    c_q = values[p + 2];
    c_u = values[p + 3];
    c_v = values[p + 4];
    rot_angle1 = values[p + 5];
    rot_angle2 = values[p + 6];

    constant = true;
    init = true;

    nr_of_photons = ullong(values[p + 7]);
}

ullong CSourceBackground::getNrOfPhotons()
{
    return ullong(bins * bins) * nr_of_photons;
}

uint CSourceBackground::getBins()
{
    return bins;
}

void CSourceBackground::setOrientation(Vector3D n1, Vector3D n2, double _rot_angle1, double _rot_angle2)
{
    rot_angle1 = _rot_angle1;
    rot_angle2 = _rot_angle2;

    ex.set(1, 0, 0);
    ey.set(0, 1, 0);
    ez.set(0, 0, 1);

    double cos_a = cos(rot_angle1);
    double sin_a = sin(rot_angle1);

    ex.rot(n1, cos_a, sin_a);
    ey.rot(n1, cos_a, sin_a);
    ez.rot(n1, cos_a, sin_a);

    cos_a = cos(rot_angle2);
    sin_a = sin(rot_angle2);

    ex.rot(n2, cos_a, sin_a);
    ey.rot(n2, cos_a, sin_a);
    ez.rot(n2, cos_a, sin_a);

    ex.normalize();
    ey.normalize();
    ez.normalize();
}
