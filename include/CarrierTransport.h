#ifndef CARRIERTRANSPORT_H
#define CARRIERTRANSPORT_H


#include <dolfin.h>
#include <TRandom3.h>
#include <CarrierMobility.h>
#include <Constants.h>

using namespace dolfin;

class DriftTransport
{
private:
	JacoboniMobility _mu;
	Function * _d_f_grad;
	int _sign;
	int _diffusion;
	double _dt;
	double _temp;
	TRandom3 gRandom;


public:
	DriftTransport(char carrier_type, Function * d_f_grad, double givenT = 253., int difussion = 0, double dt = 300);
	DriftTransport();
	~DriftTransport();
	void operator() ( const std::array< double,2> &x , std::array< double,2> &dxdt , const double /* t */ );



};

#endif // CARRIERTRANSPORT_H
