#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <thread>

//#include "libreriaPi.h"
#include <TRACSInterface.h>
#include <threading.h>
//#include <global.h>

std::vector<TRACSInterface*> TRACSsim;
std::vector<std::thread> t;
//std::vector<TRACSInterface*> TRACSsim;

int main(int argc, char *argv[])
{
	num_threads = 2;
	TRACSsim.resize(num_threads);
	t.resize(num_threads);

	for (int i = 0; i < num_threads; ++i) {
		t[i] = std::thread(call_from_thread, i);
	}

	for (int i = 0; i < num_threads; ++i) {
		t[i].join();
	}

	std::vector<double> neff_test = TRACSsim[0]->get_NeffParam();
	std::cout << "Neff param.: " << std::endl;
	for (int i = 0; i < 8; i++)
	{
		std::cout << neff_test[i] << std::endl;
	}

	TRACSsim[0]->write_to_file(0);

	for (int i = 0; i < TRACSsim.size(); i++)
	{
		delete TRACSsim[i];
	}


	printf("Hola\n");
	return 0;
}

