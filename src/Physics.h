#pragma once
#include "allIncludes.h"
#include "Ray.h"

void GeoDesic(Ray& ray, double eventHorizonRadius) 
{
	double& r = ray.r;
	double& phi = ray.phi;
	double& dr = ray.dr;
	double& dphi = ray.dphi;
	double& d2r = ray.d2r;
	double& d2phi = ray.d2phi;


	d2phi = (1 / r) * dr * dphi;
	d2r = (-c * c * eventHorizonRadius) / (2 * r * r) + r * dphi * dphi;

	////rate of change of the distance from the blackhole
	//dr += r * dphi * dphi - (c * c * eventHorizonRadius) / (2 * r * r);
	////rate of change of the angle relative to the blackhole
	//dphi = -2 * dr * dphi / r;	
}

void RK4Step(Ray& ray, double dλ, double eventHorizonRadius) 
{

}
