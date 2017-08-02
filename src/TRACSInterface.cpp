/*
 * @ Copyright 2014-2017 CERN and Instituto de Fisica de Cantabria - Universidad de Cantabria. All rigths not expressly granted are reserved [tracs.ssd@cern.ch]
 * This file is part of TRACS.
 *
 * TRACS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
 * either version 3 of the Licence.
 *
 * TRACS is distributed in the hope that it will be useful , but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with TRACS. If not, see <http://www.gnu.org/licenses/>
 */

/************************************TRACSInterface***********************************
 *
 * High-level class used as interface between files and functionalities.
 *
 */


#include <TRACSInterface.h>
#include <mutex>          // std::mutex


std::mutex mtx2;

/*
 * The constructor mainly initializes all the values that will be used during the execution. Firstly it read most of them from the steering file by means of a parsing method inside utilities class.
 * Another important task carrying out here is the definition of the vectors and coordinates. Vectors to store currents and coordinates to define the scanning, positions, steps...
 * When the information is set up, a new instance of a detector can be launched, one per TRACSInterface object, avoiding data races and concurrent access to same positions memory when solving field equations in the
 * detector. Making copy constructor and passing copies of detector instances is expensive for the program and do not work, neither improve the performance. Same behavior is used with the current collection.
 * The only global variable shared by threads is the one that stores the total induce current vItotals, but it is important to note that threads access uniquely to their tid position in the vector.
 */
/**
 *
 * @param filename
 */
TRACSInterface::TRACSInterface(std::string filename)
{
	neff_param = std::vector<double>(8,0);
	total_crosses = 0;

	utilities::parse_config_file(filename, carrierFile, depth, width,  pitch, nns, temp, trapping, fluence, nThreads, n_cells_x, n_cells_y, bulk_type,
			implant_type, waveLength, scanType, C, dt, max_time, vInit, deltaV, vMax, vDepletion, zInit, zMax, deltaZ, yInit, yMax, deltaY, neff_param, neffType,
			tolerance, chiFinal, diffusion, fitNorm);

	// Initialize vectors / n_Steps / detector / set default zPos, yPos, vBias / carrier_collection

	if (fluence == 0) // if no fluence -> no trapping
	{
		//Trapping configuration
		trapping = std::numeric_limits<double>::max();
		trap = "NOtrapping";
		start = "NOirrad";
	}
	else{
		//trapping configuration
		trap = std::to_string((int) std::floor(1.e9*trapping));
		start = "irrad";

	}

	n_zSteps = (int) std::floor((zMax-zInit)/deltaZ); // Simulation Steps
	n_zSteps1 = n_zSteps / 2;
	n_zSteps2 = (int) std::floor (n_zSteps - n_zSteps1);

	// if more threads than points
	if(num_threads>n_zSteps+1)
	{
		num_threads = n_zSteps+1;
		std::cout << "No. of threads > No. of z points! reducing No. of threads to."<< num_threads << std::endl;
	}

	n_zSteps_array = (int) std::floor ((n_zSteps+1) / num_threads);
	n_zSteps_iter = (int) std::round ((n_zSteps+1) / (num_threads)*1.0);
	n_vSteps = (int) std::floor((vMax-vInit)/deltaV);
	n_ySteps = (int) std::floor((yMax-yInit)/deltaY);
	//WRITING TO FILES!
	// Convert relevant simulation numbers to string for fileNaming	
	dtime = std::to_string((int) std::floor(dt*1.e12));
	neigh = std::to_string(nns);
	stepV = std::to_string((int) std::floor(deltaV)); 
	stepZ = std::to_string((int) std::floor(deltaZ));
	stepY = std::to_string((int) std::floor(deltaY));
	cap = std::to_string((int) std::floor(C*1.e12));
	voltage = std::to_string((int) std::floor(vInit));

	parameters["allow_extrapolation"] = true;

	detector = new SMSDetector(pitch, width, depth, nns, bulk_type, implant_type, n_cells_x, n_cells_y, temp, trapping, fluence, neff_param, neffType, diffusion, dt);

	n_tSteps = (int) std::floor(max_time / dt);

	carrierCollection = new CarrierCollection(detector);
	QString carrierFileName = QString::fromUtf8(carrierFile.c_str());
	carrierCollection->add_carriers_from_file(carrierFileName);


	//currents
	i_elec.resize((size_t) n_tSteps);
	i_hole.resize ((size_t) n_tSteps);
	i_total.resize((size_t) n_tSteps);
	i_shaped.resize((size_t) n_tSteps);


	z_shifts.resize((size_t) n_zSteps+1,0.);
	z_shifts1.resize((size_t) n_zSteps1+1,0.);
	z_shifts2.resize((size_t) n_zSteps2+1,0.);

	//distribute z coordinates evenly between threads
	z_shifts_array.resize(num_threads);
	int t_sum = 0;
	for (int i = 0; i < num_threads; i++)
	{
		n_zSteps_iter = (int) (std::ceil (((float)(n_zSteps+1-t_sum) / (num_threads-i))));
		z_shifts_array[i].resize(n_zSteps_iter, 0.);
		t_sum += n_zSteps_iter;
	}

	y_shifts.resize ((size_t) n_ySteps+1,0.);
	voltages.resize((size_t) n_vSteps+1,0.);

	// Create voltages
	for (int i = 0; i < n_vSteps + 1; i++ ) 
	{
		voltages[i] = (i*deltaV)+vInit;
	}
	// CreatE shifts in Z
	for (int i = 0; i < n_zSteps + 1; i++ ) 
	{
		z_shifts[i] = (i*deltaZ)+zInit;	
	}


	//"sampling" z array
	int l = 0;

	for (int i = 0; i < num_threads; i++)
	{	l = i;
	for (uint j = 0; j < z_shifts_array[i].size(); j++ )
	{
		z_shifts_array[i][j] = z_shifts[l];
		l+=num_threads;
	}
	}

	// Create shifts in Y
	for (int i = 0; i < n_ySteps + 1; i++ ) 
	{
		y_shifts[i] = (i*deltaY)+yInit;
	}

	vBias = vInit;
	set_tcount(0);
	vItotals.resize(voltages.size()*z_shifts.size());
	i_ramo  = NULL;
	i_rc    = NULL;
	i_conv  = NULL;

}


// Destructor
TRACSInterface::~TRACSInterface()
{
	delete i_ramo;
	delete i_rc;
	delete i_conv;
	delete carrierCollection;
	delete detector;
}

/*
 * Convert i_total to TH1D. ROOT based method.
 */
TH1D * TRACSInterface::GetItRamo()
{
	if (i_ramo != NULL) 
	{
	}
	else
	{
		//TF1 *f1 = new TF1("f1","abs(sin(x)/x)*sqrt(x)",0,10);
		//float r = f1->GetRandom();
		TString htit, hname;
		htit.Form("ramo_%d_%d", tcount, count1);
		hname.Form("Ramo_current_%d_%d", tcount, count1);
		i_ramo  = new TH1D(htit,hname,n_tSteps, 0.0, max_time);
		//std::cout << htit << std::endl;

		// Compute time + format vectors for writting to file
		for (int j=0; j < n_tSteps; j++)
		{
			i_ramo->SetBinContent(j+1, i_total[j] );
		}
		count1++;

	}
	return i_ramo;
}

/*
 * Convert i_total to TH1D after simulating simple RC circuit. ROOT based method.
 */
TH1D * TRACSInterface::GetItRc()
{

	if (i_rc != NULL) 
	{
	}
	else
	{	
		//TF1 *f1 = new TF1("f1","abs(sin(x)/x)*sqrt(x)",0,10);
		//float r = f1->GetRandom();
		TString htit, hname;
		htit.Form("ramo_rc%d%d", tcount, count2);
		hname.Form("Ramo_current_%d_%d", tcount, count2);
		i_rc    = new TH1D(htit,hname,n_tSteps, 0.0, max_time);

		double RC = 50.*C; // Ohms*Farad
		double alfa = dt/(RC+dt);

		for (int j = 1; j <n_tSteps; j++) 
		{
			i_shaped[j]=i_shaped[j-1]+alfa*(i_total[j]-i_shaped[j-1]);
			i_rc->SetBinContent(j+1, i_shaped[j]);
		}
		count2++;

	}
	return i_rc;
}

/*
 * Convert i_total to TH1D after convolution with the amplifier TransferFunction. ROOT based method.
 */
TH1D * TRACSInterface::GetItConv()
{
	if (i_conv != NULL) 
	{
	}
	else
	{
		//TF1 *f1 = new TF1("f1","abs(sin(x)/x)*sqrt(x)",0,10);
		//float r = f1->GetRandom();
		TString htit, hname;
		htit.Form("ramo_conv_%d_%d", tcount, count3);
		hname.Form("Ramo current_%d_%d", tcount, count3);
		i_conv  = new TH1D(htit,hname,n_tSteps, 0.0, max_time);
		i_ramo = GetItRamo();
		//mtx2.lock();
		i_conv = H1DConvolution( i_ramo , C*1.e12, tcount );
		//mtx2.unlock();
		count3++;
	}
	return i_conv;
}

/*
 * Performs the simulation for all given carriers and stores the current in a valarray.
 * No variable is returned. To get the current one must choose the apropiate Getter for 
 * one's needs
 */
void TRACSInterface::simulate_ramo_current()
{
	i_rc = nullptr;
	i_ramo = nullptr;
	i_conv = nullptr;
	i_hole = 0;
	i_elec = 0;
	i_total = 0;

	/*Important for Diffusion:
	yPos is later transformed into X.
	zPos is later transformed into Y.*/
	carrierCollection->simulate_drift( dt, max_time, yPos, zPos, i_elec, i_hole, total_crosses);
	i_total = i_elec + i_hole;
}



double TRACSInterface::get_vDep(){
	return vDepletion;
}
double TRACSInterface::get_fitNorm(){
	return fitNorm;
}

std::string TRACSInterface::get_neff_type(){
	return neffType;
}

/*
 *
 * Returns Neff parametrization
 *
 */
std::vector<double>TRACSInterface::get_NeffParam()
{
	return neff_param;
} 

int TRACSInterface::GetnSteps(){
	return n_tSteps;
}

double TRACSInterface::GetTolerance(){
	return tolerance;
}

double TRACSInterface::GetchiFinal(){
	return chiFinal;
}

int TRACSInterface::GettotalCrosses(){
	return total_crosses;
}

UShort_t TRACSInterface::GetYear(){
	time_t currentTime;
	struct tm *localTime;

	time( &currentTime );                   // Get the current time
	localTime = localtime( &currentTime );  // Convert the current time to the local time
	year = localTime->tm_year + 1900;
	return year;
}

UShort_t TRACSInterface::GetMonth(){
	time_t currentTime;
	struct tm *localTime;

	time( &currentTime );                   // Get the current time
	localTime = localtime( &currentTime );  // Convert the current time to the local time
	month = localTime->tm_mon + 1;
	return month;
}
UShort_t TRACSInterface::GetDay(){
	time_t currentTime;
	struct tm *localTime;

	time( &currentTime );                   // Get the current time
	localTime = localtime( &currentTime );  // Convert the current time to the local time
	day = localTime->tm_mday;
	return day;
}
UShort_t TRACSInterface::GetHour(){
	time_t currentTime;
	struct tm *localTime;

	time( &currentTime );                   // Get the current time
	localTime = localtime( &currentTime );  // Convert the current time to the local time
	hour = localTime->tm_hour;
	return hour;
}
UShort_t TRACSInterface::GetMinute(){
	time_t currentTime;
	struct tm *localTime;

	time( &currentTime );                   // Get the current time
	localTime = localtime( &currentTime );  // Convert the current time to the local time
	min = localTime->tm_min;
	return min;
}
UShort_t TRACSInterface::GetSecond(){
	time_t currentTime;
	struct tm *localTime;

	time( &currentTime );                   // Get the current time
	localTime = localtime( &currentTime );  // Convert the current time to the local time
	sec = localTime->tm_sec;
	return sec;
}

/*
 * Sets the trapping time in the detector to the input value. 
 * Remember that the trapping time must be a positive number.
 * The smaller the trapping time the bigger the signal loss.
 */
/**
 *
 * @param newTrapTime
 */
void TRACSInterface::set_trappingTime(double newTrapTime)
{
	trapping = newTrapTime;
	detector->set_trapping_time(trapping);
}

/*
 * Sets how much the carriers will be displaced in the Z axis from its original 
 * position in the file read by TRACS. Note that if the carriers are not inside 
 * the detector they will not produce current. This is relevant mainly for 
 * edge-TCT simulations.
 */
/**
 *
 * @param newZPos
 */
void TRACSInterface::set_zPos(double newZPos)
{
	zPos = newZPos;
}

/*
 * Sets how much the carriers will be displaced in the Y axis from its original 
 * position in the file read by TRACS. Note that if the carriers are not inside 
 * the detector they will not produce current. This is used to center the red 
 * pulse in redTCT and to center the focus in TPA and edgeTCT
 */
/**
 *
 * @param newYPos
 */
void TRACSInterface::set_yPos(double newYPos)
{
	if (std::abs(newYPos) > (2*nns+1)*pitch)
	{
		std::cout << "Watch out! You probably set the laser out of the detector" << std::endl;
	}
	yPos = newYPos;
}

/*
 * Sets bias voltages in the detector, fields should be recalculated again 
 * before simulating any transients
 */
/**
 *
 * @param newVBias
 */
void TRACSInterface::set_vBias(double newVBias)
{
	vBias = newVBias;
	detector->set_voltages(vBias, vDepletion);
}

/*
 * Sets a number (current thread).
 * Used to index the different output files 
 *
 */
/**
 *
 * @param tid
 */
void TRACSInterface::set_tcount(int tid)
{
	tcount = tid;
}

/*
 * Sets the desired Neff parameters in the detector. Fields should be calculated
 * again before simulating any current. Note that different neff parametrizations
 * use different parameters so not all may be used at once.
 */
/**
 *
 * @param newParam
 */
void TRACSInterface::set_FitParam(std::vector<double> newFitParam)
{
	detector->setFitParameters(newFitParam);
	fitNorm = newFitParam[8];

}

void TRACSInterface::set_Fit_Norm(std::vector<double> vector_fitTri)
{
	//vDepletion = vector_fitTri[0];
	fitNorm = vector_fitTri[0];

}



/*
 * Calculates the electric field and potential inside the detector. It is 
 * required after any modification of the Neff or the bias voltage applied. 
 * Weighting field and potential need not be calculated again since they 
 * are independent on those parameters.
 */



void TRACSInterface::calculate_fields()
{
	// Get detector ready

	detector->solve_w_u();
	detector->solve_d_u();
	detector->solve_w_f_grad();
	detector->solve_d_f_grad();
	detector->get_mesh()->bounding_box_tree();

}

/*
 * Change parametrization of the Neff. Possibilities are: Trilinear (default)
 * Linear, Triconstant. More information on this three parametrizarions can
 * be found in the documentation (Config.TRACS and README.md)
 */
/**
 *
 * @param newParametrization
 */
void TRACSInterface::set_neffType(std::string newParametrization)
{
	neffType = newParametrization;
	detector->set_neff_type(neffType);

}

/*
 * Allows the user to select a new carrier distribution from a file that
 * complies with the TRACS format. 
 */
/**
 *
 * @param newCarrFile
 */
void TRACSInterface::set_carrierFile(std::string newCarrFile)
{
	QString carrierFileName = QString::fromUtf8(newCarrFile.c_str());
	carrierCollection->add_carriers_from_file(carrierFileName);
}

/**
 *
 * @param tid
 */



void TRACSInterface::loop_on(int tid)
{


	for (int vPos = 0; vPos < n_vSteps + 1; vPos++)
	{
		//vPos = Voltage steps. Normally one step is used.

		detector->set_voltages(voltages[vPos], vDepletion);
		mtx2.lock();
		calculate_fields();
		mtx2.unlock();

		for (int yPos = 0; yPos < n_ySteps + 1; yPos++)
		{
			set_yPos(y_shifts[yPos]);
			/*for (params[0] = 0; params[0] < n_par0 + 1; params[0]++)*/
			for (uint zPos = 0; zPos < z_shifts_array[tid].size(); zPos++)
			{

				std::cout << "Height " << z_shifts_array[tid][zPos] << " of " << z_shifts.back()  <<  " || Y Position " << y_shifts[yPos]
						  << " of " << y_shifts.back() << " || Voltage " << voltages[vPos] << " of " << voltages.back() << std::endl;
				set_zPos(z_shifts_array[tid][zPos]);
				simulate_ramo_current();
				if (num_threads > 1)
					i_rc_array[tid][zPos] = GetItRc();
				else i_rc_array[vPos][zPos] = GetItRc();

				//-------------------------
				//Build and sort the final array of currents.
				//First, get the position of the Z.
				std::vector<double>::iterator it = find(z_shifts.begin(), z_shifts.end(), z_shifts_array[tid][zPos]);
				auto pos = it - z_shifts.begin();
				int ind = vPos*z_shifts.size() + pos;
				vItotals[ind] = i_shaped;
				//-------------------------
				//Filling histograms-------> commented for Fitting
				//i_ramo = GetItRamo();
				//i_ramo_array[tid][zPos] = i_ramo; // for output
				//i_ramo = NULL;
				//i_rc = GetItRc();
				//i_rc_array[tid][zPos] = i_rc; // for output

				//Check this: Pbm with TFile opened many times. Commented 8.11.2016 cause we don't need conv for the moment
				//mtx2.lock();
				//i_conv = GetItConv();
				//i_conv_array[tid][zPos] = i_conv; // for output
				//mtx2.unlock();

			}


		}
		if (tid == 0) fields_hist_to_file(tid, vPos);

	}

}

/*
 *
 * Write to file header. The input int is used to label files (multithreading)!
 *
 *
 *
 */
/**
 *
 * @param tid
 */
void TRACSInterface::write_header(int tid)
{
	// Convert Z to milimeters
	std::vector<double> aux_zsh(n_zSteps+1);
	aux_zsh = z_shifts;
	std::transform(aux_zsh.begin(), aux_zsh.end(), aux_zsh.begin(), std::bind1st(std::multiplies<double>(),(1./1000.)));

	// Convert Z to milimeters
	std::vector<double> aux_ysh(n_ySteps+1);
	aux_ysh = y_shifts;
	std::transform(aux_ysh.begin(), aux_ysh.end(), aux_ysh.begin(), std::bind1st(std::multiplies<double>(),(1./1000.)));
	// filename for data analysis
	//hetct_conv_filename = start+"_dt"+dtime+"ps_"+cap+"pF_t"+trap+"ns_dz"+stepZ+"um_dy"+stepY+"dV"+stepV+"V_"+neigh+"nns_"+scanType+"_"+std::to_string(tcount)+"_conv.hetct";
	//hetct_noconv_filename = start+"_dt"+dtime+"ps_"+cap+"pF_t"+trap+"ns_dz"+stepZ+"um_dy"+stepY+"dV"+stepV+"V_"+neigh+"nns_"+scanType+"_"+std::to_string(tcount)+"_noconv.hetct";
	hetct_rc_filename = start+"_dt"+dtime+"ps_"+cap+"pF_t"+trap+"ns_dz"+stepZ+"um_dy"+stepY+"dV"+stepV+"V_"+neigh+"nns_"+scanType+"_"+std::to_string(tcount)+"_rc.hetct";


	// write header for data analysis
	//utilities::write_to_hetct_header(hetct_conv_filename, detector, C, dt, aux_ysh, aux_zsh, waveLength, scanType, carrierFile, voltages);
	//utilities::write_to_hetct_header(hetct_noconv_filename, detector, C, dt, aux_ysh, aux_zsh, waveLength, scanType, carrierFile, voltages);
	utilities::write_to_hetct_header(hetct_rc_filename, detector, C, dt, aux_ysh, aux_zsh, waveLength, scanType, carrierFile, voltages);

}
/*
 *Resizing for storing data in memory 
 *and using it later/writing to a single output file.
 *
 */
void TRACSInterface::resize_array()
{
	i_ramo_array.resize(num_threads);
	i_rc_array.resize(num_threads);
	i_conv_array.resize(num_threads);

	for (int i = 0; i < num_threads; i++)
	{
		i_ramo_array[i].resize(z_shifts_array[i].size());
		i_rc_array[i].resize(z_shifts_array[i].size());
		i_conv_array[i].resize(z_shifts_array[i].size());
		std::cout << "i_ramo_array[xlen][ylen]   " << i_ramo_array.size()<<"  " <<i_ramo_array[i].size() <<std::endl;
	}


	if(num_threads == 1){
		i_rc_array.resize(n_vSteps + 1);
		for (int i = 0; i <= n_vSteps; i++)
		{
			i_rc_array[i].resize(z_shifts_array[0].size());
		}
	}



}
/*
 * Writing to a single file
 *
 *
 */
/**
 *
 * @param tid
 */
void TRACSInterface::write_to_file(int tid)
{
	write_header(tid);

	std::cout << "Writing to file..." <<std::endl;

	//n_par0 = (int) z_shifts_array[tid].size()-1;
	params[0] = 0; //thread
	params[1] = 0; //yPos;
	params[2] = 0; //vPos;
	n_par1 = n_ySteps;
	n_par2 = n_vSteps;
	//loop
	for (params[2] = 0; (num_threads == 1) ? params[2] <= 0 : params[2] < n_par2 + 1; params[2]++)
	{
		for (params[1] = 0; (num_threads == 1) ? params[1] <= 0 : params[1] < n_par1 + 1; params[1]++)
		{
			for (int i = 0; (num_threads == 1) ? i < n_vSteps + 1 : i < num_threads; i++)
			{
				for (params[0] = 0; (num_threads == 1) ? params[0] < i_rc_array[i].size() : params[0] < i_ramo_array[i].size(); params[0]++)
				{
					//utilities::write_to_file_row(hetct_noconv_filename, i_ramo_array[i][params[0]], detector->get_temperature(), y_shifts[params[1]], z_shifts_array[i][params[0]], voltages[params[2]]);
					//utilities::write_to_file_row(hetct_conv_filename, i_conv_array[i][params[0]], detector->get_temperature(), y_shifts[params[1]], z_shifts_array[i][params[0]], voltages[params[2]]);
					if(num_threads > 1)
						utilities::write_to_file_row(hetct_rc_filename, i_rc_array[i][params[0]], detector->get_temperature(), y_shifts[params[1]], z_shifts_array[i][params[0]], voltages[params[2]]);
					else utilities::write_to_file_row(hetct_rc_filename, i_rc_array[i][params[0]], detector->get_temperature(), y_shifts[0], z_shifts_array[0][params[0]], voltages[i]);

				}

			}

		}

	}



}

void TRACSInterface::fields_hist_to_file(int tid, int vPos)
{

	/*Exporting 2D Histograms to file*/
	// get plot and set new data
	TString file_name;
	file_name.Form("wf%dV", TMath::Nint(voltages[vPos]));//"2Dhistos"+voltages[vPos]+"V"+".root";
	TFile *fout = new TFile(file_name,"RECREATE");

	TString wpm;
	wpm.Form("WP_%d_V", TMath::Nint(voltages[vPos]) ) ;
	TString wfm;
	wfm.Form("WF_%d_V", TMath::Nint(voltages[vPos]) ) ;
	TString efm;
	efm.Form("EF_%d_V", TMath::Nint(voltages[vPos]) ) ;

	TH2D h_w_u      = utilities::export_to_histogram    ( *detector->get_w_u(), "h_w_u", wpm /*"Weighting potential"*/, detector->get_n_cells_x(), detector->get_x_min(), detector->get_x_max(), detector->get_n_cells_y(), detector->get_y_min(), detector->get_y_max());
	TH2D h_w_f_grad = utilities::export_mod_to_histogram( *detector->get_w_f_grad(), "h_w_f_grad", wfm /*"Weighting field"*/, detector->get_n_cells_x(), detector->get_x_min(), detector->get_x_max(), detector->get_n_cells_y(), detector->get_y_min(), detector->get_y_max());
	TH2D h_d_f_grad = utilities::export_mod_to_histogram( *detector->get_d_f_grad(), "h_d_f_grad", efm /*"Electric field"*/, detector->get_n_cells_x(), detector->get_x_min(), detector->get_x_max(), detector->get_n_cells_y(), detector->get_y_min(), detector->get_y_max());

	h_w_u.Write();
	h_w_f_grad.Write();
	h_d_f_grad.Write();
	fout->Close();
}






/* ---------------------------------------------------------------- */
/**
 *
 * @param tree
 */
void TRACSInterface::GetTree( TTree *tree ) {

	//TFile *f=new TFile("test.root","RECREATE") ;

	// Create a ROOT Tree
	//tree->SetDirectory(0);

	// Create a pointer to an raw data object
	TMeas *em = new TMeas( );
	em->Nt = n_tSteps ;
	em->volt = new Double_t [n_tSteps] ;
	em->time = new Double_t [n_tSteps] ;
	em->Qt = new Double_t [n_tSteps] ;

	// Create branches
	tree->Branch("raw", &em,32000,0);

	//Read RAW file
	DumpToTree( em , tree ) ;
	//tree->Draw("volt-BlineMean:time","event==0","l"); gPad->Modified();gPad->Update();
	//f->Write();
	//f->Close();


	delete em ;

}
/*----------------------------------------------------------*/
/**
 *
 * @param em
 * @param tree
 */
void TRACSInterface::DumpToTree( TMeas *em , TTree *tree ) {

	//Time information
	TString sdate; //= TString( line ) ;
	UShort_t  dd, mm, yy , hh , mn, ss;
	yy=GetYear();
	mm=GetMonth();
	dd=GetDay();
	hh=GetHour();
	mn=GetMinute();
	ss=GetSecond();
	TDatime date ;
	date.Set(yy, mm, dd, hh, mn, ss) ;
	em->utc = date ;

	//Total number of Scans

	Int_t NumOfScans = 1 ;
	if ( n_vSteps !=0 ) NumOfScans = NumOfScans * (1 + n_vSteps) ;
	if ( n_zSteps !=0 ) NumOfScans = NumOfScans * (1+ n_zSteps) ;
	if ( n_ySteps !=0 ) NumOfScans = NumOfScans * (1+ n_ySteps);


	// NVoltages
	em->NV = n_vSteps + 1 ;

	//Create a TMeasHeader object to store info that does not depend on event
	TMeasHeader *emh=new TMeasHeader( em->NV ) ;
	emh->Lambda = waveLength ;
	emh->NV = em->NV;
	emh->comment=TString( "TRACS simulated data" ) ;
	emh->Setup = 5 ;
	emh->Fluence = fluence ;
	emh->Nav  = 1 ;
	emh->Gain = 1.0 ;
	emh->iann = 0. ;
	emh->Illum = -2 ;

	//Cend
	emh->Cend = 0. ;

	//Vbias vector
	for ( int i=0 ; i< em->NV ; i++) emh->vVbias[i] = voltages[i] ;

	//Nominal power
	emh->Power = 1.0 ;

	//Ax
	emh->Ax = 0. ;
	emh->Nx = 1 ;

	//Ay
	em->Ay  = emh->Ay = deltaY  ;
	em->Ny  = emh->Ny = n_ySteps + 1 ;

	//Az
	em->Az  = emh->Az = deltaZ  ;
	em->Nz  = emh->Nz = n_zSteps + 1;

	em->Temp = temp ;

	int  iRead=0 , iactual = 1 ;
	int  Polarity = 0 ;
	Double_t x0=0., y0=0., z0=0., t0s;
	em->x = 0. ;
	for (Int_t iloop = 0 ; iloop < NumOfScans ; iloop++ ) {

		Int_t tms = iloop * 4000 ;
		em->Vbias = detector->get_vbias();
		em->y = carrierCollection->beamy;
		em->z = carrierCollection->beamy;

		em->Itot = 0. ;

		//Calculate the bin slice along each coordinate
		if (iloop==0) { x0=em->x ;y0=em->y ;z0=em->z ; }
		em->ix = (em->Ax!=0.)? 1 + TMath::Nint((em->x - x0)/(em->Ax)):1 ;
		em->iy = (em->Ay!=0.)? 1 + TMath::Nint((em->y - y0)/(em->Ay)):1 ;
		em->iz = (em->Az!=0.)? 1 + TMath::Nint((em->z - z0)/(em->Az)):1 ;

		//Calculate the time increment wrt t0s
		if (iloop==0) t0s=tms ;
		Double_t Nseconds = 0.001*(tms - t0s) ;
		UShort_t ddi , hhi , mni, ssi , ndd, nhh, nmn , nss  ;
		ndd = TMath::Floor(Nseconds/86400) ;
		ddi = dd + ndd ;
		nhh = TMath::Floor((Nseconds - ndd*86400)/3600.) ;
		hhi = hh+nhh;
		nmn = TMath::Floor((Nseconds - ndd*86400-nhh*3600)/60) ;
		mni = mn+nmn;
		nss = TMath::Floor(Nseconds - ndd*86400-nhh*3600-nmn*60) ;
		ssi = ss + nss;
		if (ssi>60) { mni++ ; ssi=ssi-60; }
		if (mni>60) { hhi++ ; mni=mni-60; }
		if (hhi>24) { ddi++ ; hhi=hhi-24; }
		date.Set(yy, mm, ddi, hhi, mni, ssi) ;
		em->utc = date ;

		em->At = dt*1.e9 ; emh->At = em->At ;
		std::valarray<double> I_tot = vItotals[iloop];//vectorObjetosSimuladosAlmacenadosCorrelativamente.Get_Itot[iloop] ;
		for ( int i=0 ; i< em->Nt ; i++) {
			em->volt[i] = I_tot[i] ;
			em->time[i] = i*em->At ;

		}

		//Estimate polarity
		if ( TMath::Abs(TMath::MaxElement(em->Nt,em->volt)) > TMath::Abs(TMath::MinElement(em->Nt,em->volt)) ) Polarity++ ;
		else Polarity-- ;

		em->event=iRead ;

		//Now postprocess this entry (find out baseline, rtime and so on
		//TWaveform *wvi = new TWaveform( em ) ;
		TWaveform wvi = TWaveform( em ) ;

		if (iRead==0) tree->Branch("proc" , &wvi , 32000 , 0 );


		tree->Fill() ;
		iRead++   ;

		//delete wvi ;

		iactual++ ;

	}

	emh->Polarity = (Polarity>1) ? 1 : -1 ;
	tree->GetUserInfo()->Add( emh ) ;
	em->Ntevent=iRead ; //It does not go into the tree, only in the class!

	//delete emh ;


}


