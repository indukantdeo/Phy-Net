/************************************************************************
 * MechSys - Open Library for Mechanical Systems                        *
 * Copyright (C) 2014 Sergio Galindo                                    *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * any later version.                                                   *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program. If not, see <http://www.gnu.org/licenses/>  *
 ************************************************************************/

/////////////////////// Test 02 The dielectric medium

// MechSys
#include <mechsys/emlbm2/Domain.h>
#include <mechsys/util/fatal.h>
#include <mechsys/util/util.h>

using std::cout;
using std::max;
using std::endl;

struct UserData
{
    int nx;
    int ny;
    int nz;
};

void Setup (EMLBM::Domain & dom, void * UD)
{
    UserData & dat = (*static_cast<UserData *>(UD));
#ifdef USE_OMP
    #pragma omp parallel for schedule (static) num_threads(dom.Nproc)
#endif
    for (int i=0;i<dat.nx;i++)
    for (int j=0;j<dat.ny;j++)
    {
        Cell * c0  = dom.Lat.GetCell(iVec3_t(i,j,       0));
        Cell * c0n = dom.Lat.GetCell(iVec3_t(i,j,       1));
        Cell * c1  = dom.Lat.GetCell(iVec3_t(i,j,dat.nz-1));
        Cell * c1n = dom.Lat.GetCell(iVec3_t(i,j,dat.nz-2));

        //c0->Initialize(0.0,OrthoSys::O,c0n->E,c0n->B);
        //c1->Initialize(0.0,OrthoSys::O,c1n->E,c1n->B);
        c0->Initialize(0.0,OrthoSys::O,OrthoSys::O,OrthoSys::O);
        c1->Initialize(0.0,OrthoSys::O,OrthoSys::O,OrthoSys::O);
    }
}

int main(int argc, char **argv) try
{
    size_t nproc = 1; 
    if (argc==2) nproc=atoi(argv[1]);
    int nx = 256;
    int ny = 1;
    int nz = 256;
    EMLBM::Domain Dom(iVec3_t(nx,ny,nz), 1.0, 1.0);
    UserData dat;
    Dom.UserData = &dat;
    Dom.Step = 1;
    dat.nx = nx;
    dat.ny = ny;
    dat.nz = nz;
    double E0 = 0.001;
    //double B0 = sqrt(2.0)*E0;
    double B0 = sqrt(2.0)*E0;
    double alpha = 0.01;
   //double beta  = 0.0005;
    double z0 = 80;
    double x0 = 100;

    for (int i=0;i<nx;i++)
    for (int j=0;j<ny;j++)
    for (int k=0;k<nz;k++)
    {
        //Vec3_t E(E0*exp(-alpha*((k-z0)*(k-z0)+(i-x0)*(i-x0))),0.0,0.0);
        //Vec3_t B(0.0,B0*exp(-alpha*((k-z0)*(k-z0)+(i-x0)*(i-x0))),0.0);
        //Vec3_t E(E0*exp(-alpha*((k-z0)*(k-z0))-beta*((i-x0)*(i-x0))),0.0,0.0);
        //Vec3_t B(0.0,B0*exp(-alpha*((k-z0)*(k-z0))-beta*((i-x0)*(i-x0))),0.0);
        Vec3_t E(E0*exp(-alpha*(k-z0)*(k-z0)),0.0,0.0);
        Vec3_t B(0.0,B0*exp(-alpha*(k-z0)*(k-z0)),0.0);
        Dom.Lat.GetCell(iVec3_t(i,j,k))->Initialize(0.0,OrthoSys::O,E,B);
        //Dom.Lat.GetCell(iVec3_t(i,j,k))->Eps = 2.0*tanh(k-nz/2)+3.0;
        //if (k>nz/2) Dom.Lat.GetCell(iVec3_t(i,j,k))->Eps = 5.0;
    }


    // number of objects between 10 and 25
    //size_t num_objects = (rand() % 10) + 10;
    size_t num_objects = 2*(nx/128)*(nx/128);

    // set objects
    size_t h = 0;
    size_t trys = 0;
    while (h<num_objects && trys<1000)
    {
        trys++;
        int object_type = (rand() % 2);
        if (object_type == 0) // oval
        {
	    // set inner obstacle
            int radius_x = (rand() % 60) + 10;
            int radius_y = (rand() % 60) + 10;
            int max_radius = radius_x; 
            if (radius_y > radius_x) { max_radius = radius_y; }
	    double obsX   = (rand() % (nx-(3*max_radius))) + (1.5*max_radius) ;   // x position
	    double obsY   = (rand() % (ny-(3*max_radius))) + (1.5*max_radius) ;   // y position
            int alpha = (rand() % 90);
            h++;
            for (size_t i=0;i<nx;i++)
            {
                for (size_t j=0;j<nz;j++)
                {
                    if ((pow(cos(alpha)*(i-obsX) + sin(alpha)*(j-obsY),2.0))/(radius_x*radius_x)+(pow(sin(alpha)*(i-obsX) - cos(alpha)*(j-obsY),2.0))/(radius_y*radius_y)<1.0)
                    {
                        Dom.Lat.GetCell(iVec3_t(i,0,j))->Eps = max(Dom.Lat.GetCell(iVec3_t(i,0,j))->Eps, (5.0*(1.- ((pow(cos(alpha)*(i-obsX) + sin(alpha)*(j-obsY),2.0))/(radius_x*radius_x)+(pow(sin(alpha)*(i-obsX) - cos(alpha)*(j-obsY),2.0))/(radius_y*radius_y))) + 1.0));
                    }
                }
            }

        }


    }



    //Dom.WriteXDMF("test");
    //Dom.Solve(600.0,6.0,&Setup,NULL,"temlbm02",true,nproc);
    Dom.Solve(600.0,6.0,NULL,NULL,"temlbm02",true,nproc);

    return 0;
}
MECHSYS_CATCH

