#ifndef TRACSINTERFACE_H
#define TRACSINTERFACE_H

#include <stdlib.h>
#include <iostream>
#include <ctime>

#include <iterator>
#include <limits>  // std::numeric_limits
#include <cmath>
#include <functional>
#include <vector>

#include <TFile.h>
#include <TF1.h>
#include <TH1D.h> // 1 Dimesional ROOT histogram
#include <TTree.h>
#include <TMeas.h>
#include <TWaveform.h>
#include <TMeasHeader.h>
#include <TMath.h>
#include <TPad.h>

#include <SMSDetector.h>
#include <Source.h>
#include <utilities.h>
#include <Carrier.h>
#include <CarrierCollection.h>
#include <global.h>



using std::vector;

extern TH1D *H1DConvolution( TH1D *htct, Double_t Cend=0. , int tid=0) ; 

class TRACSInterface
{

//friend TTree * GetTree( ) ;
private:

	// Declaring external convolution function
	double pitch;
	double width;
	double depth;
	double temp;
	double trapping;
	double fluence;
	double C;
	double dt;
	double max_time;
	double vInit; //added v
	double deltaV;
	double vMax;
	double v_depletion;
	double deltaZ;
	double zInit;
	double zMax;
	double yInit;
	double yMax; //((2*nns)+1)*pitch,
	double deltaY; //added ^
	double vBias;
	double vDepletion;
	double zPos;
	double yPos;
	double tolerance;
	double chiFinal;

	int nThreads;
	int nns;
	int n_cells_y;
	int n_cells_x;
	int n_tSteps;
	int waveLength; //added v
	int n_vSteps;
	int n_zSteps, n_zSteps1, n_zSteps2, n_zSteps_array, n_zSteps_iter, n_balance;
	int n_ySteps;
	//int num_threads;


	int n_par0;
	int n_par1;
	int n_par2;
	std::vector<int> params = {0, 0, 0};
	int tcount;
	int count1, count2, count3;

	char bulk_type;
	char implant_type;

	std::vector<double> neff_param = {0};
	std::valarray<double> i_total;
	std::valarray<double> i_elec;
	std::valarray<double> i_hole;
	std::valarray<double> i_shaped;

	std::vector<double>  z_shifts;
	vector<vector <double> >  z_shifts_array;

	//double z_shifts_array[10][10];
	std::vector<double>  z_shifts1, z_shifts2;
	std::vector<double>  y_shifts; // laser shift in X axis to center laser focus over read-out strip
	std::vector<double>  voltages;

	//vector of i_total
	//std::vector<double> vI_totals;
	//std::vector<double> valItotals;

	std::string carrierFile;
	std::string neffType;
	std::string scanType;

	//file naming
	std::string trap, start;
	// Convert relevant simulation numbers to string for fileNaming
	std::string dtime;
	std::string neigh;
	std::string stepV;
	std::string stepZ;
	std::string stepY;
	std::string cap;
	//std::string z_step  = std::to_string((int) std::floor(deltaZ));
	std::string voltage;

	// filename for data analysis
	std::string hetct_conv_filename;
	std::string hetct_noconv_filename;
	std::string hetct_rc_filename;

	//TH1D i_ramo;
	TH1D *i_ramo;
	TH1D *i_rc;
	TH1D *i_conv;



	//TH1D *hnoconv , *hconv;
	// Pointer to detector and carrier collection
	SMSDetector * detector;
	//SMSDetector * pDetector;
	CarrierCollection * carrierCollection;

	//vector of i_total
	//std::valarray<std::valarray <double> > vItotals;

	//Time variables
	UShort_t year, month, day, hour, min, sec;

public:

	// Constructor
	TRACSInterface(std::string filename); // Reads values, initializes detector

	// Destructor
	~TRACSInterface();

	// Getters
	//TH1D GetItRamo();
	TH1D *GetItRamo();
	TH1D *GetItRc();
	TH1D *GetItConv();
	std::vector<double> get_NeffParam(); //Returns Neff parametrization
	inline int GetnSteps(){
		return n_tSteps;
	}

	inline double GetTolerance(){
			return tolerance;
	}

	inline double GetchiFinal(){
			return chiFinal;
	}

	//Tree functions
	//friend TTree * GetTree(); //Returns the pointer to the TRACS simulated tree
	//friend void DumpToTree( TMeas em , TTree *tree );

	// Simulations
	void simulate_ramo_current();
	void calculate_fields();

	//Calculate time
	UShort_t GetYear();
	UShort_t GetMonth();
	UShort_t GetDay();
	UShort_t GetHour();
	UShort_t GetMinute();
	UShort_t GetSecond();



	//Loops
	void loop_on(int tid = 0); //MULTITHREADING

	// Setters
	void set_NeffParam(std::vector<double> newParam);
	void set_trappingTime(double newTrapTime);
	void set_zPos(double newZPos);
	void set_yPos(double newYPos);
	void set_vBias(double newVBias);
	void set_tcount(int tid = 0);
	void write_header(int tid = 0);
	void resize_array();
	void write_to_file(int tid = 0);
	void set_neffType(std::string newParametrization);
	void set_carrierFile(std::string newCarrFile);
	void set_vItotals(double);

     //ROOT related
	void DumpToTree( TMeas *em , TTree *tree ) ;
	void GetTree( TTree * tree ) ;
};

#endif // TRACSINTERFACE_H
