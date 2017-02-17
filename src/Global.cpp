/*****This code is property of CERN and IFCA. Developed by: Marcos Fernandez, Pablo de Castro, Alvaro Diez, Urban Senica and Julio Calvo.*****/

/*Global.cpp is used to define some global variables that are used during the whole execution by the threads in different functions and methods.
 *The steering file and the number of threads are declared here
 */

#include <mutex>
#include <TRACSInterface.h>

//Main variables of TRACS to store the induced current during the whole execution
std::valarray<std::valarray <double> > vItotals;
vector<vector <TH1D*> >  i_ramo_array, i_conv_array, i_rc_array;

//Define here the steering file you want to use. Store it in myApp folder.
std::string fnm="MyConfigTRACS";
//For mutex areas
std::mutex mtx;
int num_threads;


