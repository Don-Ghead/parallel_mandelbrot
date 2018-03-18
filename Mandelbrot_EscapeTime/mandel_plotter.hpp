#pragma once

#ifndef _MANDEL_PLOTTER_HPP
#define _MANDEL_PLOTTER_HPP

#include <complex>
#include <functional>
#include <stdbool.h>
#include <vector>

#include "window.hpp"
#include "mandel_logger.hpp"

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

	//To reduce the number of calls we need to make to getters & setters
	//during fractal generation.
	double m_fractal_width;
	double m_fractal_height;
	double m_fractal_y_min;
	double m_fractal_y_max;
	double m_fractal_x_min;
	double m_fractal_x_max;

	int m_screen_width;
	int m_screen_height;
	int m_screen_y_min;
	int m_screen_y_max;
	int m_screen_x_min;
	int m_screen_x_max;

	//This will hold the chosen mandelbrot function 
	const std::function<Complex(Complex, Complex)> m_mandel_func;

	mandel_logger* m_logger;

public:

	mandel_plotter(	window<int> screen, 
					window<double> fractal, 
					int iter_max, 
					const std::function<Complex(Complex, Complex)> &mandel_func,
					mandel_logger* logger);

	~mandel_plotter();

	//Utility

	//Core

	Complex pixel_to_complex(Complex complex);

	Complex pixel_to_complex(unsigned int x, unsigned int y);

	int check_value_within_set(Complex c);

	void get_number_iterations(std::vector<int> &colours, bool use_parallel);

	void fractal(std::vector<int> &colours, bool use_parallel);
};

/*

	MANDEL PLOTTER PRESET VALUES FOR ZOOM UNTIL BETTER METHOD DETERMINED

	//First attempt
	double min_real = 0.3575, max_real = 0.3585;
	double min_imaginary = 0.11; 



*/


#endif
