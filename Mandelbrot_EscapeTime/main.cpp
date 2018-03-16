#include "image_handler.hpp"
#include "mandel_plotter.hpp"
#include <iostream>


using namespace std;
// Use an alias to simplify the use of complex type

bool get_userdefined_params(int &width, int &height, int &max_iter, bool &use_parallel)
{
	int temp;
	cout << "Define image width (x): ";
	cin >> width;
	cout << endl << "Define image height(y): ";
	cin >> height;
	cout << endl << "Define the max number of iterations: ";
	cin >> max_iter;
	cout << endl << "Use OpenMP For parallelization? 0 for no, 1 for yes: ";
	cin >> temp;
	cout << endl;

	if (0 == temp) 
	{
		use_parallel = false;
	} 
	else if (1 == temp)
	{
		use_parallel = true;
	}
	else
	{
		use_parallel = false;
	}

	if ((width > 0) && (height > 0) && (max_iter > 0) )
		return true;
	else
		return false;
}

int main(void)
{
	/*
	Store our first and third order mandelbrot lamba's inside a std::function for simplicity
	*/
	using Complex = std::complex<double>;
	std::function<Complex(Complex, Complex)> first_order_mandel = [](Complex z, Complex c) -> Complex {return z * z + c; };
	std::function<Complex(Complex, Complex)> third_order_mandel = [](Complex z, Complex c) -> Complex {return z * z * z + c; };

	//Filepath stuff
	string default_image_filepath("..\\resources\\mandelbrot\\"), new_image_filepath;
	string default_image_filename("mandel.bmp"), new_image_filename;

	//Define the image parameters
	//window<double> fractal(-.2, .1, -.12, 0.12);
	int width, height, max_iter;
	bool use_parallel = false;
	int testmode;
	cout << "Test mode? 0 for no, 1 for yes: ";
	cin >> testmode;
	if (1 == testmode)
	{
		width = 1000; 
		height = 1000; 
		max_iter = 300; 
		use_parallel = true;
	}
	else
	{
		if (!get_userdefined_params(width, height, max_iter, use_parallel))
		{
			cout << "User defined paramaters are not valid, exiting" << endl;
			return 0;
		}
	}
	window<int> screen(0, width, 0, height);
	
	//May re-enable the progress bar for larger fractal computations
	cout << "Computing Mandelbrot Fractals please wait..." << endl;

	//Fractal parameters - Calculating max_imaginary this way means that the image doesn't
	//stretch given particular screen dimensions.
	double min_real = -2.0, max_real = 1.0;
	double min_imaginary = -1.2;
	double max_imaginary = min_imaginary + (max_real - min_real) * screen.height() / screen.width();
	window<double> fractal(min_real, max_real, min_imaginary , max_imaginary);

	//Now create the plotter using the parameters specified above
	mandel_plotter plotter(screen, fractal, max_iter , first_order_mandel);

	//This will be the vector that will contain the iterations for each pixel point.
	//Doing it in this way means we can very easily add other polynomials to see how
	//the colours change.
	vector<int> colours(screen.size());

	//Now plot the fractal, for convenience sake this is fairly well wrapped up, however
	//when it comes to performance testing and parallelization there will likely be changes
	//to the underlying way in which it computes these fractals.
	plotter.fractal(colours, use_parallel);

	cout << "Fractal computation complete, please enter absolute path of image including name or enter 'default' to use the default values: ";
	cin >> new_image_filepath;
	if ("default" == new_image_filepath )
	{
		cout << "Using default path" << endl;
		new_image_filename = default_image_filename;
		new_image_filepath = default_image_filepath;
	}
	else
	{
		//Get the filename out of the path 
		size_t found = new_image_filepath.find_last_of("\\");
		if (found > 0)
		{
			new_image_filename = new_image_filepath.substr(found+1);
			new_image_filepath = new_image_filepath.substr(0, found);
		}
		cout << "Writing image to: " << new_image_filepath << endl;
	}
	image_handler img_hand((new_image_filepath + new_image_filename),
		max_iter,
		screen.width(),
		screen.height());

	img_hand.write_image(screen, colours);

	return 0;
}