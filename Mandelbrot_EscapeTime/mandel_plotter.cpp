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

#if defined(__unix__)
#include <mpi.h>
#endif

#define DEFAULT_MAX_ITERATIONS 800

#define MAX_COLOURS_PER_ELEMENT 256
#define MAX_COLOURS_RGB	16777216  // 256^3 

using namespace std;
using std::cout;

mandel_plotter::mandel_plotter(	window<int> screen,
								window<double> fractal,
								int iter_max,
								const std::function<Complex(Complex, Complex)> &mandel_func,
								mandel_logger* logger)
	:	m_iter_max(iter_max), 
		m_mandel_func(mandel_func),
		m_logger(logger)
{
	int rank = -1;
	int mpi_size = 0;

#if defined(__unix__)	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
#endif
	m_mpi_rank = rank;
	m_mpi_size = mpi_size;

	m_screen_width = screen.width();
	m_screen_height = screen.height();
	m_screen_y_min = screen.get_y_min();
	m_screen_y_max = screen.get_y_max();
	m_screen_x_min = screen.get_x_min();
	m_screen_x_max = screen.get_x_max();
		
	m_fractal_min_real = fractal.get_x_min();
	m_fractal_max_real = fractal.get_x_max();
	m_fractal_min_imaginary = fractal.get_y_min();
	m_fractal_max_imaginary = m_fractal_min_imaginary + (m_fractal_max_real - m_fractal_min_real) * m_screen_height / m_screen_width;
	m_fractal_width = m_fractal_max_real - m_fractal_min_real;
	m_fractal_height = m_fractal_max_imaginary - m_fractal_min_imaginary;

	//So in this instance x is the real part, and y is the imaginary part of the fractal boundary
	//So y_min = minimum imaginary , x_max = maximum real
	//We calculate y_max based on the dimensions to make sure the image does not skew
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
void mandel_plotter::get_number_iterations(std::vector<int> &colours, parallelisation_type parallel_type)
{
	int colour_index = 0;
	if (NO_PARALLEL == parallel_type)
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
	else if( OMP_PARALLEL == parallel_type)
	{
		cout << "Using OpenMP parallelised Mandelbrot" << endl;
		//Unfortunately OpenMP version on Visual studio doesn't support the collapse clause 
		//So check at uni but using workaround for now

		omp_set_num_threads(16);
#pragma omp parallel for private(colour_index) schedule(dynamic, 1)
		for (int y = m_screen_y_min; y < m_screen_y_max; ++y) 
		{
			//cout << "Number of threads = " << omp_get_num_threads() << endl;
			for (int x = m_screen_x_min; x < m_screen_x_max; ++x)
			{
				//for Row-major ordering the offset is calculated as (row * NumColumns )+ column
				colour_index = y * m_screen_width + x;
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
	else if (MPI_PARALLEL == parallel_type)
	{
		//Divide size of colours vector by number of MPI instances to get 
		//Boundaries for each instance of MPI program to work on 
		int mpincrement = 0;
		int index = 0;

		mpincrement = colours.size() / m_mpi_size;
		int *mpi_colours = new int[mpincrement];

		//Output the buffer boundaries during testing (this could also go to logger)
		cout << "Size of entire buffer: " << m_screen_height * m_screen_width << endl;
		cout << "Increment size of buffer: " << mpincrement << endl;

		//define the boundary based on the MPI instance rank
		//bounds low = x * S/N
		//bounds high = ((x+1) * S/N) - 1 
		size_t buf_bounds_low = m_mpi_rank * mpincrement;
		size_t buf_bounds_high = ((m_mpi_rank + 1) * mpincrement) - 1;

		cout << "Rank: " << m_mpi_rank << " Using MPI only Mandelbrot" << endl;
		cout << "Lower boundary: " << buf_bounds_low << " , Upper boundary: " << buf_bounds_high << endl;

		for (int i = buf_bounds_low; i < buf_bounds_high; ++i)
		{
			int x = i % m_screen_x_max;
			int y = (i - x) / m_screen_x_max;

			Complex c = pixel_to_complex(x, y); //convert to complex domain

												//returns the number of iterations of our complex C
												//and assigns it to the appropriate colours index
			mpi_colours[index] = check_value_within_set(c);
			index++;
		}

		//Now Either Send or receive data to combine into master data container
		if (0 != m_mpi_rank) //If not master send data to master
		{
#if defined(__unix__)

			MPI_Send(&mpi_colours[0],	//Buffer 
				mpincrement,				//Amount of data to send
				MPI_INT,			 		//data type
				0,							//Destination (master)
				0,							//Tag (don't need one)
				MPI_COMM_WORLD);
			
			cout << "Send call from rank " << m_mpi_rank << endl;
#endif
		}
		else //Master, receive data and allocate to colours vector
		{
#if defined(__unix__)
			index = 0;
			for (int b = buf_bounds_low; b < buf_bounds_high; b++)
			{
				colours[b] = mpi_colours[index];
				index++;
			}

			for (int t = 1; t < m_mpi_size; t++)
			{ 
				MPI_Status status;
				size_t rcv_rank;

				MPI_Recv(&mpi_colours[0],		//buffer
					mpincrement,				//Amount of data to recv
					MPI_INT,					//Data type
					MPI_ANY_SOURCE,				//source
					0,							//Tag
					MPI_COMM_WORLD,				//Comm
					&status);					//status

				rcv_rank = status.MPI_SOURCE;
				cout << "Master Receive call from rank " << rcv_rank << endl;

				//Get boundaries for sender 
				size_t rcvbuf_bounds_low = rcv_rank * mpincrement;
				size_t rcvbuf_bounds_high = ((rcv_rank + 1) * mpincrement) - 1;

				int index = 0;
				//Now copy this to the main colours vector
				for (int b = rcvbuf_bounds_low; b < rcvbuf_bounds_high; b++)
				{
					colours[b] = mpi_colours[index];
					index++;
				}
			}
#endif
		}
		
	}
	else if (BOTH_PARALLEL == parallel_type)
	{
		//Divide size of colours vector by number of MPI instances to get 
		//Boundaries for each instance of MPI program to work on 
		int mpincrement = 0;
		int mpi_comm_size = 0;
		int index = 0;

#if defined(__unix__)
		MPI_Comm_size(MPI_COMM_WORLD, &mpi_comm_size);
#endif

		mpincrement = colours.size() / mpi_comm_size;
		int *mpi_colours = new int[mpincrement];

		//Output the buffer boundaries during testing (this could also go to logger)
		cout << "Size of entire buffer: " << m_screen_height * m_screen_width << endl;
		cout << "Increment size of buffer: " << mpincrement << endl;

		//define the boundary based on the MPI instance rank
		//bounds low = x * S/N
		//bounds high = ((x+1) * S/N) - 1
		size_t buf_bounds_low = m_mpi_rank * mpincrement;
		size_t buf_bounds_high = ((m_mpi_rank + 1) * mpincrement) - 1;

		cout << "Rank: " << m_mpi_rank << " Using MPI only Mandelbrot" << endl;
		cout << "Lower boundary: " << buf_bounds_low << " , Upper boundary: " << buf_bounds_high << endl;

#pragma omp parallel for private(index)
		for (int i = buf_bounds_low; i < buf_bounds_high; ++i)
		{
			int x = i % m_screen_x_max;
			int y = (i - x) / m_screen_x_max;

			Complex c = pixel_to_complex(x, y); //convert to complex domain

												//returns the number of iterations of our complex C
												//and assigns it to the appropriate colours index
			mpi_colours[index] = check_value_within_set(c);
			index++;
		}

		//Now Either Send or receive data to combine into master data container
		if (0 != m_mpi_rank) //If not master send data to master
		{
#if defined(__unix__)
			MPI_Send(&mpi_colours[0],	//Buffer 
				mpincrement,				//Amount of data to send
				MPI_INT,			 		//data type
				0,							//Destination (master)
				0,							//Tag (don't need one)
				MPI_COMM_WORLD);

			cout << "Send call from rank " << m_mpi_rank << endl;
#endif
		}
		else //Master, receive data and allocate to colours vector
		{
#if defined(__unix__)
			index = 0;
			for (int b = buf_bounds_low; b < buf_bounds_high; b++)
			{
				colours[b] = mpi_colours[index];
				index++;
			}

			for (int t = 1; t < mpi_comm_size; t++)
			{
				MPI_Status status;
				size_t rcv_rank;

				MPI_Recv(&mpi_colours[0],		//buffer
					mpincrement,				//Amount of data to recv
					MPI_INT,					//Data type
					MPI_ANY_SOURCE,				//source
					0,							//Tag
					MPI_COMM_WORLD,				//Comm
					&status);					//status

				rcv_rank = status.MPI_SOURCE;
				cout << "Master Receive call from rank " << rcv_rank << endl;

				//Get boundaries for sender 
				size_t rcvbuf_bounds_low = rcv_rank * mpincrement;
				size_t rcvbuf_bounds_high = ((rcv_rank + 1) * mpincrement) - 1;

				int index = 0;
				//Now copy this to the main colours vector
				for (int b = rcvbuf_bounds_low; b < rcvbuf_bounds_high; b++)
				{
					colours[b] = mpi_colours[index];
					index++;
				}
			}
#endif
		}
	}
}

//Can definitely expand the performance testing & analysis in here once working as intended.
void mandel_plotter::fractal(std::vector<int> &colours, parallelisation_type parallel_type) 
{
	//May re-enable the progress bar for larger fractal computations
	cout << "Computing Mandelbrot Fractals please wait..." << endl;
	double start = omp_get_wtime();
	get_number_iterations(colours, parallel_type);
	double end = omp_get_wtime();

	/*Now we add some basic details to the logfile 
	switch(parallel_type)
	{
	case NO_PARALLEL:
		m_logger->add_logfile_detail("Sequential");
		break;
	
	case OMP_PARALLEL:
		m_logger->add_logfile_detail("Parallelized: OpenMP");
		break;

	case MPI_PARALLEL:
		m_logger->add_logfile_detail("Parallelized: MPI");
		break;

	case BOTH_PARALLEL:
		m_logger->add_logfile_detail("Parallelized OpenMP & MPI");
		break;
	} 

	m_logger->add_logfile_detail("Image Dimensions: [" + to_string(m_screen_width) + "," + to_string(m_screen_height) + ']');
	*/
	double temp_duration;
	double total_duration;
#if defined(__unix__)
	MPI_Status status;

		if (0 != m_mpi_rank) //Send temp values to master
		{
			temp_duration = std::chrono::duration <double, std::milli>(end - start).count();
			
			MPI_Send(&temp_duration,		//Buffer 
				1,				//Amount of data to send
				MPI_DOUBLE,			 		//data type
				0,							//Destination (master)
				1,							//Tag (don't need one)
				MPI_COMM_WORLD);			
		}
		else
		{
			temp_duration = std::chrono::duration <double, std::milli>(end - start).count();
			total_duration += temp_duration;

			for (int t = 1; t < m_mpi_size; t++)
			{
				MPI_Recv(&temp_duration,		//buffer
					1,				//Amount of data to recv
					MPI_DOUBLE,					//Data type
					MPI_ANY_SOURCE,				//source
					1,							//Tag
					MPI_COMM_WORLD,				//Comm
					&status);					//status

				total_duration += temp_duration;
			}

			m_logger->add_logfile_detail(to_string(total_duration) + ' ');
			m_logger->write_logdetails_to_path();
			std::cout << "Total time to generate fractals: " << total_duration << " [s]" << std::endl;
		}
	
#endif
		
}


