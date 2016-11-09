
#include "TMeasHeader.h"
#include <string.h>  //strcpy

ClassImp(TMeasHeader)


TMeasHeader::TMeasHeader( ) {
     Nav=0;
     NV=Nx=Ny=Nz=0 ;
     At=Ax=Ay=Az=Cend=tann=Etann=iann=Fluence=0. ;
     Temp=Position=0. ;
     comment=TString("");
     vVbias = 0.0 ;
     Polarity=0;
     Setup = 1;
     Hum = Illum = Power = Gain = 0.0;	 
     Lambda =1060.0 ;    
     Freq = 200 ;	  

}

TMeasHeader::TMeasHeader( Int_t NV) {
     Nx=Ny=Nz=0 ;
     At=Ax=Ay=Az=Cend=tann=Etann=iann=Fluence=0. ;
     Temp=0. ;
     comment=TString("");
     vVbias.ResizeTo(NV) ; //http://root.cern.ch/root/roottalk/roottalk09/1231.html
     vIleak.ResizeTo(NV) ; //http://root.cern.ch/root/roottalk/roottalk09/1231.html
     vTemp.ResizeTo(NV) ;  //http://root.cern.ch/root/roottalk/roottalk09/1231.html
     Polarity=0;
     Setup = 1;
     Hum = Illum = Power = Gain = 0.0;	 
     Lambda =1060.0 ;    
     Freq = 200 ;	  
}

TMeasHeader::~TMeasHeader()
{
  //Emptiness
  vVbias.Clear() ;
  
}

