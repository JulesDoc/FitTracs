/*****This code is property of CERN and IFCA under GPL License. Developed by: Marcos Fernandez, Pablo de Castro, Alvaro Diez, Urban Senica and Julio Calvo.*****/

/************************************CarrierTransport***********************************
 * DriftTransport class defines the drift object used to carry out each carrier step. It defines certain characteristics associated with the carrier, sign, dt, mobility...
 * The operator method is used in the stepper.do_step, ODEINT (Ordinary differential equations to calculate the new x,y coordinates of a particle in the detector mesh in a certain dt.
 *
 */


#include "CarrierTransport.h"
/**
 *
 * @param carrier_type
 * @param d_f_grad
 * @param givenT
 * @param diffusion
 * @param dt
 */
DriftTransport::DriftTransport(char carrier_type, Function * d_f_grad, double givenT, int diffusion, double dt) :
_mu(carrier_type, givenT),
_diffusion(diffusion),
_dt(dt),
_temp(givenT)
{
	_d_f_grad = d_f_grad;
	if (carrier_type == 'e') {
		_sign = -1;
	}
	else {
		_sign = 1;
	}
}
/**
 *
 * @param x
 * @param dxdt
 * @param
 */
void DriftTransport::operator() ( const std::array<double,2>  &x , std::array<double,2>  &dxdt , const double /* t */ )
{
	Array<double> e_field((std::size_t) 2); // temp wrap for e. field
	double e_field_mod;
	//TRandom3 Rand(0);
	Point eval_point(x[0],x[1],0.0);
	(*_d_f_grad)(e_field, eval_point);
	e_field_mod = sqrt(e_field[0]*e_field[0] + e_field[1]*e_field[1]);
	dxdt[0] = _sign*_mu.obtain_mobility(e_field_mod) * e_field[0];
	dxdt[1] = _sign*_mu.obtain_mobility(e_field_mod) * e_field[1];
}


DriftTransport::~DriftTransport()
{

}

DriftTransport::DriftTransport()
{
}
