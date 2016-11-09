/*
 * threading.cpp
 *
 *  Created on: Oct 11, 2016
 *      Author: jcalvopi
 */

#include <threading.h>
#include <global.h>
#include <TRACSInterface.h>

extern std::vector<TRACSInterface*> TRACSsim;
//This function will be called from a thread
void call_from_thread(int tid) {
	// every thread instantiates a new TRACSInterface object

	mtx.lock();
	std::cout << "Thread with tid " << tid << " is INSIDE the critical section "<< std::endl;
	TRACSsim[tid] = new TRACSInterface(fnm);
	TRACSsim[tid]->set_tcount(tid);
	if(tid==0)
	{
		i_ramo_array.clear();
		TRACSsim[tid]->resize_array();
		TRACSsim[tid]->write_header(tid);
		TRACSsim.resize(num_threads);
	}
	std::cout << "Thread with tid " << tid << " is OUTSIDE the critical section "<< std::endl;
	mtx.unlock();

	std::cout << "Thread with tid " << tid << " simulating ramo current - drifting "<< std::endl;
	TRACSsim[tid]->loop_on(tid);

}

void call_from_thread_NoLoop(int tid, const std::vector<Double_t>& par) {
	// every thread instantiates a new TRACSInterface object
	//extern std::vector<TRACSInterface*> TRACSsim;

	mtx.lock();
	std::cout << "Thread with tid " << tid << " is INSIDE the critical section "<< std::endl;
	TRACSsim[tid] = new TRACSInterface(fnm);
	TRACSsim[tid]->set_tcount(tid);
	if(tid==0)
	{
		i_ramo_array.clear();
		TRACSsim[tid]->resize_array();
		TRACSsim[tid]->write_header(tid);
		TRACSsim.resize(num_threads);
	}
	std::cout << "Thread with tid " << tid << " is OUTSIDE the critical section "<< std::endl;
	mtx.unlock();
	TRACSsim[tid]->set_NeffParam(par);
	TRACSsim[tid]->loop_on(tid);


}
