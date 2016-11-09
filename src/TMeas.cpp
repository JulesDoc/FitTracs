#include "TMeas.h"

ClassImp(TMeas)

TMeas::TMeas( ) {
     NV=Nx=Ny=Nz=0;
     At=Ax=Ay=Az=0. ;	    
     event=Ntevent=0 ;	    
     Itot=x=y=z=Vbias=0. ;	    
     Temp=999.0;
     ix=iy=iz=0 ;
     
     Setup = 1 ;
     Nt=0 ;
     volt=0 ;
     time=0 ;
     Qt=0;

     pWidth = pAmpl = 0. ;
}

TMeas::~TMeas()
{
  delete [] volt ;
  delete [] time ;
  delete [] Qt ;
}

//____________________________________________________________________________


