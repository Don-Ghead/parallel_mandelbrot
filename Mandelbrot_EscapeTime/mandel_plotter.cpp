//add description
//COPIED FROM https://github.com/sol-prog/Mandelbrot_set/blob/master/mandel.cpp
//Refactored for better clarity 

#include "mandel_plotter.hpp"
#include "mandel_logger.hpp"

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


mandel_plotter::mandel_plotter(	window<int> screen,
								window<double> fractal,
								int iter_max,
								const std::function<Complex(Complex, Complex)> &mandel_func,
								mandel_logger* logger)
	:	m_screen(screen), 
		m_fractal(fractal), 
		m_iter_max(iter_max), 
		m_mandel_func(mandel_func),
		m_logger(logger)
{
	m_fractal_width = fractal.width();
	m_fractal_height = fractal.height();
	m_fractal_min_imaginary = fractal.get_y_min();
	m_fractal_max_imaginary = fractal.get_y_max();
	m_fractal_min_real = fractal.get_x_min();
	m_fractal_max_real = fractal.get_x_max();

	m_screen_width = screen.width();
	m_screen_height = screen.height();
	m_screen_y_min = screen.get_y_min();
	m_screen_y_max = screen.get_y_max();
	m_screen_x_min = screen.get_x_min();
	m_screen_x_max = screen.get_x_max();

	//So in this instance x is the real part, and y is the imaginary part of the fractal boundary
	//So y_min = minimum imaginary , x_max = maximum real
	//We calculate y_max based on the dimensions to make sure the image does not skew
	m_max_imaginary = m_fractal_min_imaginary + (m_fractal_max_real - m_fractal_min_real) * m_screen_height / m_screen_width;
	m_real_factor = (m_fractal_max_real - m_fractal_min_real) / (m_screen_width - 1);
	m_imaginary_factor = (m_fractal_max_imaginary - m_fractal_min_imaginary) / (m_screen_height - 1);
}

mandel_plotter::~mandel_plotter()
{
}

// Convert a pixel coordinate to the complex domain using a complex of the form Complex(x,y)
Complex mandel_plotter::pixel_to_complex(Complex c) {
	Complex aux(c.real() / (double)m_screen_width * m_fractal_width + m_fractal_min_real,
		c.imag() / (double)m_screen_height * m_fractal_height + m_fractal_min_imaginary);
	return aux;
}

// Convert a pixel coordinate to the complex domain using the x,y coordinates
Complex mandel_plotter::pixel_to_complex(unsigned int x, unsigned int y)
{
	//So in this instance x is the real part, and y is the imaginary part of the fractal boundary
	//double min_real = 0.3575, max_real = 0.3585;
	//double min_imaginary = 0.11;
	//double max_imaginary = min_imaginary + (max_real - min_real) * m_screen_height / m_screen_width;
	
	Complex aux(m_fractal_min_real + x * m_real_factor, m_fractal_max_imaginary - y * m_imaginary_factor);
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
void mandel_plotter::get_number_iterations(std::vector<int> &colours, bool use_parallel)
{
	int colour_index = 0;
	if (!use_parallel)
	{
		cout << "Using sequential Mandelbrot" << endl;
		for (int i = m_screen_y_min; i < m_screen_y_max; ++i) 
		{
			for (int j = m_screen_x_min; j < m_screen_x_max; ++j) 
			{

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
	else
	{
		cout << "Using OpenMP parallelised Mandelbrot" << endl;
		//Unfortunately OpenMP version on Visual studio doesn't support the collapse clause 
		//So check at uni but using workaround for now

#pragma omp parallel private(colour_index)
//#pragma omp parallel for private(colour_index) collapse(2)
		for (int y = m_screen_y_min; y < m_screen_y_max; ++y) 
		{
#pragma omp parallel for schedule(dynamic, 1)
			for (int x = m_screen_x_min; x < m_screen_x_max; ++x) 
			{
				//for Row-major ordering the offset is calculated as (row * NumColumns )+ column
				colour_index = y * m_screen_x_max + x;
				//Complex c((double)x, (double)y); //Assign two coordinate positions to complex
				Complex c = pixel_to_complex(x, y); //convert to complex domain

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
}

//Can definitely expand the performance testing & analysis in here once working as intended.
void mandel_plotter::fractal(std::vector<int> &colours, bool use_parallel) 
{
	//May re-enable the progress bar for larger fractal computations
	cout << "Computing Mandelbrot Fractals please wait..." << endl;
	auto start = std::chrono::steady_clock::now();
	get_number_iterations(colours, use_parallel);
	auto end = std::chrono::steady_clock::now();

	//Now we add some basic details to the logfile 
	if (!use_parallel)
	{
		m_logger->add_logfile_detail("Sequential");

	}
	else
	{
		m_logger->add_logfile_detail("Parallelized: OpenMP");
	}
	m_logger->add_logfile_detail("Image Dimensions: " + to_string(m_screen_width) + "," + to_string(m_screen_height));

	//TODO - This may not work check logfile to double check
	double temp_duration = std::chrono::duration <double, std::milli>(end - start).count();
	m_logger->add_logfile_detail("Time to generate fractals: " + to_string( temp_duration ) + " [ms]");
	std::cout << "Time to generate fractals: " << std::chrono::duration <double, std::milli>(end - start).count() << " [ms]" << std::endl;
}


