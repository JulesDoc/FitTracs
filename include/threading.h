/*
 * threading.h
 *
 *  Created on: Oct 11, 2016
 *      Author: jcalvopi
 */

#ifndef SRC_THREADING_H_
#define SRC_THREADING_H_

#include <vector>
#include <TRACSFit.h>

void call_from_thread(int);
void call_from_thread_NoLoop(int, const std::vector<Double_t>& par);


#endif /* SRC_THREADING_H_ */
