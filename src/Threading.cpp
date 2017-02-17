/*****This code is property of CERN and IFCA under GPL License. Developed by: Marcos Fernandez, Pablo de Castro, Alvaro Diez, Urban Senica and Julio Calvo.*****/

/************************************threading************************************
 *
 * Next functions are used to create modularity in TRACS. Call_from_thread functions can be called from any external program, it launches a whole process of
 * TRACS with all the functionalities in there. Used for the fitting purposes where TRACS needs to be executed several time, called by Migrand. The unique difference between
 * both functions showed below, is the inclusion or not of the setting of the Neff parameters, depending on which part of de code we will need to calculate it or just set it.
 *
 *
 */

#include "../include/Threading.h"

#include <TRACSInterface.h>
#include "../include/Global.h"

extern std::vector<TRACSInterface*> TRACSsim;
//This function will be called from a threadx
/**
 *
 * @param tid
 */
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
/**
 *
 * @param tid
 * @param par
 */
void call_from_thread_NeffPar(int tid, const std::vector<Double_t>& par) {
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
	TRACSsim[tid]->set_NeffParam(par);
	TRACSsim[tid]->loop_on(tid);


}
