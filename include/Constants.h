#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <iostream>
#include "TString.h"
#include "TMath.h"
/** Constants definition used for diffusion.
 */
//#define ECH		1.602177e-19	// elementary charge in C
//#define kB	    1.38065e-23	    // Boltzmann constant (J/K)
//#define EPS     11.9            // EPS silicon relative permitivity
//#define EPS0    8.85e-14       // EPS0 F/cm  vacuum permitivity

//Mobility electrons	≤1400 cm2 V-1s-1
//Mobility holes	≤450 cm2 V-1s-1


	const double ECH=1.602177e-19;
	const double kB=1.38065e-23;
	const double EPS=11.9;
	//in micrometers
	const double EPS0=8.85e-18;

	//Lenght in micrometers
	const double cm=10000.;
	const double m =100*cm;
	const double um=1.;

	//Time in secs
	const double s = 1.;
	const double us= 1.e-6;
	const double ns= 1.e-9;

	const double coefficientElectron = 36*cm*cm/s;
	const double coefficientHole = 12*cm*cm/s;
	const double keV =1.;
	const double MeV=1000.;

	const double V=1;
	const double uA=1.;
	const double mA=1000.;
	const double A=1.e6;

	const double T=V*s/m/m;
	const double pi = TMath::Pi();

	#endif


