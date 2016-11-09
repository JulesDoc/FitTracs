//#include "TracsMaster/src/global.h"

#include <mutex>
#include <TRACSInterface.h>

std::valarray<std::valarray <double> > vItotals;
vector<vector <TH1D*> >  i_ramo_array, i_conv_array, i_rc_array;
std::string fnm="Config.TRACS";
std::mutex mtx;
int num_threads;

