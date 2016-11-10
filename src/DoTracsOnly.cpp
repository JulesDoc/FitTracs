/*
 * DoTracsOnly.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: jcalvopi
 */


/*
  Example

    DoTRACSFit MeasurementFile TRACS.conf "Vbias==200 && Tset == 20"

 */

//#include <TApplication.h>
#include <thread>
#include <boost/asio.hpp>
#include <threading.h>

#include <TRACSFit.h>
#include <TRACSInterface.h>
#include <TString.h>
#include <stdio.h>

std::vector<TRACSInterface*> TRACSsim;
std::vector<std::thread> t;


int main( int argc, char *argv[]) {

	if(argc==1)
	{
		num_threads = std::thread::hardware_concurrency(); // No. of threads = No. of cores
	}
	else num_threads = atoi(argv[1]);

	if (num_threads == 0){

		num_threads = 1;
	}

	TRACSsim.resize(num_threads);
	t.resize(num_threads);
	for (uint i = 0; i < num_threads; ++i) {
		t[i] = std::thread(call_from_thread, i);
	}
	for (int i = 0; i < num_threads; ++i) {
		t[i].join();
	}


	//write output to single file!
	TRACSsim[0]->write_to_file(0);
	//Finalizing the execution
	//getter test
	std::vector<double> neff_test = TRACSsim[0]->get_NeffParam();
	std::cout << "Neff param.: " << std::endl;
	for (int i = 0; i < 8; i++)
	{
		std::cout << neff_test[i] << std::endl;
	}

	//Clean
	for (int i = 0; i < TRACSsim.size(); i++)	{
		delete TRACSsim[i];
	}

	std::quick_exit(1);
}

//-----------
Double_t TRACSFit::operator() ( const std::vector<Double_t>& par  ) const {

	return 0;
}
