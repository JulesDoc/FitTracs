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
