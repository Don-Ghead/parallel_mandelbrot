#include "image_handler.hpp"
#include "mandel_plotter.hpp"
#include <iostream>

#if defined (__unix__)
#include <mpi.h>
#endif

using namespace std;
// Use an alias to simplify the use of complex type

//Filepath stuff
#if defined(__unix__)
const string default_image_filepath("../resources/mandelbrot/");
#elif defined(_WIN32) || defined(WIN32)
const string default_image_filepath("..\\resources\\mandelbrot\\");
#endif
const string default_image_filename("mandel.bmp");

bool get_userdefined_params(int &width, int &height, int &max_iter, parallelisation_type &parallel_type)
{
	int temp;
	cout << "Define image width (x): ";
	cin >> width;
	cout << endl << "Define image height(y): ";
	cin >> height;
	cout << endl << "Define the max number of iterations: ";
	cin >> max_iter;
	cout << endl << "Define parallelisation type: " << endl;
	cout << "\t 0: no parallelisation\n" << \
		"\t	1: OpenMP parallelisation\n" << \
		"\t 2: MPI parallelisation\n" << \
		"\t 3: OpenMP & MPI parallelisation" << endl;  \
	cin >> temp;
	cout << endl;

	if (0 == temp)
	{
		parallel_type = NO_PARALLEL;
	} 
	else if (1 == temp)
	{
		parallel_type = OMP_PARALLEL;
	}
	else
	{
		parallel_type= MPI_PARALLEL;
	}

	if ((width > 0) && (height > 0) && (max_iter > 0) )
		return true;
	else
		return false;
}

int main(int argc, char **argv)
{


	/**************************************
		Initial Parameter declaration
	***************************************/

	/*
	Store our first and third order mandelbrot lamba's inside a std::function for readability
	*/
	using Complex = std::complex<double>;
	std::function<Complex(Complex, Complex)> first_order_mandel = [](Complex z, Complex c) -> Complex {return z * z + c; };
	std::function<Complex(Complex, Complex)> third_order_mandel = [](Complex z, Complex c) -> Complex {return z * z * z + c; };

	int width, height, max_iter;
	parallelisation_type parallel_type = NO_PARALLEL;
	int testmode;

	/**************************************
				Test Mode Config
	***************************************/

	cout <<		"Use parallel test mode? 0 for no," <<
		endl << "1 for 1k x 1k @ 500 iterations," << endl;
	cout <<		"2 for 2k x 2k @ 600 iterations," <<
		endl << "3 for 3k x 3k @ 700 iterations," << endl;
	cout <<		"4 for 4k resolution image @ 800 iterations," <<
		endl << "8 for 8k resolution image @ 800 iterations" << endl;
	cin >> testmode;
	switch (testmode)
	{
	case 0:
		if (!get_userdefined_params(width, height, max_iter, parallel_type))
		{
			cout << "User defined paramaters are not valid, exiting" << endl;
			return 0;
		}
		break;

	case 1:
		width = 1000;
		height = 1000;
		max_iter = 500;
		break;

	case 2:
		width = 2000;
		height = 2000;
		max_iter = 600;
		break;

	case 3:
		width = 3000;
		height = 3000;
		max_iter = 700;
		break;

	case 4:
		width = 3840;
		height = 2160;
		max_iter = 800;
		break;

	default:
		cout << "invalid test mode provided, reverting to case 1" << endl;
		width = 1000;
		height = 1000;
		max_iter = 500;
		break;
	}


	/**************************************
					Core
	***************************************/

	//image dimensions
	window<int> screen(0, width, 0, height);

	//Fourth value doesn't matter for fractal as it is calculated based on other values
	//window<double> fractal(-2.2, 1.2, -1.7, 1.7); //Zoomed out preset
	//double max_imag = 0.1 + (0.385-0.375) * height / width;
	window<double> fractal(0.3575, 0.3585, 0.11, 0); //Zoomed in preset

	//Create the mandel_logger - Don't care about alternate logfile for now
	mandel_logger logger(Log_level::DEFAULT);

#if defined (__unix__)
	MPI_Init(&argc, &argv);
#endif

	//Now create the plotter using the parameters specified above
	mandel_plotter plotter(screen, fractal, max_iter, first_order_mandel, &logger);

	//This will be the vector that will contain the iterations for each pixel point.
	//Doing it in this way means we can very easily add other polynomials to see how
	//the colours change.
	vector<int> colours(screen.size());

	//Now plot the fractal, for convenience sake this is fairly well wrapped up, however
	//when it comes to performance testing and parallelization there will likely be changes
	//to the underlying way in which it computes these fractals.
	plotter.fractal(colours, parallel_type);

#if defined (__unix__)
	MPI_Finalize();
#endif

	string new_image_filepath, new_image_filename;
	if (0 == testmode)
	{
		cout << "Fractal computation complete, please enter absolute path of image including name or enter 'default' to use the default values: ";
		cin >> new_image_filepath;
		if ("default" == new_image_filepath)
		{
			cout << "Using default path" << endl;
			new_image_filename = default_image_filename;
			new_image_filepath = default_image_filepath;
		}
		else
		{
			//Get the filename out of the path
			size_t found;
#if defined(__unix__)
			found = new_image_filepath.find_last_of("/");
#elif defined(_WIN32) || defined(WIN32)
			found = new_image_filepath.find_last_of("\\");
#endif
			if (0 < found)
			{
				new_image_filename = new_image_filepath.substr(found + 1);
				new_image_filepath = new_image_filepath.substr(0, found);
			}
			cout << "Writing image to: " << new_image_filepath << endl;
		}
	}
	else
	{
		cout << "Writing to default path" << endl;
		new_image_filename = default_image_filename;
		new_image_filepath = default_image_filepath;
	}
	//Finally create the image handler which will convert the iterations in the Colours
	//Vector to RGB and write the image to the filepath provided.
	image_handler img_hand((new_image_filepath + new_image_filename),
							max_iter,
							screen.width(),
							screen.height());

	img_hand.write_image(screen, colours);

	return 0;
}
