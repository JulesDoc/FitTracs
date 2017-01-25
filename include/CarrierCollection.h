#ifndef CARRIER_COLLECTION_H
#define CARRIER_COLLECTION_H

#include "Carrier.h"


#include <string>
#include <sstream>
#include <fstream>

#include <QString>
#include <TH2D.h>
#include <TString.h>
#include <TMath.h>
#include <TRandom3.h>



/*
 ***********************************CARRIER COLLECTION***********************************
 *
 *
 *Detailed description
 *
 *
 *
 */

class CarrierCollection
{
private:
	//std::vector< std::vector<Carrier> > _carrier_list;
	std::vector<Carrier> _carrier_list_sngl;
	SMSDetector * _detector;

	TRandom3 gRandom;



public:
	CarrierCollection(SMSDetector * detector);
	~CarrierCollection();

	double beamy = 0. , beamz = 0.; //Mean position of the injected carriers in detector plane (y,z)

	//Not used
	// void add_carriers_from_file(QString filename, int n_thr);
	//Not used
	//void simulate_drift( double dt, double max_time, std::valarray<double> &curr_elec, std::valarray<double> &curr_hole, int thr_id);
	//USED
	void simulate_drift( double dt, double max_time, double shift_x, double shift_y,  std::valarray<double> &curr_elec, std::valarray<double> &curr_hole, int thr_id);

	//    TH2D get_e_dist_histogram(int n_bins_x, int n_bins_y, TString hist_name = "e_dist", TString hist_title ="e_dist", int thr_id);

	/*
	 ********************* OVERLOADED FUNCTION FOR GUI COMPATIBILITY ****************************
	 */
	//USED
	void add_carriers_from_file(QString filename);
	//Not used
	//void simulate_drift( double dt, double max_time, std::valarray<double> &curr_elec, std::valarray<double> &curr_hole);
	//USED
	void simulate_drift( double dt, double max_time, double shift_x, double shift_y,  std::valarray<double> &curr_elec, std::valarray<double> &curr_hole);

	TH2D get_e_dist_histogram(int n_bins_x, int n_bins_y, TString hist_name = "e_dist", TString hist_title ="e_dist");
	TH2D get_e_dist_histogram(int n_bins_x, int n_bins_y, double shift_x, double shift_y, TString hist_name = "e_dist", TString hist_title ="e_dist");



};




#endif // CARRIER_COLLECTION_H


