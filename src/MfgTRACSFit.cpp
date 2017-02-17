/*****This code is property of CERN and IFCA under GPL License. Developed by: Marcos Fernandez, Pablo de Castro, Alvaro Diez, Urban Senica and Julio Calvo.*****/

/*
  Example

    DoTRACSFit MeasurementFile TRACS.conf "Vbias==200 && Tset == 20"

*/

//#include <TApplication.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnUserParameterState.h>
#include <Minuit2/MnUserParameters.h>
#include <Minuit2/MnMachinePrecision.h>
#include <Minuit2/MnPrint.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnMinos.h>
#include <Minuit2/MnSimplex.h>
#include <Minuit2/MnStrategy.h>
#include <Minuit2/MnPlot.h>
#include <Minuit2/MinosError.h>
#include <Minuit2/FCNBase.h>
#include <Math/MinimizerOptions.h>

#include <boost/asio.hpp>

#include <TRACSFit.h>
#include <TRACSInterface.h>
#include <TString.h>
#include <stdio.h>

#include "../include/Global.h"

std::vector<TRACSInterface*> TRACSsim;
std::vector<std::thread> t;
boost::posix_time::time_duration total_timeTaken ;
TRACSFit *fit ;
using namespace ROOT::Minuit2;

int main( int argc, char *argv[]) {

    //TApplication theApp("DoTRACSFit", 0, 0);

    //Number of threads
    num_threads = atoi(argv[1]);

    //Measurement file
    TString FileMeas = TString( argv[2] ) ;

    //Configuration file
    TString FileConf = TString( argv[3] ) ;
    std::string lfnm(argv[3]) ;
    fnm = lfnm;

    //Restrictions for fits
    TString how="";
    if (argc>2) how = TString( argv[4] ) ;

    TRACSsim.resize(num_threads);
    t.resize(num_threads);
    t[0] = std::thread(call_from_thread, 0);
    t[0].join();

    TRACSsim.resize(num_threads);
    t.resize(num_threads);
    for (int i = 1; i < num_threads; ++i) {
    	t[i] = std::thread(call_from_thread, i);
    }
    for (int i = 1; i < num_threads; ++i) {
    	t[i].join();
    }

    fit = new TRACSFit( FileMeas, FileConf , how ) ;

    //Define parameters and their errors to Minuit

    vector<Double_t> parIni = TRACSsim[0]->get_NeffParam();
    Int_t nNeff = parIni.size() ;

    vector<Double_t> parErr(nNeff, 60.) ;

    //Pass parameters to Minuit
    MnUserParameters upar(parIni,parErr) ;
    for ( int i=0 ; i < nNeff ; i++ ) {
        char pname[8]; sprintf( pname , "p%d" , i);
    	upar.SetName( i , pname );
    }
         
    //Fix parameters
    upar.Fix(0) ;
    upar.Fix(1) ; upar.Fix(2) ;
    //upar.Fix(3) ;
    upar.Fix(4) ; upar.Fix(5); upar.Fix(6) ; upar.Fix(7);

    std::cout << "=============================================" << std::endl;
    std::cout<<"Initial parameters: "<<upar<<std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "tolerance= " << TRACSsim[0]->GetTolerance()    << std::endl;
    std::cout << "chiFinal= " << TRACSsim[0]->GetchiFinal()      << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "=============================================" << std::endl;


    //Do the minimization

    ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);    
    ROOT::Math::MinimizerOptions::SetDefaultTolerance(TRACSsim[0]->GetTolerance());
    MnMigrad mn( *fit , upar , MnStrategy(0)) ;
    FunctionMinimum min = mn() ;

    //Status report
    std::cout << "Total time: " << total_timeTaken.total_seconds() << std::endl ;
    std::cout << "MINIMIZATION OUTCOME: " <<  min  << std::endl ;
 
    //Release parameter 3, fix 0, minimize again
    upar.SetValue(3, min.UserState().Value(3) ) ; upar.SetError(3, min.UserState().Error(3)) ; upar.Fix(3) ; 
    upar.Release(0) ; upar.SetError(0,60.);

    std::cout << "=============================================" << std::endl;
    std::cout<<"Second Minimization: "<<upar<<std::endl;
    std::cout << "=============================================" << std::endl;
    MnMigrad mnr( *fit , upar , MnStrategy(0)) ;
    min = mnr() ;

    //Status report
    if (min.IsValid()) std::cout << "Fit success"         << std::endl ;
    else               std::cout << "Fit failed"   << std::endl ;
    std::cout << "Total time: " << total_timeTaken.total_seconds() << std::endl ;
    std::cout << "MINIMIZATION OUTCOME: " <<  min  << std::endl ;

    //Release parameter 3, fix 0, minimize again
    for ( int i=0 ; i < 4 ; i++ ) { upar.Release(i) ; upar.SetError(i,60.); }

    std::cout << "=============================================" << std::endl;
    std::cout<<"All par free: "<<upar<<std::endl;
    std::cout << "=============================================" << std::endl;
    MnMigrad mn3( *fit , upar , MnStrategy(0)) ;
    min = mn3() ;

    //Status report
    if (min.IsValid()) std::cout << "Fit success"         << std::endl ;
    else               std::cout << "Fit failed"   << std::endl ;
    std::cout << "Total time: " << total_timeTaken.total_seconds() << std::endl ;
    std::cout << "MINIMIZATION OUTCOME: " <<  min  << std::endl ;

   //Get the fitting parameters
    for (uint i=0; i<parIni.size();i++) {
      parIni[i]=min.UserState().Value(i);
      parErr[i]=min.UserState().Error(i);
    }

    //Calculate TCT pulses with the fit output parameters
    for (int i = 0; i < num_threads; ++i) {
	    t[i] = std::thread(call_from_thread_NeffPar, i, parIni);
    }

    for (int i = 0; i < num_threads; ++i) {
	    t[i].join();
    }

    //Dump tree to disk
    TFile fout("output.root","RECREATE") ;
    TTree *tout = new TTree("edge","Fitting results");

    TMeas *emo = new TMeas( );
    emo->Nt   = TRACSsim[0]->GetnSteps() ;
    emo->volt = new Double_t [emo->Nt] ;
    emo->time = new Double_t [emo->Nt] ;
    emo->Qt   = new Double_t [emo->Nt] ;

    // Create branches
    tout->Branch("raw", &emo,32000,0);

    //Read RAW file
    TRACSsim[0]->DumpToTree( emo , tout ) ;

    //TRACSsim[0]->GetTree( tsim );
    fout.Write();
    delete tout ;
    fout.Close();
    delete emo ;

    //Clean
    for (uint i = 0; i < TRACSsim.size(); i++)	{
       delete TRACSsim[i];
    }

    delete fit;
    std::quick_exit(1);
}

//_____________________________________________________________________

Double_t TRACSFit::operator() ( const std::vector<Double_t>& par  ) const {

	static int icalls ;
	boost::posix_time::ptime start = boost::posix_time::second_clock::local_time();

	for (int i = 0; i < num_threads; ++i) {
		t[i] = std::thread(call_from_thread_NeffPar, i, par);
	}

	for (int i = 0; i < num_threads; ++i) {
		t[i].join();
	}

    Double_t chi2 = fit->LeastSquares( ) ;
    boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration timeTaken = end - start;
    total_timeTaken += timeTaken;

	std::cout << "-------------------------------------------------------------------------------------> " << std::endl;
	std::cout << "----------------------------> Time taken for chi2 calculation (milliseconds): " << timeTaken.total_milliseconds() << std::endl;
	std::cout << "-------------------------------------------------------------------------------------> " << std::endl;
	std::cout << "----------------------------> icalls="<<icalls<<" chi2=" << chi2 << "\t" ;
	for (uint ipar=0 ; ipar<par.size() ; ipar++) std::cout << "p["<<ipar<<"]="<<par[ipar]<<"\t" ; std::cout << std::endl;
	std::cout << "-------------------------------------------------------------------------------------> " << std::endl;
	icalls++;

    for (uint i = 0; i < TRACSsim.size(); i++)	{
    	    delete TRACSsim[i];
    }

	return chi2 ;


}