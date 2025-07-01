/************************************************************************************
*                      POLARIS: POLArized RadIation Simulator                       *
*                         Copyright (C) 2018 Stefan Reissl                          *
************************************************************************************/

#include "GasSpecies.hpp"
#include "CommandParser.hpp"

// This function is based on
// Mol3d: 3D line and dust continuum radiative transfer code
// by Florian Ober 2015, Email: fober@astrophysik.uni-kiel.de

bool CGasSpecies::calcLTE(CGridBasic * grid, bool full)
{
    uint nr_of_spectral_lines = getNrOfSpectralLines();
    float last_percentage = 0;
    long cell_count = 0;
    long max_cells = grid->getMaxDataCells();
    bool no_error = true;

    cout << CLR_LINE;
    cout << "-> Calculating LTE level population  : 0.0 [%]               \r";

    #pragma omp parallel for schedule(dynamic)
    for(long i_cell = 0; i_cell < long(max_cells); i_cell++)
    {
        cell_basic * cell = grid->getCellFromIndex(i_cell);
        // Calculate percentage of total progress per source
        float percentage = 100 * float(cell_count) / float(max_cells);

        // Show only new percentage number if it changed
        if((percentage - last_percentage) > PERCENTAGE_STEP)
        {
            #pragma omp critical
            {
                cout << "-> Calculating LTE level population  : " << percentage << " [%]               \r";
                last_percentage = percentage;
            }
        }
        #pragma omp atomic update
        cell_count++;
        double ** tmp_lvl_pop = new double *[nr_of_energy_level];
        for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
        {
            tmp_lvl_pop[i_lvl] = new double[nr_of_sublevel[i_lvl]];
        }

        double temp_gas = grid->getGasTemperature(*cell);
        if(temp_gas < 1e-200)
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                    tmp_lvl_pop[i_lvl][i_sublvl] = 0;
            }
            tmp_lvl_pop[0][0] = 1;
        }
        else
        {
            double sum = 0;
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                // Calculate level pop for Zeeman sublevel too.
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                {
                    // Calculate LTE level population
                    tmp_lvl_pop[i_lvl][i_sublvl] =
                        exp(-con_h * getEnergyOfLevel(i_lvl) * con_c * 100.0 / (con_kB * temp_gas));

                    // If the sublevel are not treated separately, multiply by degeneracy
                    if(nr_of_sublevel[i_lvl] == 1)
                        tmp_lvl_pop[i_lvl][i_sublvl] *= g_level[i_lvl];

                    // Add the level population to the total sum
                    sum += tmp_lvl_pop[i_lvl][i_sublvl];
                }
            }

            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                {
                    tmp_lvl_pop[i_lvl][i_sublvl] /= sum;
                }
            }
        }

        if(full)
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                {
                    grid->setLvlPop(cell, i_lvl, i_sublvl, tmp_lvl_pop[i_lvl][i_sublvl]);
                }
            }
        }
        else
        {
            for(uint i_line = 0; i_line < nr_of_spectral_lines; i_line++)
            {
                uint i_trans = getTransitionFromSpectralLine(i_line);

                uint i_lvl_l = getLowerEnergyLevel(i_trans);
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl_l]; i_sublvl++)
                {
                    grid->setLvlPopLower(cell, i_line, i_sublvl, tmp_lvl_pop[i_lvl_l][i_sublvl]);
                }

                uint i_lvl_u = getUpperEnergyLevel(i_trans);
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl_u]; i_sublvl++)
                {
                    grid->setLvlPopUpper(cell, i_line, i_sublvl, tmp_lvl_pop[i_lvl_u][i_sublvl]);
                }
            }
        }

        delete[] tmp_lvl_pop;
    }
    return no_error;
}

// This function is based on
// Mol3d: 3D line and dust continuum radiative transfer code
// by Florian Ober 2015, Email: fober@astrophysik.uni-kiel.de

bool CGasSpecies::calcFEP(CGridBasic * grid, bool full)
{
    uint nr_of_total_energy_levels = getNrOfTotalEnergyLevels();
    uint nr_of_total_transitions = getNrOfTotalTransitions();
    uint nr_of_spectral_lines = getNrOfSpectralLines();
    float last_percentage = 0;
    long max_cells = grid->getMaxDataCells();
    double * J_mid = new double[nr_of_total_transitions];
    bool no_error = true;

    cout << CLR_LINE;

    for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
    {
        for(uint i_sublvl_u = 0; i_sublvl_u < getNrOfSublevelUpper(i_trans); i_sublvl_u++)
        {
            for(uint i_sublvl_l = 0; i_sublvl_l < getNrOfSublevelLower(i_trans); i_sublvl_l++)
            {
                uint i_tmp_trans = getUniqueTransitionIndex(i_trans, i_sublvl_u, i_sublvl_l);
                J_mid[i_tmp_trans] = CMathFunctions::planck_hz(getTransitionFrequency(i_trans), 2.75);
            }
        }
    }

    #pragma omp parallel for schedule(dynamic)
    for(long i_cell = 0; i_cell < long(max_cells); i_cell++)
    {
        cell_basic * cell = grid->getCellFromIndex(i_cell);
        double * tmp_lvl_pop = new double[nr_of_total_energy_levels];
        double gas_number_density = grid->getGasNumberDensity(*cell);

        // Calculate percentage of total progress per source
        float percentage = 100 * float(i_cell) / float(max_cells);

        // Show only new percentage number if it changed
        if((percentage - last_percentage) > PERCENTAGE_STEP)
        {
            #pragma omp critical
            {
                cout << "-> Calculating FEP level population  : " << last_percentage
                     << " [%]                       \r";
                last_percentage = percentage;
            }
        }

        if(gas_number_density > 1e-200)
        {
            Matrix2D A(nr_of_total_energy_levels, nr_of_total_energy_levels);
            double * b = new double[nr_of_total_energy_levels];

            double *** final_col_para = calcCollisionParameter(grid, cell);
            createMatrix(J_mid, &A, b, final_col_para);
            CMathFunctions::gauss(A, b, tmp_lvl_pop, nr_of_total_energy_levels);

            delete[] b;
            for(uint i_col_partner = 0; i_col_partner < nr_of_col_partner; i_col_partner++)
            {
                for(uint i_col_transition = 0; i_col_transition < nr_of_col_transition[i_col_partner];
                    i_col_transition++)
                {
                    delete[] final_col_para[i_col_partner][i_col_transition];
                }
                delete[] final_col_para[i_col_partner];
            }
            delete[] final_col_para;

            double sum_p = 0;
            for(uint i_lvl_tot = 0; i_lvl_tot < nr_of_total_energy_levels; i_lvl_tot++)
            {
                if(tmp_lvl_pop[i_lvl_tot] >= 0)
                    sum_p += tmp_lvl_pop[i_lvl_tot];
                else
                {
                    cout << WARNING_LINE << "Level population element not greater than zero! Level = " << i_lvl_tot
                         << ", Level pop = " << tmp_lvl_pop[i_lvl_tot] << endl;
                    no_error = false;
                }
            }
            for(uint i_lvl_tot = 0; i_lvl_tot < nr_of_total_energy_levels; i_lvl_tot++)
                tmp_lvl_pop[i_lvl_tot] /= sum_p;
        }
        else
        {
            for(uint i_lvl_tot = 0; i_lvl_tot < nr_of_total_energy_levels; i_lvl_tot++)
                tmp_lvl_pop[i_lvl_tot] = 0;
            tmp_lvl_pop[0] = 1;
        }

        if(full)
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                {
                    grid->setLvlPop(cell, i_lvl, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                }
            }
        }
        else
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_line = 0; i_line < nr_of_spectral_lines; i_line++)
                {
                    // Get indices
                    uint i_trans = getTransitionFromSpectralLine(i_line);
                    uint i_lvl_l = getLowerEnergyLevel(i_trans);
                    uint i_lvl_u = getUpperEnergyLevel(i_trans);

                    if(i_lvl == i_lvl_l)
                    {
                        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                            grid->setLvlPopLower(
                                cell, i_line, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                    }
                    else if(i_lvl == i_lvl_u)
                    {
                        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                            grid->setLvlPopUpper(
                                cell, i_line, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                    }
                }
            }
        }

        delete[] tmp_lvl_pop;
    }

    delete[] J_mid;
    return no_error;
}

// This function is based on
// Mol3d: 3D line and dust continuum radiative transfer code
// by Florian Ober 2015, Email: fober@astrophysik.uni-kiel.de
// and based on Pavlyuchenkov et. al (2007)

bool CGasSpecies::calcLVG(CGridBasic * grid, bool full)
{
    uint nr_of_total_energy_levels = getNrOfTotalEnergyLevels();
    uint nr_of_total_transitions = getNrOfTotalTransitions();
    uint nr_of_spectral_lines = getNrOfSpectralLines();
    float last_percentage = 0;
    long max_cells = grid->getMaxDataCells();
    bool no_error = true;

    cout << CLR_LINE;

    double * J_ext = new double[nr_of_total_transitions];
    for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
    {
        for(uint i_sublvl_u = 0; i_sublvl_u < getNrOfSublevelUpper(i_trans); i_sublvl_u++)
        {
            for(uint i_sublvl_l = 0; i_sublvl_l < getNrOfSublevelLower(i_trans); i_sublvl_l++)
            {
                uint i_tmp_trans = getUniqueTransitionIndex(i_trans, i_sublvl_u, i_sublvl_l);
                J_ext[i_tmp_trans] = CMathFunctions::planck_hz(getTransitionFrequency(i_trans), 2.75);
            }
        }
    }

    #pragma omp parallel for schedule(dynamic)
    for(long i_cell = 0; i_cell < long(max_cells); i_cell++)
    {
        cell_basic * cell = grid->getCellFromIndex(i_cell);
        Matrix2D A(nr_of_total_energy_levels, nr_of_total_energy_levels);

        double * J_mid = new double[nr_of_total_transitions];
        double * b = new double[nr_of_total_energy_levels];
        double * tmp_lvl_pop = new double[nr_of_total_energy_levels];
        double * old_pop = new double[nr_of_total_energy_levels];

        for(uint i = 1; i < nr_of_total_energy_levels; i++)
        {
            tmp_lvl_pop[i] = 0.0;
        }
        tmp_lvl_pop[0] = 1.0;

        // Calculate percentage of total progress per source
        float percentage = 100 * float(i_cell) / float(max_cells);

        // Show only new percentage number if it changed
        if((percentage - last_percentage) > PERCENTAGE_STEP)
        {
            #pragma omp critical
            {
                cout << "-> Calculating LVG level population : " << last_percentage
                     << " [%]                       \r";
                last_percentage = percentage;
            }
        }

        // Check temperature and density
        double temp_gas = grid->getGasTemperature(*cell);
        double dens_species = getNumberDensity(grid, *cell);
        if(temp_gas < 1e-200 || dens_species < 1e-200)
            continue;

        // Get gauss A
        double gauss_a = getGaussA(temp_gas, grid->getTurbulentVelocity(cell));

        Vector3D pos_xyz_cell = grid->getCenter(*cell);
        double abs_vel = getCellVelocity(grid, *cell, pos_xyz_cell).length();

        if(gauss_a * abs_vel < 1e-16)
            continue;

        double R_mid = sqrt(pow(pos_xyz_cell.X(), 2) + pow(pos_xyz_cell.Y(), 2));
        double L = R_mid * sqrt(2.0 / 3.0 / (gauss_a * abs_vel));
        uint i_iter = 0;

        double *** final_col_para = calcCollisionParameter(grid, cell);

        for(i_iter = 0; i_iter < MAX_LVG_ITERATIONS; i_iter++)
        {
            for(uint i_lvl = 0; i_lvl < nr_of_total_energy_levels; i_lvl++)
                old_pop[i_lvl] = tmp_lvl_pop[i_lvl];

            for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
            {
                for(uint i_sublvl_u = 0; i_sublvl_u < getNrOfSublevelUpper(i_trans); i_sublvl_u++)
                {
                    for(uint i_sublvl_l = 0; i_sublvl_l < getNrOfSublevelLower(i_trans); i_sublvl_l++)
                    {
                        uint i_tmp_trans = getUniqueTransitionIndex(i_trans, i_sublvl_u, i_sublvl_l);
                        double j, alpha;
                        calcEmissivityFromLvlPop(
                            i_trans, i_sublvl_u, i_sublvl_l, dens_species, gauss_a, tmp_lvl_pop, &j, &alpha);
                        J_mid[i_tmp_trans] = calcJFromInteractionLength(j, alpha, J_ext[i_trans], L);
                    }
                }
            }

            createMatrix(J_mid, &A, b, final_col_para);
            CMathFunctions::gauss(A, b, tmp_lvl_pop, nr_of_total_energy_levels);

            double sum_p = 0;
            for(uint j_lvl = 0; j_lvl < nr_of_total_energy_levels; j_lvl++)
            {
                if(tmp_lvl_pop[j_lvl] >= 0)
                    sum_p += tmp_lvl_pop[j_lvl];
                else
                {
                    cout << WARNING_LINE << "Level population element not greater than zero! Level = " << j_lvl
                         << ", Level pop = " << tmp_lvl_pop[j_lvl] << endl;
                    no_error = false;
                }
            }
            for(uint j_lvl = 0; j_lvl < nr_of_total_energy_levels; j_lvl++)
                tmp_lvl_pop[j_lvl] /= sum_p;

            uint j_lvl = 0;
            for(uint i_lvl = 1; i_lvl < nr_of_total_energy_levels; i_lvl++)
                if(tmp_lvl_pop[i_lvl] > tmp_lvl_pop[j_lvl])
                    j_lvl = i_lvl;

            if(i_iter > 1)
            {
                if(abs(tmp_lvl_pop[j_lvl] - old_pop[j_lvl]) /
                       (old_pop[j_lvl] + numeric_limits<double>::epsilon()) <
                   1e-2)
                {
                    break;
                }
            }
        }
        if(i_iter == MAX_LVG_ITERATIONS)
            cout << WARNING_LINE << "Maximum iteration needed in cell: " << i_cell << endl;

        if(full)
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                {
                    grid->setLvlPop(cell, i_lvl, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                }
            }
        }
        else
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_line = 0; i_line < nr_of_spectral_lines; i_line++)
                {
                    // Get indices
                    uint i_trans = getTransitionFromSpectralLine(i_line);
                    uint i_lvl_l = getLowerEnergyLevel(i_trans);
                    uint i_lvl_u = getUpperEnergyLevel(i_trans);

                    if(i_lvl == i_lvl_l)
                    {
                        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                            grid->setLvlPopLower(
                                cell, i_line, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                    }
                    else if(i_lvl == i_lvl_u)
                    {
                        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                            grid->setLvlPopUpper(
                                cell, i_line, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                    }
                }
            }
        }

        delete[] J_mid;
        delete[] b;
        delete[] tmp_lvl_pop;
        delete[] old_pop;
        for(uint i_col_partner = 0; i_col_partner < nr_of_col_partner; i_col_partner++)
        {
            for(uint i_col_transition = 0; i_col_transition < nr_of_col_transition[i_col_partner];
                i_col_transition++)
            {
                delete[] final_col_para[i_col_partner][i_col_transition];
            }
            delete[] final_col_para[i_col_partner];
        }
        delete[] final_col_para;
    }
    delete[] J_ext;
    return no_error;
}

bool CGasSpecies::calcDeguchiWatsonLVG(CGridBasic * grid, bool full)
{
    uint nr_of_total_energy_levels = getNrOfTotalEnergyLevels();
    uint nr_of_total_transitions = getNrOfTotalTransitions();
    uint nr_of_spectral_lines = getNrOfSpectralLines();
    float last_percentage = 0;
    long max_cells = grid->getMaxDataCells();
    bool no_error = true;

    cout << CLR_LINE;

    double * J_ext = new double[nr_of_total_transitions];
    for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
    {
        for(uint i_sublvl_u = 0; i_sublvl_u < getNrOfSublevelUpper(i_trans); i_sublvl_u++)
        {
            for(uint i_sublvl_l = 0; i_sublvl_l < getNrOfSublevelLower(i_trans); i_sublvl_l++)
            {
                uint i_tmp_trans = getUniqueTransitionIndex(i_trans, i_sublvl_u, i_sublvl_l);
                J_ext[i_tmp_trans] = CMathFunctions::planck_hz(getTransitionFrequency(i_trans), 2.75);
            }
        }
    }

    #pragma omp parallel for schedule(dynamic)
    for(long i_cell = 0; i_cell < long(max_cells); i_cell++)
    {
        cell_basic * cell = grid->getCellFromIndex(i_cell);
        Matrix2D A(nr_of_total_energy_levels, nr_of_total_energy_levels);

        double * J_mid = new double[nr_of_total_transitions];
        double * b = new double[nr_of_total_energy_levels];
        double * tmp_lvl_pop = new double[nr_of_total_energy_levels];
        double * old_pop = new double[nr_of_total_energy_levels];

        for(uint i = 1; i < nr_of_total_energy_levels; i++)
        {
            tmp_lvl_pop[i] = 0.0;
        }
        tmp_lvl_pop[0] = 1.0;

        // Calculate percentage of total progress per source
        float percentage = 100 * float(i_cell) / float(max_cells);

        // Show only new percentage number if it changed
        if((percentage - last_percentage) > PERCENTAGE_STEP)
        {
            #pragma omp critical
            {
                cout << "-> Calculating LVG level population : " << last_percentage
                     << " [%]                       \r";
                last_percentage = percentage;
            }
        }

        // Check temperature and density
        double temp_gas = grid->getGasTemperature(*cell);
        double dens_species = getNumberDensity(grid, *cell);
        if(temp_gas < 1e-200 || dens_species < 1e-200)
            continue;

        // Get gauss a
        double gauss_a = getGaussA(temp_gas, grid->getTurbulentVelocity(cell));

        double *** final_col_para = calcCollisionParameter(grid, cell);
        uint i_iter = 0;
        for(i_iter = 0; i_iter < MAX_LVG_ITERATIONS; i_iter++)
        {
            for(uint i_lvl = 0; i_lvl < nr_of_total_energy_levels; i_lvl++)
                old_pop[i_lvl] = tmp_lvl_pop[i_lvl];

            for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
            {
                uint i_lvl_l = getLowerEnergyLevel(i_trans);
                uint i_lvl_u = getUpperEnergyLevel(i_trans);

                for(uint i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel[i_lvl_u]; i_sublvl_u++)
                {
                    // Calculate the quantum number of the upper energy level
                    float sublvl_u = -getMaxM(i_lvl_u) + i_sublvl_u;

                    for(uint i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel[i_lvl_l]; i_sublvl_l++)
                    {
                        // Calculate the quantum number of the lower energy level
                        float sublvl_l = -getMaxM(i_lvl_l) + i_sublvl_l;

                        uint i_tmp_trans = getUniqueTransitionIndex(i_trans, i_sublvl_u, i_sublvl_l);

                        double j, alpha;
                        calcEmissivityFromLvlPop(
                            i_trans, i_sublvl_u, i_sublvl_l, dens_species, gauss_a, tmp_lvl_pop, &j, &alpha);

                        uint nr_of_theta = 181;
                        dlist solid_angle(nr_of_theta), J_theta(nr_of_theta);
                        double tau_0 = grid->getDustTemperature(*cell) / 100;
                        for(uint sth = 0; sth < nr_of_theta; sth++)
                        {
                            double theta = PI * double(sth) / double(nr_of_theta - 1);
                            double cos_theta = cos(theta);
                            double sin_theta = sin(theta);

                            // Calculate the modified angle for integration
                            solid_angle[sth] = -PIx2 * cos_theta;

                            double tau = tau_0 / (cos_theta * cos_theta);
                            double J_tmp = calcJFromOpticalDepth(j, alpha, J_ext[i_trans], tau);
                            switch(int(sublvl_l - sublvl_u))
                            {
                                case TRANS_PI:
                                    // Update radiation field
                                    J_theta[sth] = J_tmp;
                                    if(isTransZeemanSplit(i_trans))
                                        J_theta[sth] *= (3.0 / 2.0) * (sin_theta * sin_theta);
                                    break;
                                case TRANS_SIGMA_P:
                                case TRANS_SIGMA_M:
                                    // Update radiation field
                                    J_theta[sth] = (3.0 / 4.0) * (1 + cos_theta * cos_theta) * J_tmp;
                                    break;
                            }
                        }
                        J_mid[i_tmp_trans] =
                            CMathFunctions::integ(solid_angle, J_theta, 0, nr_of_theta - 1) / PIx4;
                    }
                }
            }

            createMatrix(J_mid, &A, b, final_col_para);
            CMathFunctions::gauss(A, b, tmp_lvl_pop, nr_of_total_energy_levels);

            // if(i_iter == 0)
            // {
            //     for(uint i_col_transition = 0; i_col_transition < getNrCollisionTransitions(0);
            //         i_col_transition++)
            //     {
            //         // Get level indices for lower and upper energy level
            //         uint i_lvl_u = getUpperCollisionLevel(0, i_col_transition);
            //         uint i_lvl_l = getLowerCollisionLevel(0, i_col_transition);
            //         if(i_lvl_u == 1 && i_lvl_l == 0)
            //             cout << final_col_para[0][i_col_transition][0] / getEinsteinA(0) << endl;
            //     }
            // }

            double sum_p = 0;
            for(uint j_lvl = 0; j_lvl < nr_of_total_energy_levels; j_lvl++)
            {
                if(tmp_lvl_pop[j_lvl] >= 0)
                    sum_p += tmp_lvl_pop[j_lvl];
                else
                {
                    cout << WARNING_LINE << "Level population element not greater than zero! Level = " << j_lvl
                         << ", Level pop = " << tmp_lvl_pop[j_lvl] << endl;
                    no_error = false;
                }
            }
            for(uint j_lvl = 0; j_lvl < nr_of_total_energy_levels; j_lvl++)
                tmp_lvl_pop[j_lvl] /= sum_p;

            if(i_iter > 1)
            {
                uint j_lvl = 0;
                for(uint i_lvl = 1; i_lvl < nr_of_total_energy_levels; i_lvl++)
                    if(tmp_lvl_pop[i_lvl] > tmp_lvl_pop[j_lvl])
                        j_lvl = i_lvl;

                if(abs(tmp_lvl_pop[j_lvl] - old_pop[j_lvl]) /
                       (tmp_lvl_pop[j_lvl] + old_pop[j_lvl] + __DBL_EPSILON__) <
                   1e-3)
                    break;
            }
        }
        if(i_iter == MAX_LVG_ITERATIONS)
            cout << WARNING_LINE << "Maximum iteration needed in cell: " << i_cell << endl;

        if(full)
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                {
                    grid->setLvlPop(cell, i_lvl, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                }
            }
        }
        else
        {
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                for(uint i_line = 0; i_line < nr_of_spectral_lines; i_line++)
                {
                    // Get indices
                    uint i_trans = getTransitionFromSpectralLine(i_line);
                    uint i_lvl_l = getLowerEnergyLevel(i_trans);
                    uint i_lvl_u = getUpperEnergyLevel(i_trans);

                    if(i_lvl == i_lvl_l)
                    {
                        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                            grid->setLvlPopLower(
                                cell, i_line, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                    }
                    else if(i_lvl == i_lvl_u)
                    {
                        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
                            grid->setLvlPopUpper(
                                cell, i_line, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
                    }
                }
            }
        }

        // cout << CLR_LINE;
        // cout << grid->getDustTemperature(*cell) / 100 << TAB
        //      << tmp_lvl_pop[getUniqueLevelIndex(1, 0)] / tmp_lvl_pop[getUniqueLevelIndex(1, 1)] << endl;

        delete[] J_mid;
        delete[] b;
        delete[] tmp_lvl_pop;
        delete[] old_pop;
        for(uint i_col_partner = 0; i_col_partner < nr_of_col_partner; i_col_partner++)
        {
            for(uint i_col_transition = 0; i_col_transition < nr_of_col_transition[i_col_partner];
                i_col_transition++)
            {
                delete[] final_col_para[i_col_partner][i_col_transition];
            }
            delete[] final_col_para[i_col_partner];
        }
        delete[] final_col_para;
    }
    delete[] J_ext;
    return no_error;
}

bool CGasSpecies::updateLevelPopulation(CGridBasic * grid, cell_basic * cell, double * J_total)
{
    // uint nr_of_energy_level = getNrOfEnergyLevels();
    uint nr_of_total_energy_levels = getNrOfTotalEnergyLevels();

    double * tmp_lvl_pop = new double[nr_of_total_energy_levels];

    double gas_number_density = grid->getGasNumberDensity(*cell);
    if(gas_number_density > 1e-200)
    {
        Matrix2D A(nr_of_total_energy_levels, nr_of_total_energy_levels);
        double * b = new double[nr_of_total_energy_levels];

        double *** final_col_para = calcCollisionParameter(grid, cell);
        createMatrix(J_total, &A, b, final_col_para);
        CMathFunctions::gauss(A, b, tmp_lvl_pop, nr_of_total_energy_levels);

        delete[] b;
        for(uint i_col_partner = 0; i_col_partner < nr_of_col_partner; i_col_partner++)
        {
            for(uint i_col_transition = 0; i_col_transition < nr_of_col_transition[i_col_partner];
                i_col_transition++)
            {
                delete[] final_col_para[i_col_partner][i_col_transition];
            }
            delete[] final_col_para[i_col_partner];
        }
        delete[] final_col_para;

        double sum_p = 0;
        for(uint i_lvl_tot = 0; i_lvl_tot < nr_of_total_energy_levels; i_lvl_tot++)
        {
            if(tmp_lvl_pop[i_lvl_tot] >= 0)
                sum_p += tmp_lvl_pop[i_lvl_tot];
            else
            {
                cout << WARNING_LINE << "Level population element not greater than zero!" << endl;
                return false;
            }
        }
        for(uint i_lvl_tot = 0; i_lvl_tot < nr_of_total_energy_levels; i_lvl_tot++)
            tmp_lvl_pop[i_lvl_tot] /= sum_p;
    }
    else
    {
        for(uint i_lvl_tot = 0; i_lvl_tot < nr_of_total_energy_levels; i_lvl_tot++)
            tmp_lvl_pop[i_lvl_tot] = 0;
        tmp_lvl_pop[0] = 1;
    }

    for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
    {
        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
        {
            grid->setLvlPop(cell, i_lvl, i_sublvl, tmp_lvl_pop[getUniqueLevelIndex(i_lvl, i_sublvl)]);
        }
    }

    delete[] tmp_lvl_pop;
    return true;
}

// This function is based on
// Mol3d: 3D line and dust continuum radiative transfer code
// by Florian Ober 2015, Email: fober@astrophysik.uni-kiel.de

double *** CGasSpecies::calcCollisionParameter(CGridBasic * grid, cell_basic * cell)
{
    // Init variables
    double temp_gas = grid->getGasTemperature(*cell);
    uint nr_of_col_partner = getNrOfCollisionPartner();
    double *** final_col_para = new double **[nr_of_col_partner];

    for(uint i_col_partner = 0; i_col_partner < nr_of_col_partner; i_col_partner++)
    {
        uint nr_of_col_transition = getNrCollisionTransitions(i_col_partner);

        // Resize Matrix for lu and ul and all transitions
        final_col_para[i_col_partner] = new double *[nr_of_col_transition];

        // Get density of collision partner
        double dens = getColPartnerDensity(grid, cell, i_col_partner);

        bool skip = false;
        for(uint i_col_transition = 0; i_col_transition < nr_of_col_transition; i_col_transition++)
            if(getUpperCollisionLevel(i_col_partner, i_col_transition) == 0)
                skip = true;

        if(!skip)
        {
            uint hi_i = 0;
            if(temp_gas < getCollisionTemp(i_col_partner, 0))
                hi_i = 0;
            else if(temp_gas > getCollisionTemp(i_col_partner, getNrCollisionTemps(i_col_partner) - 1))
                hi_i = getNrCollisionTemps(i_col_partner) - 2;
            else
                hi_i = CMathFunctions::biListIndexSearch(
                    temp_gas, collision_temp[i_col_partner], getNrCollisionTemps(i_col_partner));

            for(uint i_col_transition = 0; i_col_transition < nr_of_col_transition; i_col_transition++)
            {
                // Calculate collision rates
                dlist rates = calcCollisionRate(i_col_partner, i_col_transition, hi_i, temp_gas);

                final_col_para[i_col_partner][i_col_transition] = new double[2];
                final_col_para[i_col_partner][i_col_transition][0] = rates[0] * dens;
                final_col_para[i_col_partner][i_col_transition][1] = rates[1] * dens;
            }
        }
    }
    return final_col_para;
}

double CGasSpecies::getColPartnerDensity(CGridBasic * grid, cell_basic * cell, uint i_col_partner)
{
    double dens = 0;
    switch(getOrientation_H2(i_col_partner))
    {
        case(COL_H2_FULL):
            dens = grid->getGasNumberDensity(*cell);
            break;
        case(COL_H2_PARA):
            dens = grid->getGasNumberDensity(*cell) * 0.25;
            break;
        case(COL_H2_ORTH):
            dens = grid->getGasNumberDensity(*cell) * 0.75;
            break;
        case(COL_HE_FULL):
            dens = grid->getGasNumberDensity(*cell) * max(0.0, grid->getMu() - 1.0);
            break;
        default:
            dens = 0;
            break;
    }
    return dens;
}

dlist CGasSpecies::calcCollisionRate(uint i_col_partner, uint i_col_transition, uint hi_i, double temp_gas)
{
    dlist col_mtr_tmp(2);

    double interp = CMathFunctions::interpolate(getCollisionTemp(i_col_partner, hi_i),
                                                getCollisionTemp(i_col_partner, hi_i + 1),
                                                getCollisionMatrix(i_col_partner, i_col_transition, hi_i),
                                                getCollisionMatrix(i_col_partner, i_col_transition, hi_i + 1),
                                                temp_gas);

    if(interp > 0)
    {
        col_mtr_tmp[0] = interp;

        uint i_col_lvl_u = getUpperCollisionLevel(i_col_partner, i_col_transition);
        uint i_col_lvl_l = getLowerCollisionLevel(i_col_partner, i_col_transition);

        col_mtr_tmp[1] =
            col_mtr_tmp[0] * g_level[i_col_lvl_u] / g_level[i_col_lvl_l] *
            exp(-con_h * con_c * 100.0 * (getEnergyOfLevel(i_col_lvl_u) - getEnergyOfLevel(i_col_lvl_l)) /
                (con_kB * temp_gas));
    }

    return col_mtr_tmp;
}

void CGasSpecies::createMatrix(double * J_mid, Matrix2D * A, double * b, double *** final_col_para)
{
    // Get number of colision partner and energy levels
    uint nr_of_col_partner = getNrOfCollisionPartner();
    uint nr_of_energy_level = getNrOfEnergyLevels();

    /*
     * A will be multiplied by the lvl pop. To take sublevels into account,
     * each entry has to be multiplied by the number of sublevels in the upper level.
     * Each entry that should have been multiplied by the number of sublevels in the lower level
     * is cancelling out due to the way how einstein_Blu and the collision rates are defined.
     */

    for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
    {
        // Get level indices for lower and upper energy level
        uint i_lvl_u = getUpperEnergyLevel(i_trans);
        uint i_lvl_l = getLowerEnergyLevel(i_trans);

        for(uint i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel[i_lvl_u]; i_sublvl_u++)
        {
            // Get index that goes over all level and sublevel
            uint i_tmp_lvl_u = getUniqueLevelIndex(i_lvl_u, i_sublvl_u);

            for(uint i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel[i_lvl_l]; i_sublvl_l++)
            {
                // Get index that goes over all level and sublevel
                uint i_tmp_lvl_l = getUniqueLevelIndex(i_lvl_l, i_sublvl_l);

                // Get index that goes over all transitions and transitions between sublevel
                uint i_tmp_trans = getUniqueTransitionIndex(i_trans, i_sublvl_u, i_sublvl_l);

                // Level u can be populated by level l
                A->addValue(i_tmp_lvl_u,
                            i_tmp_lvl_l,
                            getEinsteinBlu(i_trans, i_sublvl_u, i_sublvl_l) * J_mid[i_tmp_trans]);

                // Level u can be depopulated to level l
                A->addValue(i_tmp_lvl_u,
                            i_tmp_lvl_u,
                            -(getEinsteinA(i_trans, i_sublvl_u, i_sublvl_l) +
                              getEinsteinBul(i_trans, i_sublvl_u, i_sublvl_l) * J_mid[i_tmp_trans]));

                // Level l can be populated by level u
                A->addValue(i_tmp_lvl_l,
                            i_tmp_lvl_u,
                            (getEinsteinA(i_trans, i_sublvl_u, i_sublvl_l) +
                             getEinsteinBul(i_trans, i_sublvl_u, i_sublvl_l) * J_mid[i_tmp_trans]));

                // Level l can be depopulated to level u
                A->addValue(i_tmp_lvl_l,
                            i_tmp_lvl_l,
                            -getEinsteinBlu(i_trans, i_sublvl_u, i_sublvl_l) * J_mid[i_tmp_trans]);
            }
        }
    }

    for(uint i_col_partner = 0; i_col_partner < nr_of_col_partner; i_col_partner++)
    {
        // Get number of transitions covered by the collisions
        uint nr_of_col_transition = getNrCollisionTransitions(i_col_partner);

        // Add collision rates to the system of equations for each transition
        for(uint i_col_transition = 0; i_col_transition < nr_of_col_transition; i_col_transition++)
        {
            // Get level indices for lower and upper energy level
            uint i_lvl_u = getUpperCollisionLevel(i_col_partner, i_col_transition);
            uint i_lvl_l = getLowerCollisionLevel(i_col_partner, i_col_transition);

            for(uint i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel[i_lvl_u]; i_sublvl_u++)
            {
                // Get index that goes over all level and sublevel
                uint i_tmp_lvl_u = getUniqueLevelIndex(i_lvl_u, i_sublvl_u);

                for(uint i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel[i_lvl_l]; i_sublvl_l++)
                {
                    // Get index that goes over all level and sublevel
                    uint i_tmp_lvl_l = getUniqueLevelIndex(i_lvl_l, i_sublvl_l);

                    // Level u can be populated by level l
                    A->addValue(i_tmp_lvl_u,
                                i_tmp_lvl_l,
                                final_col_para[i_col_partner][i_col_transition][1] / nr_of_sublevel[i_lvl_u]);

                    // Level u can be depopulated to level l
                    A->addValue(i_tmp_lvl_u,
                                i_tmp_lvl_u,
                                -final_col_para[i_col_partner][i_col_transition][0] /
                                    nr_of_sublevel[i_lvl_l]);

                    // Level l can be populated by level u
                    A->addValue(i_tmp_lvl_l,
                                i_tmp_lvl_u,
                                final_col_para[i_col_partner][i_col_transition][0] / nr_of_sublevel[i_lvl_l]);

                    // Level l can be depopulated to level u
                    A->addValue(i_tmp_lvl_l,
                                i_tmp_lvl_l,
                                -final_col_para[i_col_partner][i_col_transition][1] /
                                    nr_of_sublevel[i_lvl_u]);
                }
            }
        }
    }

    // Setting startpoint for gaussian solving
    for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
    {
        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
        {
            uint i_tmp_lvl = getUniqueLevelIndex(i_lvl, i_sublvl);
            A->setValue(0, i_tmp_lvl, 1.0);
            b[i_tmp_lvl] = 0;
        }
    }
    b[0] = 1.0;
}

void CGasSpecies::calcEmissivity(CGridBasic * grid,
                                 const photon_package & pp,
                                 uint i_trans,
                                 double velocity,
                                 const LineBroadening & line_broadening,
                                 const MagFieldInfo & mag_field_info,
                                 StokesVector * line_emissivity,
                                 Matrix2D * line_absorption_matrix) const
{
    // Reset absorption matrix and emissivity
    line_absorption_matrix->resize(4, 4);
    *line_emissivity = 0;

    if(isTransZeemanSplit(i_trans))
    {
        // Calculate the line matrix from rotation polarization matrix and line shape
        calcEmissivityZeeman(grid,
                             pp,
                             i_trans,
                             velocity,
                             line_broadening,
                             mag_field_info,
                             line_emissivity,
                             line_absorption_matrix);
    }
    else
    {
        // Get level indices for lower and upper energy level
        uint i_lvl_u = getUpperEnergyLevel(i_trans);
        uint i_lvl_l = getLowerEnergyLevel(i_trans);

        // Calculate the optical depth of the gas particles in the current cell
        double absorption = (grid->getLvlPop(pp, i_lvl_l) * getEinsteinBlu(i_trans) -
                             grid->getLvlPop(pp, i_lvl_u) * getEinsteinBul(i_trans)) *
                            con_eps * line_broadening.gauss_a;

        // Calculate the emissivity of the gas particles in the current cell
        double emission =
            grid->getLvlPop(pp, i_lvl_u) * getEinsteinA(i_trans) * con_eps * line_broadening.gauss_a;

        // Calculate the line matrix from rotation polarization matrix and line shape
        getGaussLineMatrix(grid, pp, velocity, line_absorption_matrix);

        // Calculate the Emissivity of the gas particles in the current cell
        *line_emissivity = *line_absorption_matrix * StokesVector(emission, 0, 0, 0);

        // Calculate the line matrix from rotation polarization matrix and line shape
        *line_absorption_matrix *= absorption;
    }
}

void CGasSpecies::calcEmissivityZeeman(CGridBasic * grid,
                                       const photon_package & pp,
                                       uint i_trans,
                                       double velocity,
                                       const LineBroadening & line_broadening,
                                       const MagFieldInfo & mfo,
                                       StokesVector * line_emissivity,
                                       Matrix2D * line_absorption_matrix) const
{
    // Get level indices for lower and upper energy level
    uint i_lvl_u = getUpperEnergyLevel(i_trans);
    uint i_lvl_l = getLowerEnergyLevel(i_trans);

    // Init the current value of the line function as a complex value
    complex<double> line_function;
    
    //uint tmp_counter=0;

    // Init temporary matrix
    Matrix2D * tmp_matrix = new Matrix2D(4, 4);

    // Get rest frequency of transition
    double trans_frequency = pp.getTransFrequency();

    // Calculate the contribution of each allowed transition between Zeeman sublevels
    for(uint i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel[i_lvl_u]; i_sublvl_u++)
    {
        // Calculate the quantum number of the upper energy level
        float sublvl_u = -getMaxM(i_lvl_u) + i_sublvl_u;

        for(uint i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel[i_lvl_l]; i_sublvl_l++)
        {
            // Calculate the quantum number of the lower energy level
            float sublvl_l = -getMaxM(i_lvl_l) + i_sublvl_l;

            // Skip forbidden transitions
            if(abs(sublvl_l - sublvl_u) > 1)
                continue;

            // Calculate the emissivity of the gas particles in the current cell
            // nr_of_sublevel to take the line strength into account that is normalized to the full lvl pop
            double emission = con_eps * line_broadening.gauss_a * grid->getLvlPop(pp, i_lvl_u, i_sublvl_u) *
                              getEinsteinA(i_trans, i_sublvl_u, i_sublvl_l);

            // Calculate the optical depth of the gas particles in the current cell
            // nr_of_sublevel to take the line strength into account that is normalized to the full lvl pop
            double absorption =
                con_eps * line_broadening.gauss_a *
                (grid->getLvlPop(pp, i_lvl_l, i_sublvl_l) * getEinsteinBlu(i_trans, i_sublvl_u, i_sublvl_l) -
                 grid->getLvlPop(pp, i_lvl_u, i_sublvl_u) * getEinsteinBul(i_trans, i_sublvl_u, i_sublvl_l));

            // Calculate the frequency shift in relation to the not shifted line peak
            // Delta nu = (B * mu_Bohr) / h * (m' * g' - m'' * g'')
            double freq_shift = mfo.mag_field.length() * con_mb / con_h *
                                (sublvl_u * getLande(i_lvl_u) - sublvl_l * getLande(i_lvl_l));

            // Calculate the frequency value of the current velocity channel in
            // relation to the peak of the line function
            double f_doppler = (freq_shift + CMathFunctions::Velo2Freq(velocity, trans_frequency)) *
                               (con_c * line_broadening.gauss_a) / trans_frequency;

            // Calculate the line function value at the frequency
            // of the current velocity channel
            line_function = getLineShape_AB(f_doppler, line_broadening.voigt_a);

            // Multiply the line function value by PIsq for normalization
            double mult_A = real(line_function) / PIsq;

            // Multiply the line function value by PIsq for normalization
            // and divide it by 2 to take the difference between the faddeeva
            // and Faraday-Voigt function into account
            double mult_B = imag(line_function) / (PIsq * 2.0);

            // Reset temporary matrix
            tmp_matrix->fill(0);

            // Use the correct propagation matrix and relative line strength that
            // depends on the current type of Zeeman transition (pi, sigma_-, sigma_+)
            // PI        : line_function * 1.0 -> (/ 1.0)
            // Sigma_+/- : line_function * 1.0 / 2.0 -> (/ 2.0)
            // EinsteinA is in total 1.0 / 3.0 -> (* 3.0)
            switch(int(sublvl_l - sublvl_u))
            {
                case TRANS_SIGMA_P:
                    // Get the propagation matrix for extinction/emission
                    CMathFunctions::getPropMatrixASigmaP(mfo.cos_theta,
                                                         mfo.sin_theta,
                                                         mfo.cos_2_phi,
                                                         mfo.sin_2_phi,
                                                         mult_A * (3.0 / 2.0),
                                                         tmp_matrix);

                    // Get the propagation matrix for Faraday rotation
                    CMathFunctions::getPropMatrixBSigmaP(mfo.cos_theta,
                                                         mfo.sin_theta,
                                                         mfo.cos_2_phi,
                                                         mfo.sin_2_phi,
                                                         mult_B * (3.0 / 2.0),
                                                         tmp_matrix);
                    break;
                case TRANS_PI:
                    // Get the propagation matrix for extinction/emission
                    CMathFunctions::getPropMatrixAPi(
                        mfo.cos_theta, mfo.sin_theta, mfo.cos_2_phi, mfo.sin_2_phi, mult_A * 3.0, tmp_matrix);

                    // Get the propagation matrix for Faraday rotation
                    CMathFunctions::getPropMatrixBPi(
                        mfo.cos_theta, mfo.sin_theta, mfo.cos_2_phi, mfo.sin_2_phi, mult_B * 3.0, tmp_matrix);
                    break;
                case TRANS_SIGMA_M:
                    // Get the propagation matrix for extinction/emission
                    CMathFunctions::getPropMatrixASigmaM(mfo.cos_theta,
                                                         mfo.sin_theta,
                                                         mfo.cos_2_phi,
                                                         mfo.sin_2_phi,
                                                         mult_A * (3.0 / 2.0),
                                                         tmp_matrix);

                    // Get the propagation matrix for Faraday rotation
                    CMathFunctions::getPropMatrixBSigmaM(mfo.cos_theta,
                                                         mfo.sin_theta,
                                                         mfo.cos_2_phi,
                                                         mfo.sin_2_phi,
                                                         mult_B * (3.0 / 2.0),
                                                         tmp_matrix);
                    break;
            }

            // Calculate the Emissivity and Absorption matrix of the gas particles in the current cell
            *line_emissivity += *tmp_matrix * StokesVector(emission, 0, 0, 0);
            *line_absorption_matrix += *tmp_matrix * absorption;
        }
    }

    delete tmp_matrix;
}

bool CGasSpecies::readGasParamaterFile(string _filename, uint id, uint max)
{
    uint line_counter, cmd_counter;
    uint pos_counter = 0;
    uint row_offset, i_col_partner, i_col_transition;
    CCommandParser ps;
    fstream reader(_filename.c_str());
    unsigned char ru[4] = { '|', '/', '-', '\\' };
    string line;
    dlist values;

    cout << CLR_LINE;

    if(reader.fail())
    {
        cout << ERROR_LINE << "Cannot open gas_species catalog:" << endl;
        cout << _filename << endl;
        return false;
    }

    line_counter = 0;
    cmd_counter = 0;

    row_offset = 0;
    i_col_partner = 0;
    i_col_transition = 0;

    uint char_counter = 0;

    while(getline(reader, line))
    {
        line_counter++;

        if(line_counter % 20 == 0)
        {
            char_counter++;
            cout << "-> Reading gas species file nr. " << id + 1 << " of " << max << " : "
                 << ru[(uint)char_counter % 4] << "           \r";
        }

        ps.formatLine(line);

        if(line.size() == 0)
            continue;

        if(cmd_counter != 0)
        {
            values = ps.parseValues(line);
            if(values.size() == 0)
                continue;
        }

        cmd_counter++;

        if(cmd_counter == 1)
            stringID = line;
        else if(cmd_counter == 2)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }
            molecular_weight = values[0];
        }
        else if(cmd_counter == 3)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }
            nr_of_energy_level = uint(values[0]);

            // Init pointer array
            energy_level = new double[nr_of_energy_level];
            g_level = new double[nr_of_energy_level];
            quantum_numbers = new double[nr_of_energy_level];

            // Set number of sublevel to one and increase it in case of Zeeman
            nr_of_sublevel = new int[nr_of_energy_level];
            for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
            {
                nr_of_sublevel[i_lvl] = 1;
            }
        }
        else if(cmd_counter < 4 + nr_of_energy_level && cmd_counter > 3)
        {
            if(values.size() < 4)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }

            // ENERGIES(cm^-1)
            energy_level[pos_counter] = values[1];
            // WEIGHT
            g_level[pos_counter] = values[2];
            // Quantum numbers for corresponding energy level
            quantum_numbers[pos_counter] = values[3];

            // For each energy level
            pos_counter++;
        }
        else if(cmd_counter == 4 + nr_of_energy_level)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }
            nr_of_transitions = uint(values[0]);

            // Reset and switch to transitions
            pos_counter = 0;

            // Init pointer arrays
            upper_level = new int[nr_of_transitions];
            lower_level = new int[nr_of_transitions];
            trans_freq = new double[nr_of_transitions];
            trans_inner_energy = new double[nr_of_transitions];

            // Init pointer arrays for the einstein coefficients
            trans_einstA = new double *[nr_of_transitions];
            trans_einstB_ul = new double *[nr_of_transitions];
            trans_einstB_lu = new double *[nr_of_transitions];

            // Only one entry, but more if Zeeman sublevels are treated
            for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
            {
                trans_einstA[i_trans] = new double[1];
                trans_einstB_ul[i_trans] = new double[1];
                trans_einstB_lu[i_trans] = new double[1];
            }

            // Init list if a transition is zeeman split
            trans_is_zeeman_split = new bool[nr_of_transitions];
            for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
            {
                trans_is_zeeman_split[i_trans] = false;
            }
        }
        else if(cmd_counter < 5 + nr_of_energy_level + nr_of_transitions &&
                cmd_counter > 4 + nr_of_energy_level)
        {
            if(values.size() != 6)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }

            for(uint i_line = 0; i_line < nr_of_spectral_lines; i_line++)
            {
                if(spectral_lines[i_line] == int(values[0] - 1))
                {
                    unique_spectral_lines.push_back(i_line);
                    break;
                }
            }

            // UP (as index starting with 0)
            upper_level[pos_counter] = int(values[1] - 1);
            // LOW (as index starting with 0)
            lower_level[pos_counter] = int(values[2] - 1);
            // EINSTEIN A(s^-1)
            trans_einstA[pos_counter][0] = values[3];
            // FREQ(GHz -> Hz)
            trans_freq[pos_counter] = values[4] * 1e9;
            // E_u(K)
            trans_inner_energy[pos_counter] = values[5];

            // EINSTEIN B_ul(s^-1)
            trans_einstB_ul[pos_counter][0] = getEinsteinA(pos_counter) *
                                              pow(con_c / trans_freq[pos_counter], 2.0) /
                                              (2.0 * con_h * trans_freq[pos_counter]);

            // EINSTEIN B_lu(s^-1)
            trans_einstB_lu[pos_counter][0] = g_level[upper_level[pos_counter]] /
                                              g_level[lower_level[pos_counter]] * getEinsteinBul(pos_counter);

            // For each transition
            pos_counter++;
        }
        else if(cmd_counter == 5 + nr_of_energy_level + nr_of_transitions)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }
            nr_of_col_partner = uint(values[0]);

            nr_of_col_transition = new int[nr_of_col_partner];
            nr_of_col_temp = new int[nr_of_col_partner];
            orientation_H2 = new int[nr_of_col_partner];

            collision_temp = new double *[nr_of_col_partner];
            col_upper = new uint *[nr_of_col_partner];
            col_lower = new uint *[nr_of_col_partner];

            col_matrix = new double **[nr_of_col_partner];

            for(uint i = 0; i < nr_of_col_partner; i++)
            {
                collision_temp[i] = 0;
                col_upper[i] = 0;
                col_lower[i] = 0;

                nr_of_col_transition[i] = 0;
                orientation_H2[i] = 0;
                nr_of_col_temp[i] = 0;
                col_matrix[i] = 0;
            }
        }
        else if(cmd_counter == 6 + nr_of_energy_level + nr_of_transitions)
        {
            if(values[0] < 1)
            {
                cout << ERROR_LINE << "Line " << line_counter
                     << " wrong orientation of H2 collision partner (gas species file)!" << endl;
                return false;
            }
            orientation_H2[i_col_partner] = int(values[0]);
        }
        else if(cmd_counter == 7 + nr_of_energy_level + nr_of_transitions + row_offset)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }

            nr_of_col_transition[i_col_partner] = int(values[0]);
            col_upper[i_col_partner] = new uint[int(values[0])];
            col_lower[i_col_partner] = new uint[int(values[0])];
            col_matrix[i_col_partner] = new double *[int(values[0])];

            for(uint i = 0; i < uint(values[0]); i++)
            {
                col_matrix[i_col_partner][i] = 0;
                col_upper[i_col_partner][i] = 0;
                col_lower[i_col_partner][i] = 0;
            }
        }
        else if(cmd_counter == 8 + nr_of_energy_level + nr_of_transitions + row_offset)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }

            nr_of_col_temp[i_col_partner] = int(values[0]);

            collision_temp[i_col_partner] = new double[int(values[0])];

            for(uint i = 0; i < uint(values[0]); i++)
                collision_temp[i_col_partner][i] = 0;
        }
        else if(cmd_counter == 9 + nr_of_energy_level + nr_of_transitions + row_offset)
        {
            if(values.size() != uint(nr_of_col_temp[i_col_partner]))
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }

            for(uint i = 0; i < uint(nr_of_col_temp[i_col_partner]); i++)
                collision_temp[i_col_partner][i] = values[i];
        }
        else if(cmd_counter < 10 + nr_of_energy_level + nr_of_transitions +
                                  nr_of_col_transition[i_col_partner] + row_offset &&
                cmd_counter > 9 + nr_of_energy_level + row_offset)
        {
            if(values.size() != uint(nr_of_col_temp[i_col_partner] + 3))
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (gas species file)!"
                     << endl;
                return false;
            }

            i_col_transition = cmd_counter - 10 - nr_of_energy_level - nr_of_transitions - row_offset;
            col_upper[i_col_partner][i_col_transition] = uint(values[1] - 1);
            col_lower[i_col_partner][i_col_transition] = uint(values[2] - 1);

            col_matrix[i_col_partner][i_col_transition] = new double[nr_of_col_temp[i_col_partner]];

            for(uint i = 0; i < uint(nr_of_col_temp[i_col_partner]); i++)
                col_matrix[i_col_partner][i_col_transition][i] = values[3 + i] * 1e-6;
        }
        else if(i_col_partner < nr_of_col_partner)
        {
            if(values[0] < 1)
            {
                cout << ERROR_LINE << "Line " << line_counter
                     << " wrong orientation of H2 collision partner (gas species file)!" << endl;
                return false;
            }

            i_col_partner++;
            orientation_H2[i_col_partner] = int(values[0]);

            row_offset = cmd_counter - (6 + nr_of_energy_level + nr_of_transitions);
        }
    }
    reader.close();

    return true;
}

bool CGasSpecies::readZeemanParamaterFile(string _filename)
{
    // Init basic variables
    fstream reader(_filename.c_str());
    CCommandParser ps;
    string line;
    dlist values;

    // Init variables
    dlist line_strength_pi, line_strength_sigma_p, line_strength_sigma_m;
    uint nr_pi_spectral_lines = 0, nr_sigma_spectral_lines = 0;
    uint i_trans_zeeman = 0;

    // Init pointer array for the lande factor
    lande_factor = new double[nr_of_energy_level];
    for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
    {
        lande_factor[i_lvl] = 0;
    }
    
    //cout << "Landee:" << lande_factor[0] << "   " << lande_factor[1] << endl;

    if(reader.fail())
    {
        cout << ERROR_LINE << "Cannot open Zeeman splitting catalog:" << endl;
        cout << _filename << endl;
        return false;
    }

    uint line_counter = 0;
    uint cmd_counter = 0;

    while(getline(reader, line))
    {
        line_counter++;

        ps.formatLine(line);

        if(line.size() == 0)
            continue;

        if(cmd_counter != 0)
        {
            values = ps.parseValues(line);
            if(values.size() == 0)
                continue;
        }

        cmd_counter++;

        if(cmd_counter == 1)
        {
            if(line != stringID)
            {
                cout << "wrong Zeeman splitting catalog file chosen!" << endl;
                return false;
            }
        }
        else if(cmd_counter == 2)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            gas_species_radius = values[0];
        }
        else if(cmd_counter == 3)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            nr_zeeman_spectral_lines = uint(values[0]);
        }
        else if(cmd_counter == 4)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
            {
                if(i_trans == int(values[0] - 1))
                {
                    // Set current zeeman transition index
                    i_trans_zeeman = int(values[0] - 1);
                    trans_is_zeeman_split[i_trans] = true;
                    break;
                }
            }
        }
        else if(cmd_counter == 5)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            // Save the lande factor of the upper energy level
            lande_factor[getUpperEnergyLevel(i_trans_zeeman)] = values[0];
            //cout << "Landee:" << lande_factor[0] << "   " << lande_factor[1] << endl;
        }
        else if(cmd_counter == 6)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            // Save the lande factor of the lower energy level
            lande_factor[getLowerEnergyLevel(i_trans_zeeman)] = values[0];
            //cout << "Landee:" << lande_factor[0] << "   " << lande_factor[1] << endl;
        }
        else if(cmd_counter == 7)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            // Save the number of sublevel per energy level
            nr_of_sublevel[getUpperEnergyLevel(i_trans_zeeman)] = int(values[0]);
        }
        else if(cmd_counter == 8)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            // Save the number of sublevel per energy level
            nr_of_sublevel[getLowerEnergyLevel(i_trans_zeeman)] = int(values[0]);

            // Set local number of sublevel for the involved energy levels
            uint nr_of_sublevel_upper = nr_of_sublevel[getUpperEnergyLevel(i_trans_zeeman)];
            uint nr_of_sublevel_lower = nr_of_sublevel[getLowerEnergyLevel(i_trans_zeeman)];

            // Calculate the numbers of transitions possible for sigma and pi transitions
            nr_pi_spectral_lines = min(nr_of_sublevel_upper, nr_of_sublevel_lower);
            nr_sigma_spectral_lines = nr_of_sublevel_upper - 1;
            if(nr_of_sublevel_upper != nr_of_sublevel_lower)
            {
                nr_sigma_spectral_lines = min(nr_of_sublevel_upper, nr_of_sublevel_lower);
            }

            // Clear the line strenth lists
            line_strength_pi.clear();
            line_strength_sigma_p.clear();
            line_strength_sigma_m.clear();
        }
        else if(cmd_counter <= 8 + nr_pi_spectral_lines && cmd_counter > 8)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            line_strength_pi.push_back(values[0]);
        }
        else if(cmd_counter <= 8 + nr_pi_spectral_lines + nr_sigma_spectral_lines &&
                cmd_counter > 8 + nr_pi_spectral_lines)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            line_strength_sigma_p.push_back(values[0]);
        }
        else if(cmd_counter <= 8 + nr_pi_spectral_lines + 2 * nr_sigma_spectral_lines &&
                cmd_counter > 8 + nr_pi_spectral_lines + nr_sigma_spectral_lines)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            line_strength_sigma_m.push_back(values[0]);
        }
        else if(cmd_counter == 9 + nr_pi_spectral_lines + 2 * nr_sigma_spectral_lines)
        {
            if(values.size() != 1)
            {
                cout << ERROR_LINE << "Line " << line_counter << " wrong amount of numbers (Zeeman file)!" << endl;
                return false;
            }
            for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
            {
                if(i_trans == int(values[0] - 1))
                {
                    // Set current zeeman transition index
                    i_trans_zeeman = int(values[0] - 1);
                    trans_is_zeeman_split[i_trans] = true;
                    break;
                }
            }

            cmd_counter = 4;
        }

        // After each Zeeman transition, adjust einstein coefficients
        if(cmd_counter == 8 + nr_pi_spectral_lines + 2 * nr_sigma_spectral_lines)
        {
            // Init indices
            uint i_pi = 0, i_sigma_p = 0, i_sigma_m = 0;

            // Init variables
            double line_strength;

            // Get maximum number of transitions between sublevel
            uint nr_sublevel_trans = getNrOfTransBetweenSublevels(i_trans_zeeman);

            // Get indices to involved energy level
            uint i_lvl_u = getUpperEnergyLevel(i_trans_zeeman);
            uint i_lvl_l = getLowerEnergyLevel(i_trans_zeeman);

            // Save einstein coefficients of major level and extend pointer array for
            // the Zeeman transition
            // Take also into account that the sublevels are treated separately
            double tmp_einst_A = getEinsteinA(i_trans_zeeman);
            delete[] trans_einstA[i_trans_zeeman];
            trans_einstA[i_trans_zeeman] = new double[nr_sublevel_trans + 1];
            

            double tmp_einst_Bul = getEinsteinBul(i_trans_zeeman);
            delete[] trans_einstB_ul[i_trans_zeeman];
            trans_einstB_ul[i_trans_zeeman] = new double[nr_sublevel_trans + 1];
            

            double tmp_einst_Blu = getEinsteinBlu(i_trans_zeeman);
            delete[] trans_einstB_lu[i_trans_zeeman];
            trans_einstB_lu[i_trans_zeeman] = new double[nr_sublevel_trans + 1];
            
            for(uint ie=0;ie<nr_sublevel_trans + 1;ie++)
            {
                trans_einstA[i_trans_zeeman][ie] = 0;
                trans_einstB_ul[i_trans_zeeman][ie] = 0;
                trans_einstB_lu[i_trans_zeeman][ie] = 0;
            }
            
            trans_einstA[i_trans_zeeman][0] = tmp_einst_A;
            trans_einstB_ul[i_trans_zeeman][0] = tmp_einst_Bul;
            trans_einstB_lu[i_trans_zeeman][0] = tmp_einst_Blu;
            
            //cout << CLR_LINE;
            //cout << "Here A\n" << trans_einstB_lu[i_trans_zeeman][0] << "  " << i_trans_zeeman <<  "  " << nr_sublevel_trans + 1 << endl;

            // Calculate the contribution of each allowed transition between Zeeman sublevels
            for(uint i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel[i_lvl_u]; i_sublvl_u++)
            {
                // Calculate the quantum number of the upper energy level
                float sublvl_u = -getMaxM(i_lvl_u) + i_sublvl_u;

                for(uint i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel[i_lvl_l]; i_sublvl_l++)
                {
                    // Calculate the quantum number of the lower energy level
                    float sublvl_l = -getMaxM(i_lvl_l) + i_sublvl_l;
                    uint i_sublvl = getSublevelIndex(i_trans_zeeman, i_sublvl_u, i_sublvl_l);


                    switch(int(sublvl_l - sublvl_u))
                    {
                        // Factor 2/3 or 1/3 comes from normalization in the Larsson paper
                        // See Deguchi & Watson 1984 as well!
                        case TRANS_SIGMA_P:
                            // Get the relative line strength from Zeeman file
                            line_strength = line_strength_sigma_p[i_sigma_p] * (2.0 / 3.0);

                            // Increase the sigma_+ counter to circle through the line strengths
                            i_sigma_p++;
                            break;
                        case TRANS_PI:
                            // Get the relative line strength from Zeeman file
                            line_strength = line_strength_pi[i_pi] * (1.0 / 3.0);

                            // Increase the pi counter to circle through the line strengths
                            i_pi++;
                            break;
                        case TRANS_SIGMA_M:
                            // Get the relative line strength from Zeeman file
                            line_strength = line_strength_sigma_m[i_sigma_m] * (2.0 / 3.0);

                            // Increase the sigma_- counter to circle through the line strengths
                            i_sigma_m++;
                            break;
                        default:
                            // Forbidden line
                            line_strength = 0;
                            break;
                    }

                    // Set the einstein coefficients for the sublevels
                    trans_einstA[i_trans_zeeman][i_sublvl + 1] =
                        tmp_einst_A * line_strength * nr_of_sublevel[i_lvl_u];
                    trans_einstB_ul[i_trans_zeeman][i_sublvl + 1] =
                        tmp_einst_Bul * line_strength * nr_of_sublevel[i_lvl_u];
                    trans_einstB_lu[i_trans_zeeman][i_sublvl + 1] = tmp_einst_Blu * line_strength * nr_of_sublevel[i_lvl_l];
                }
            }
        }
    }
    reader.close();

    for(uint i_line = 0; i_line < nr_of_spectral_lines; i_line++)
    {
        uint i_trans = getTransitionFromSpectralLine(i_line);
        
        if(getLandeUpper(i_trans) == 0) // || getLandeLower(i_trans) == 0
        {
            cout << SEP_LINE;
            cout << ERROR_LINE << "For transition number " << uint(i_trans + 1)
                 << " exists no Zeeman splitting data" << endl;
            return false;
        }
        else
        {
            trans_is_zeeman_split[i_trans] = true;
        }
    }

    return true;
}

void CGasSpecies::calcLineBroadening(CGridBasic * grid)
{
    long max_cells = grid->getMaxDataCells();

    cout << CLR_LINE;
    cout << "-> Calculating line broadening for each grid cell ...     \r" << flush;

    #pragma omp parallel for schedule(dynamic)
    for(long i_cell = 0; i_cell < long(max_cells); i_cell++)
    {
        cell_basic * cell = grid->getCellFromIndex(i_cell);

        // Get necessary quantities from the current cell
        double temp_gas = grid->getGasTemperature(*cell);
        double dens_gas = grid->getGasNumberDensity(*cell);
        double dens_species = getNumberDensity(grid, *cell);
        double turbulent_velocity = grid->getTurbulentVelocity(cell);

        // Set gauss_a for each transition only once
        grid->setGaussA(cell, getGaussA(temp_gas, turbulent_velocity));

        uint i_zeeman = 0;
        for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
        {
            if(isTransZeemanSplit(i_trans))
            {
                // Get transition frequency
                double trans_frequency = getTransitionFrequency(i_trans);

                // Init line broadening structure and fill it
                LineBroadening line_broadening;
                line_broadening.gauss_a = getGaussA(temp_gas, turbulent_velocity);
                double doppler_width = trans_frequency / (con_c * line_broadening.gauss_a);
                double Gamma = getGamma(i_trans, dens_gas, dens_species, temp_gas, turbulent_velocity);
                line_broadening.voigt_a = Gamma / (4 * PI * doppler_width);

                // Add broadening to grid cell information
                grid->setLineBroadening(cell, i_zeeman, line_broadening);
                i_zeeman++;
            }
        }
    }

    cout << CLR_LINE;
}

void CGasSpecies::applyRadiationFieldFactor(uint i_trans,
                                            double sin_theta,
                                            double cos_theta,
                                            double energy,
                                            double * J_nu) const
{
    if(!isTransZeemanSplit(i_trans))
    {
        // Update radiation field
        uint i_tmp_trans = getUniqueTransitionIndex(i_trans);
        J_nu[i_tmp_trans] += energy;
        return;
    }

    // Get indices to involved energy level
    uint i_lvl_u = getUpperEnergyLevel(i_trans);
    uint i_lvl_l = getLowerEnergyLevel(i_trans);

    // Calculate the contribution of each allowed transition between Zeeman sublevels
    for(uint i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel[i_lvl_u]; i_sublvl_u++)
    {
        // Calculate the quantum number of the upper energy level
        float sublvl_u = -getMaxM(i_lvl_u) + i_sublvl_u;

        for(uint i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel[i_lvl_l]; i_sublvl_l++)
        {
            // Calculate the quantum number of the lower energy level
            float sublvl_l = -getMaxM(i_lvl_l) + i_sublvl_l;

            // Skip forbidden transitions
            if(abs(sublvl_l - sublvl_u) > 1)
                continue;

            uint i_trans_unique = getUniqueTransitionIndex(i_trans, i_sublvl_u, i_sublvl_l);

            switch(int(sublvl_l - sublvl_u))
            {
                case TRANS_PI:
                    // Update radiation field
                    J_nu[i_trans_unique] += (3.0 / 2.0) * (sin_theta * sin_theta) * energy;
                    break;
                case TRANS_SIGMA_P:
                case TRANS_SIGMA_M:
                    // Update radiation field
                    J_nu[i_trans_unique] += (3.0 / 4.0) * (1 + cos_theta * cos_theta) * energy;
                    break;
            }
        }
    }
}

string CGasSpecies::getGasSpeciesName() const
{
    return stringID;
}

double CGasSpecies::getLineStrength(uint i_trans, uint i_sublvl_u, uint i_sublvl_l) const
{
    if(getEinsteinA(i_trans) > 0)
        return getEinsteinA(i_trans, i_sublvl_u, i_sublvl_l) / getEinsteinA(i_trans);
    return 0;
}

double CGasSpecies::getAbundance() const
{
    return abundance;
}

double CGasSpecies::getKeplerStarMass() const
{
    return kepler_star_mass;
}

int CGasSpecies::getTransitionFromSpectralLine(uint i_line) const
{
    return spectral_lines[i_line];
}

double CGasSpecies::getTransitionFrequency(uint i_trans) const
{
    return trans_freq[i_trans];
}

double CGasSpecies::getSpectralLineFrequency(uint i_line) const
{
    uint i_trans = getTransitionFromSpectralLine(i_line);
    return getTransitionFrequency(i_trans);
}

uint CGasSpecies::getNrOfSpectralLines() const
{
    return nr_of_spectral_lines;
}

double CGasSpecies::getEinsteinA(uint i_trans) const
{
    return trans_einstA[i_trans][0];
}

double CGasSpecies::getEinsteinBul(uint i_trans) const
{
    return trans_einstB_ul[i_trans][0];
}

double CGasSpecies::getEinsteinBlu(uint i_trans) const
{
    return trans_einstB_lu[i_trans][0];
}

uint CGasSpecies::getSublevelIndex(uint i_trans, uint i_sublvl_u, uint i_sublvl_l) const
{
    return i_sublvl_u * getNrOfSublevelLower(i_trans) + i_sublvl_l;
}

double CGasSpecies::getEinsteinA(uint i_trans, uint i_sublvl_u, uint i_sublvl_l) const
{
    // If not Zeeman split, use total value
    if(!isTransZeemanSplit(i_trans))
        return getEinsteinA(i_trans) / getNrOfSublevelLower(i_trans);

    uint i_sublvl = getSublevelIndex(i_trans, i_sublvl_u, i_sublvl_l);
    return trans_einstA[i_trans][i_sublvl + 1];
}

double CGasSpecies::getEinsteinBul(uint i_trans, uint i_sublvl_u, uint i_sublvl_l) const
{
    // If not Zeeman split, use total value
    if(!isTransZeemanSplit(i_trans))
        return getEinsteinBul(i_trans) / getNrOfSublevelLower(i_trans);

    uint i_sublvl = getSublevelIndex(i_trans, i_sublvl_u, i_sublvl_l);
    return trans_einstB_ul[i_trans][i_sublvl + 1];
}

double CGasSpecies::getEinsteinBlu(uint i_trans, uint i_sublvl_u, uint i_sublvl_l) const
{
    // If not Zeeman split, use total value
    if(!isTransZeemanSplit(i_trans))
        return getEinsteinBlu(i_trans) / getNrOfSublevelUpper(i_trans);

    uint i_sublvl = getSublevelIndex(i_trans, i_sublvl_u, i_sublvl_l);
    return trans_einstB_lu[i_trans][i_sublvl + 1];
}

double CGasSpecies::getJLevel(uint i_lvl) const
{
    return quantum_numbers[i_lvl];
}

double CGasSpecies::getEnergyOfLevel(uint i_lvl) const
{
    return energy_level[i_lvl];
}

uint CGasSpecies::getUpperCollisionLevel(uint m, uint n) const
{
    return col_upper[m][n];
}

uint CGasSpecies::getLowerCollisionLevel(uint m, uint n) const
{
    return col_lower[m][n];
}

double CGasSpecies::getCollisionTemp(uint m, uint n) const
{
    return collision_temp[m][n];
}

double CGasSpecies::getCollisionMatrix(uint m, uint n, uint k) const
{
    return col_matrix[m][n][k];
}

double CGasSpecies::getLandeUpper(uint i_trans) const
{
    //cout << "Landeu up:" << lande_factor[upper_level[i_trans]] << endl;
    return lande_factor[upper_level[i_trans]];
}

double CGasSpecies::getLandeLower(uint i_trans) const
{
    //cout << "Landee low:" << lande_factor[lower_level[i_trans]] << endl;
    return lande_factor[lower_level[i_trans]];
}

double CGasSpecies::CGasSpecies::getLande(uint i_lvl) const
{
    return lande_factor[i_lvl];
}

double CGasSpecies::getCollisionRadius() const
{
    return gas_species_radius;
}

double CGasSpecies::getMolecularWeight() const
{
    return molecular_weight;
}

double CGasSpecies::getNumberDensity(CGridBasic * grid, const photon_package & pp) const
{
    return getNumberDensity(grid, *pp.getPositionCell());
}

double CGasSpecies::getNumberDensity(CGridBasic * grid, const cell_basic & cell) const
{
    double dens_species = 0;
    // If the abundance is negative, get its value from the grid
    if(abundance <= 0)
    {
        uint fr_id = uint(-abundance - 1);
        dens_species = grid->getCellAbundance(cell, fr_id);
    }
    else
        dens_species = abundance;
    dens_species *= grid->getGasNumberDensity(cell);
    return dens_species;
}

double CGasSpecies::getMassDensity(CGridBasic * grid, const photon_package & pp) const
{
    return getMassDensity(grid, *pp.getPositionCell());
}

double CGasSpecies::getMassDensity(CGridBasic * grid, const cell_basic & cell) const
{
    double dens_species = 0;
    // If the abundance is negative, get its value from the grid
    if(abundance <= 0)
    {
        uint fr_id = uint(-abundance - 1);
        dens_species = grid->getCellAbundance(cell, fr_id);
    }
    else
        dens_species = abundance;
    dens_species *= grid->getGasNumberDensity(cell);
    dens_species *= molecular_weight * m_H;
    return dens_species;
}

Vector3D CGasSpecies::getCellVelocity(CGridBasic * grid, const cell_basic & cell, const Vector3D & tmp_pos) const
{
    Vector3D cell_velocity;

    // Get the velocity in the photon
    // direction of the current position
    if(kepler_star_mass > 0)
    {
        // Get velocity from Kepler rotation
        cell_velocity = CMathFunctions::calcKeplerianVelocity(tmp_pos, kepler_star_mass);
    }
    else if(grid->hasVelocityField())
    {
        // Get velocity from grid cell
        cell_velocity = grid->getVelocityField(cell);
    }
    return cell_velocity;
}

double CGasSpecies::getProjCellVelocityInterp(const Vector3D & tmp_pos,
                                    const Vector3D & dir_map_xyz,
                                    const VelFieldInterp & vel_field_interp)
{
    double cell_velocity = 0;

    // Get the velocity in the photon direction of the current position
    if(kepler_star_mass > 0)
    {
        // Get velocity from Kepler rotation
        cell_velocity = CMathFunctions::calcKeplerianVelocity(tmp_pos, kepler_star_mass) * dir_map_xyz;
    }
    else if(vel_field_interp.vel_field.size() > 0 && !vel_field_interp.zero_vel_field)
    {
        // Get velocity from grid cell with interpolation
        Vector3D rel_pos = tmp_pos - vel_field_interp.start_pos;
        cell_velocity = vel_field_interp.vel_field.getValue(rel_pos.length());
    }
    return cell_velocity;
}

void CGasSpecies::initReferenceLists()
{
    // Init first dimension of 2D array
    level_to_index = new uint *[nr_of_energy_level];

    uint i_lvl_unique = 0;
    for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
    {
        // Init second dimension of 2D array
        level_to_index[i_lvl] = new uint[nr_of_sublevel[i_lvl]];

        for(uint i_sublvl = 0; i_sublvl < nr_of_sublevel[i_lvl]; i_sublvl++)
        {
            level_to_index[i_lvl][i_sublvl] = i_lvl_unique;
            i_lvl_unique++;
        }
    }

    // Init first dimension of 2D array
    trans_to_index = new uint **[nr_of_transitions];

    uint i_trans_unique = 0;
    for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
    {
        // Get all indices and number of sublevels
        uint i_lvl_u = getUpperEnergyLevel(i_trans);
        uint i_lvl_l = getLowerEnergyLevel(i_trans);
        uint nr_of_sublevel_u = nr_of_sublevel[i_lvl_u];
        uint nr_of_sublevel_l = nr_of_sublevel[i_lvl_l];

        // Init second dimension of 2D array
        trans_to_index[i_trans] = new uint *[nr_of_sublevel_u];

        for(uint i_sublvl_u = 0; i_sublvl_u < nr_of_sublevel_u; i_sublvl_u++)
        {
            // Init third dimension of 2D array
            trans_to_index[i_trans][i_sublvl_u] = new uint[nr_of_sublevel_l];

            for(uint i_sublvl_l = 0; i_sublvl_l < nr_of_sublevel_l; i_sublvl_l++)
            {
                trans_to_index[i_trans][i_sublvl_u][i_sublvl_l] = i_trans_unique;
                i_trans_unique++;
            }
        }
    }
}

uint CGasSpecies::getZeemanSplitIndex(uint i_trans)
{
    if(trans_is_zeeman_split != 0)
    {
        uint i_zeeman = 0;
        for(uint i = 0; i < i_trans; i++)
        {
            if(trans_is_zeeman_split[i])
                i_zeeman++;
        }
        if(trans_is_zeeman_split[i_trans])
            return i_zeeman;
        else
            return MAX_UINT;
    }
    return MAX_UINT;
}

bool CGasSpecies::isTransZeemanSplit(uint i_trans) const
{
    if(trans_is_zeeman_split != 0)
        return trans_is_zeeman_split[i_trans];
    return false;
}

bool CGasSpecies::isLineZeemanSplit(uint i_line) const
{
    uint i_trans = getTransitionFromSpectralLine(i_line);
    return isTransZeemanSplit(i_trans);
}

bool CGasSpecies::isZeemanSplit() const
{
    for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
    {
        if(isTransZeemanSplit(i_trans))
            return true;
    }
    return false;
}

uint CGasSpecies::getUniqueLevelIndex(uint i_lvl, uint i_sublvl) const
{
    return level_to_index[i_lvl][i_sublvl];
}

uint CGasSpecies::getUniqueTransitionIndex(uint i_trans, uint i_sublvl_u, uint i_sublvl_l) const
{
    return trans_to_index[i_trans][i_sublvl_u][i_sublvl_l];
}

uilist CGasSpecies::getUniqueTransitions() const
{
    return unique_spectral_lines;
}

uint CGasSpecies::getUniqueTransitions(uint i) const
{
    return unique_spectral_lines[i];
}

uint CGasSpecies::getUpperEnergyLevel(uint i_trans) const
{
    return upper_level[i_trans];
}

uint CGasSpecies::getLowerEnergyLevel(uint i_trans) const
{
    return lower_level[i_trans];
}

uint CGasSpecies::getNrOfEnergyLevels() const
{
    return nr_of_energy_level;
}

uint CGasSpecies::getNrOfTotalEnergyLevels() const
/*
Including Zeeman sublevels
*/
{
    uint res = 0;
    for(uint i_lvl = 0; i_lvl < nr_of_energy_level; i_lvl++)
    {
        // Includes the 1 for no Zeeman split energy levels
        res += nr_of_sublevel[i_lvl];
    }
    return res;
}

uint CGasSpecies::getNrOfTotalTransitions() const
/*
Including Zeeman sublevels
*/
{
    uint res = 0;
    for(uint i_trans = 0; i_trans < nr_of_transitions; i_trans++)
    {
        uint i_lvl_u = getUpperEnergyLevel(i_trans);
        uint i_lvl_l = getLowerEnergyLevel(i_trans);

        // Includes the 1 for no Zeeman split energy levels
        res += nr_of_sublevel[i_lvl_u] * nr_of_sublevel[i_lvl_l];
    }
    return res;
}

uint CGasSpecies::getNrOfTransitions() const
{
    return nr_of_transitions;
}

uint CGasSpecies::getNrOfCollisionPartner() const
{
    return nr_of_col_partner;
}

uint CGasSpecies::getNrCollisionTransitions(uint i_col_partner) const
{
    return nr_of_col_transition[i_col_partner];
}

uint CGasSpecies::getNrCollisionTemps(uint i_col_partner) const
{
    return nr_of_col_temp[i_col_partner];
}

uint CGasSpecies::getLevelPopType() const
{
    return lvl_pop_type;
}

uint CGasSpecies::getOrientation_H2(uint i_col_partner) const
{
    return orientation_H2[i_col_partner];
}

int CGasSpecies::getNrOfSublevelUpper(uint i_trans) const
{
    return nr_of_sublevel[upper_level[i_trans]];
}

int CGasSpecies::getNrOfSublevelLower(uint i_trans) const
{
    return nr_of_sublevel[lower_level[i_trans]];
}

int CGasSpecies::getNrOfSublevel(uint i_lvl) const
{
    return nr_of_sublevel[i_lvl];
}

int CGasSpecies::getNrOfTransBetweenSublevels(uint i_trans) const
{
    return getNrOfSublevelUpper(i_trans) * getNrOfSublevelLower(i_trans);
}

float CGasSpecies::getMaxMUpper(uint i_trans) const
{
    return float((getNrOfSublevelUpper(i_trans) - 1) / 2.0);
}

float CGasSpecies::CGasSpecies::getMaxMLower(uint i_trans) const
{
    return float((getNrOfSublevelLower(i_trans) - 1) / 2.0);
}

float CGasSpecies::getMaxM(uint i_lvl) const
{
    return float((nr_of_sublevel[i_lvl] - 1) / 2.0);
}

void CGasSpecies::setKeplerStarMass(double val)
{
    kepler_star_mass = val;
}

void CGasSpecies::setLevelPopType(uint type)
{
    lvl_pop_type = type;
}

void CGasSpecies::setMolecularWeight(double val)
{
    molecular_weight = val;
}

void CGasSpecies::setAbundance(double val)
{
    abundance = val;
}

void CGasSpecies::setNrOfSpectralLines(uint val)
{
    nr_of_spectral_lines = val;
}

void CGasSpecies::setSpectralLines(int * lines)
{
    spectral_lines = lines;
}

double CGasSpecies::getGamma(uint i_trans, double dens_gas, double dens_species, double temp_gas, double v_turb)
{
    double gamma = getEinsteinA(i_trans);

    // "http://chem.libretexts.org/Core/Physical_and_Theoretical_Chemistry/
    //  ->
    //  Kinetics/Modeling_Reaction_Kinetics/Collision_Theory/Collisional_Cross_Section"
    // "http://www.phy.ohiou.edu/~mboett/astro401_fall12/broadening.pdf
    double v_th = sqrt(2.0 * con_kB * temp_gas / (molecular_weight * 1e-3 / con_Na));
    double col_param =
        dens_gas * PI * pow(con_r_bohr + gas_species_radius, 2) * sqrt(pow(v_th, 2) + pow(v_turb, 2));

    return gamma + 2 * col_param;
}

double CGasSpecies::getGaussA(double temp_gas, double v_turb)
{
    double v_th = sqrt(2.0 * con_kB * temp_gas / (molecular_weight * 1e-3 / con_Na));
    double gauss_a = 1.0 / sqrt(pow(v_th, 2) + pow(v_turb, 2));
    return gauss_a;
}

void CGasSpecies::getGaussLineMatrix(CGridBasic * grid,
                        const cell_basic & cell,
                        double velocity,
                        Matrix2D * line_absorption_matrix) const
{

    // Calculate gaussian shape
    double line_amplitude = getGaussLineShape(grid, cell, velocity);

    // Only diagonal without polarization rotation matrix elements
    for(uint i = 0; i < 4; i++)
        line_absorption_matrix->setValue(i, i, line_amplitude);
}

void CGasSpecies::getGaussLineMatrix(CGridBasic * grid,
                        const photon_package & pp,
                        double velocity,
                        Matrix2D * line_absorption_matrix) const
{
    getGaussLineMatrix(grid, *pp.getPositionCell(), velocity, line_absorption_matrix);
}

double CGasSpecies::getGaussLineShape(CGridBasic * grid, const cell_basic & cell, double velocity) const
{
    double gauss_a = grid->getGaussA(cell);
    return exp(-(pow(velocity, 2) * pow(gauss_a, 2))) / PIsq;
}

double CGasSpecies::getGaussLineShape(CGridBasic * grid, const photon_package & pp, double velocity) const
{
    return getGaussLineShape(grid, *pp.getPositionCell(), velocity);
}

void CGasSpecies::calcEmissivityFromLvlPop(uint i_trans,
                                uint i_sublvl_u,
                                uint i_sublvl_l,
                                double dens_species,
                                double gauss_a,
                                double * tmp_lvl_pop,
                                double * j,
                                double * alpha)
{
    uint i_lvl_l = getLowerEnergyLevel(i_trans);
    uint i_lvl_u = getUpperEnergyLevel(i_trans);

    double lvl_pop_l = tmp_lvl_pop[getUniqueLevelIndex(i_lvl_l, i_sublvl_l)];
    double lvl_pop_u = tmp_lvl_pop[getUniqueLevelIndex(i_lvl_u, i_sublvl_u)];

    *j = dens_species * lvl_pop_u * getEinsteinA(i_trans, i_sublvl_u, i_sublvl_l) * gauss_a * con_eps /
            PIsq;
    *alpha = dens_species *
                (lvl_pop_l * getEinsteinBlu(i_trans, i_sublvl_u, i_sublvl_l) -
                lvl_pop_u * getEinsteinBul(i_trans, i_sublvl_u, i_sublvl_l)) *
                gauss_a * con_eps / PIsq;
}

double CGasSpecies::calcJFromInteractionLength(double j, double alpha, double J_ext, double L)
{
    // Init variables
    double beta, S;

    if(alpha < 1e-20)
    {
        S = 0.0;
        alpha = 0.0;
    }
    else
        S = j / alpha;

    double tau = alpha * L;

    if(tau < 1e-6)
        beta = 1.0 - 0.5 * tau;
    else
        beta = (1.0 - exp(-tau)) / tau;

    double J_mid = (1.0 - beta) * S + beta * J_ext;
    return J_mid;
}

double CGasSpecies::calcJFromOpticalDepth(double j, double alpha, double J_ext, double tau)
{
    // Init variables
    double beta, S;

    if(alpha < 1e-20)
        S = 0.0;
    else
        S = j / alpha;

    if(tau < 1e-6)
        beta = 1.0 - 0.5 * tau;
    else
        beta = (1.0 - exp(-tau)) / tau;

    double J_mid = (1.0 - beta) * S + beta * J_ext;
    return J_mid;
}
            