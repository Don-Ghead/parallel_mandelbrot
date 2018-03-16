//add description
//COPIED FROM https://github.com/sol-prog/Mandelbrot_set/blob/master/mandel.cpp
//Refactored for better clarity 

#include "mandel_plotter.hpp"

#include <tuple>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <omp.h>

#define DEFAULT_MAX_ITERATIONS 800

#define MAX_COLOURS_PER_ELEMENT 256
#define MAX_COLOURS_RGB	16777216  // 256^3 

using namespace std;


mandel_plotter::mandel_plotter(window<int> screen, window<double> fractal, int iter_max, const std::function<Complex(Complex, Complex)>& mandel_func)
	: m_screen(screen), m_fractal(fractal), m_iter_max(iter_max), m_mandel_func(mandel_func)
{
}

mandel_plotter::~mandel_plotter()
{
}


// Convert a pixel coordinate to the complex domain
Complex mandel_plotter::pixel_to_complex(Complex complex) {
	Complex aux(complex.real() / (double)m_screen.width() * m_fractal.width() + m_fractal.get_x_min(),
		complex.imag() / (double)m_screen.height() * m_fractal.height() + m_fractal.get_y_min());
	return aux;
}

// Convert a pixel coordinate to the complex domain
Complex mandel_plotter::pixel_to_complex2(unsigned int x, unsigned int y)
{
	double min_real = 0.3575, max_real = 0.3580;
	double min_imaginary = 0.111111111111;
	double max_imaginary = min_imaginary + (max_real - min_real) * m_screen.height() / m_screen.width();
	double real_factor = (max_real - min_real) / (m_screen.width() - 1);
	double imaginary_factor = (max_imaginary - min_imaginary) / (m_screen.height() - 1);
	Complex aux(min_real + x * real_factor, max_imaginary - y * imaginary_factor);
	return aux;
}

// Check if a point is in the set or escapes to infinity, return the number of iterations
int mandel_plotter::check_value_within_set(Complex c) {
	Complex z(c);
	int iter = 0;

	//This is where we apply the desired mandelbrot function and check the 
	//abs(solute) value of our complex to ensure it is still within range
	while (abs(z) < 2.0 && iter < m_iter_max) {
		z = m_mandel_func(z, c);
		iter++;
	}

	return iter;
}

// Loop over each pixel from our image and check if the points associated with this pixel escape to infinity
void mandel_plotter::get_number_iterations(std::vector<int> &colours) {
	int colour_index = 0, progress = -1;
	for (int i = m_screen.get_y_min(); i < m_screen.get_y_max(); ++i) {
		for (int j = m_screen.get_x_min(); j < m_screen.get_x_max(); ++j) {

			Complex c((double)j, (double)i); //Assign real(j) and imaginary(i) coord positions
			c = pixel_to_complex(c); //convert to complex domain

			//returns the number of iterations of our complex C 
			//and assigns it to the appropriate colours index
			colours[colour_index] = check_value_within_set(c);  
			++colour_index;
		}
		/* May Reenable this given particular fractal parameters
		if (progress < (int)(i*100.0 / m_screen.get_y_max())) {
			progress = (int)(i*100.0 / m_screen.get_y_max());
			std::cout << progress << "%\n";
		}
		*/
	}
}

// Loop over each pixel from our image and check if the points associated with this pixel escape to infinity
void mandel_plotter::get_parallel_number_iterations(std::vector<int> &colours) {
	//Unfortunately OpenMP version on Visual studio doesn't support the collapse clause 
	//So check at uni but using workaround for now
	int colour_index = 0;
#pragma omp parallel private(colour_index)
	for (int y = m_screen.get_y_min(); y < m_screen.get_y_max(); ++y) {
#pragma omp parallel for schedule(dynamic,1) 
		for (int x = m_screen.get_x_min(); x < m_screen.get_x_max(); ++x) {

			//I think for Row-major ordering the offset is calculated as (row * NumColumns )+ column
			colour_index = y * m_screen.get_x_max() + x; 
			//Complex c((double)x, (double)y); //Assign two coordinate positions to complex
			Complex c = pixel_to_complex2(x, y); //convert to complex domain

			//returns the number of iterations of our complex C 
			//and assigns it to the appropriate colours index
			colours[colour_index] = check_value_within_set(c);
		}
		/* May Reenable this given particular fractal parameters
		if (progress < (int)(i*100.0 / m_screen.get_y_max())) {
		progress = (int)(i*100.0 / m_screen.get_y_max());
		std::cout << progress << "%\n";
		}
		*/
	}
}

//Can definitely expand the performance testing & analysis in here once working as intended.
void mandel_plotter::fractal(std::vector<int> &colours, bool use_parallel) {
	
	auto start = std::chrono::steady_clock::now();
	if (!use_parallel)
	{
		cout << "Using sequential fractal generation" << endl;
		get_number_iterations(colours);
	}
	else
	{
		cout << "Using parallel OpenMP for fractal generation" << endl;
		get_parallel_number_iterations(colours);
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << "Time to generate fractals= " << std::chrono::duration <double, std::milli>(end - start).count() << " [ms]" << std::endl;
}


