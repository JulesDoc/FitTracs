#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>
#include <valarray>
#include <mutex>

#include <TH1D.h> // 1 Dimensional ROOT histogram


extern std::vector<std::vector <TH1D*> >  i_ramo_array, i_conv_array, i_rc_array;
extern int num_threads;
extern std::mutex mtx;
extern std::string fnm;
extern std::valarray<std::valarray <double> > vItotals;
//extern std::atomic<double> numberDs;
//extern std::atomic<int> tempNumberDs;
//extern bool printa;

#endif // GLOBAL_H
