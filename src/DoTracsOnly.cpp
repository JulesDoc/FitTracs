/*****This code is property of CERN and IFCA under GPL License. Developed by: Marcos Fernandez, Pablo de Castro, Alvaro Diez, Urban Senica and Julio Calvo.*****/

/************************************DoTRACSOnly***********************************
 *
 * Basic main TRACS execution which gives back the Neff of a defined detector and some extra information related to diffusion, if swithed ON.
 *
 */

#include <thread>
#include <boost/asio.hpp>
#include <TRACSFit.h>
#include <TRACSInterface.h>
#include <TString.h>
#include <stdio.h>
#include "../include/Threading.h"

std::vector<TRACSInterface*> TRACSsim;
std::vector<std::thread> t;


int main( int argc, char *argv[]) {

	if(argc==1){
		num_threads = std::thread::hardware_concurrency(); // No. of threads = No. of cores
	}
	else num_threads = atoi(argv[1]);
	if (num_threads == 0){
		num_threads = 1;
	}

	//These lines turn the number of threads to the maximum number of machine cores but taking into account the number of threads pointed by the user. It is possible that the number of threads launched do not match
	//with the array of steps, then it breaks. It can be commented if the user want to control it by himself. When commented, start the thread loop from 0 instead of from 1!!.

	/*TRACSsim.resize(num_threads);
	t.resize(num_threads);
	t[0] = std::thread(call_from_thread, 0);
	t[0].join();*/


	TRACSsim.resize(num_threads);
	t.resize(num_threads);
	for (int i = 0; i < num_threads; ++i) {
		t[i] = std::thread(call_from_thread, i);
	}
	for (int i = 0; i < num_threads; ++i) {
		t[i].join();
	}

	//write output to single file!
	TRACSsim[0]->write_to_file(0);

	//Results showed for diffusion
	int crosses = 0;
	for (int i = 0; i < num_threads; i++){
		crosses+=TRACSsim[i]->GettotalCrosses();
	}
	std::cout << "Total particles crossed to Depleted Region: " << crosses << std::endl;
	//End results due to diffusion

	//Neff show
	std::vector<double> neff_test = TRACSsim[0]->get_NeffParam();
	std::cout << "Neff param.: " << std::endl;
	for (int i = 0; i < 8; i++)
	{
		std::cout << neff_test[i] << std::endl;
	}
	//End Neff show

	//Clean
	for (uint i = 0; i < TRACSsim.size(); i++)	{
		delete TRACSsim[i];
	}

	std::quick_exit(1);
}

//-----------
/**
 *
 * @param par
 * @return
 */
Double_t TRACSFit::operator() ( const std::vector<Double_t>& par  ) const {

	return 0;
}
