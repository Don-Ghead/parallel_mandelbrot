#pragma once

#ifndef _MANDEL_PLOTTER_HPP
#define _MANDEL_PLOTTER_HPP

#include <complex>
#include <functional>
#include <stdbool.h>
#include <vector>

#include "window.hpp"

// Use an alias to simplify the use of complex type
using Complex = std::complex<double>;

/***************************************************************

BEGIN CLASS::MANDEL_PLOTTER

****************************************************************/

class mandel_plotter
{
	using Complex = std::complex<double>;

private:

	//The specified screen boundary
	window<int> m_screen;

	//The specified fractal boundary
	window<double> m_fractal;

	//Holds the max iterations
	int m_iter_max;

	//This will hold the chosen mandelbrot function 
	const std::function<Complex(Complex, Complex)> m_mandel_func;

public:

	mandel_plotter(window<int> screen, window<double> fractal, int iter_max, const std::function<Complex(Complex, Complex)> &mandel_func);

	~mandel_plotter();

	//Utility

	//Core

	Complex pixel_to_complex(Complex complex);

	int check_value_within_set(Complex c);

	void get_number_iterations(std::vector<int> &colours);

	void fractal(std::vector<int> &colours);
};




#endif
