#include <CarrierCollection.h>
#include <CarrierMobility.h>


// _carrier_list should be a N_thr-dimensional vector

/*
 * Comments
 *
 * Simulate_drift is an overloaded function 
 * 		WATCHOUT
 */

CarrierCollection::CarrierCollection(SMSDetector * detector) :
_detector(detector)
{

}


/*
 ********************** OVERLOADED FUNCTIONS FOR GUI COMPATIBILITY **************************
 */

///USED
void CarrierCollection::add_carriers_from_file(QString filename)
{
	// get char representation and make ifstream
	char * char_fn = filename.toLocal8Bit().data();
	std::ifstream infile(char_fn);

	// process line by line
	std::string line;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		char carrier_type;
		double q, x_init, y_init, gen_time;
		if (!(iss >> carrier_type >> q >> x_init >> y_init >> gen_time)) { 
			std::cout << "Error while reading file" << std::endl; 
			break;
		} 

		//Calculate average beam position
		beamy += x_init;
		beamz += y_init;
		Carrier carrier(carrier_type, q, x_init, y_init , _detector, gen_time);
		_carrier_list_sngl.push_back(carrier);
	}
	if ( _carrier_list_sngl.size()!=0 ) {
		beamy = beamy / _carrier_list_sngl.size();
		beamz = beamz / _carrier_list_sngl.size();
	}
	//double sizeCarriers = _carrier_list_sngl.size();
	//std::cout << "****************************Carrier list size: "  << sizeCarriers << std::endl;
}

//USED
void CarrierCollection::simulate_drift( double dt, double max_time, double shift_x /*yPos*/, double shift_y /*zPos*/,
		std::valarray<double>&curr_elec, std::valarray<double> &curr_hole, int &totalCrosses)
{

	int i = 0;
	int totalCross = 0;
	// range for through the carriers
	for (auto carrier : _carrier_list_sngl)
	{
		char carrier_type = carrier.get_carrier_type();
		// simulate drift and add to proper valarray
		if (carrier_type == 'e')
		{

			// get and shift carrier position
			std::array< double,2> x = carrier.get_x();
			double x_init = x[0] + shift_x;
			double y_init = x[1] + shift_y;

			curr_elec += carrier.simulate_drift( dt , max_time, x_init, y_init);
		}

		else if (carrier_type =='h')
		{
			// get and shift carrier position
			std::array< double,2> x = carrier.get_x();
			double x_init = x[0] + shift_x;
			double y_init = x[1] + shift_y ;

			curr_hole += carrier.simulate_drift( dt , max_time, x_init, y_init);
		}
		//Let's see how many carriers from the carrier list for this step in Z have crossed to the depleted region.
		//A flag is switched to true on carrier.simulate_drift when a carrier filfull the requirements. See carrier.simulate_drift
		if (carrier.crossed()){
			totalCross += 1;
		}
	}
	std::cout << "Number of carriers crossed to DR in last Z step with Height " << shift_y << ": " << totalCross << std::endl;
	totalCrosses += totalCross;

	double trapping_time = _detector->get_trapping_time();

	for (double i = 0.; i < curr_hole.size(); i ++)
	{
		double elapsedT = i*dt;
		curr_elec[i] *= exp(-elapsedT/trapping_time);
		curr_hole[i] *= exp(-elapsedT/trapping_time);
	}

}

TH2D CarrierCollection::get_e_dist_histogram(int n_bins_x, int n_bins_y,  TString hist_name, TString hist_title)
{
	// get detector limits
	double x_min = _detector->get_x_min();
	double x_max = _detector->get_x_max();
	double y_min = _detector->get_y_min();
	double y_max = _detector->get_y_max();

	// create histogram object
	TH2D e_dist = TH2D(hist_name, hist_title, n_bins_x , x_min, x_max, n_bins_y, y_min, y_max);

	// range for through the carriers and fill the histogram
	for (auto carrier : _carrier_list_sngl)
	{
		char carrier_type = carrier.get_carrier_type();
		if (carrier_type == 'e')
		{
			std::array< double,2> x = carrier.get_x();
			double q = carrier.get_q();
			e_dist.Fill(x[0], x[1], q);
		}
	}
	return e_dist;
}

TH2D CarrierCollection::get_e_dist_histogram(int n_bins_x, int n_bins_y, double shift_x, double shift_y, TString hist_name, TString hist_title)
{
	// get detector limits
	double x_min = _detector->get_x_min();
	double x_max = _detector->get_x_max();
	double y_min = _detector->get_y_min();
	double y_max = _detector->get_y_max();

	// create histogram object
	TH2D e_dist = TH2D(hist_name, hist_title, n_bins_x , x_min, x_max, n_bins_y, y_min, y_max);

	// range for through the carriers and fill the histogram
	for (auto carrier : _carrier_list_sngl)
	{
		char carrier_type = carrier.get_carrier_type();
		if (carrier_type == 'e')
		{
			std::array< double,2> x = carrier.get_x();
			double q = carrier.get_q();
			e_dist.Fill(x[0]+shift_x, x[1]+shift_y, q);
		}
	}
	return e_dist;
}

/*
 ********************** DESTRUCTOR OF THE CLASS CARRIER	COLLECTION **************************
 */
CarrierCollection::~CarrierCollection()
{

}
