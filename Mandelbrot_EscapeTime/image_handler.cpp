#include "image_handler.hpp"
#include <cmath>
#include <iostream>

image_handler::image_handler(string filename, int max_iter, int dimension_x, int dimension_y)
{
	if (!filename.empty())
	{
		m_filename = filename;
	}
	m_max_iter = max_iter;

	try {
#ifdef USING_OCV
		//For now we allocate a 1 channel to get it working and extend to 3 channel later for RGB
		m_img_mat = new Mat(dimension_x, dimension_y, CV_8UC3);
#else
		m_img_bmp = new bitmap_image(dimension_x, dimension_y);
#endif
	}
	catch (std::exception &e)
	{
		cout << "Exception during image_handler construction: " << e.what() << endl;
	}
}

image_handler::~image_handler()
{
	try {
#ifdef USING_OCV
		if (nullptr != m_img_mat)
		{
			delete m_img_mat;
		}
#else
		if (nullptr != m_img_bmp)
		{
			delete m_img_bmp;
		}
#endif
	}
	catch (std::exception &e)
	{
		cout << "Exception during image_handler destruction: " << e.what() << endl;
	}
}

int image_handler::set_filename(string filename)
{
	if (!filename.empty())
	{
		m_filename = filename;
		return 0;
	}
	return -1;
}

/*
	This uses a slightly modified version of a bernstein polynomial to determine the RGB spectrum.
	By using this polynomial, we map the number of iterations on a continous [0...1] scale giving a
	continous colour gradient.
	r(t)=9*(1−t)*t^3
	g(t)=15*(1−t)^2*t^2
	b(t)=8.5*(1−t)^3*t
*/
RGB_T image_handler::get_smooth_RGB_from_iter(int iterations)
{
	//First we need to map the iterations from 0..1
	double t = (double)iterations / (double)m_max_iter;

	//then we apply these in our polynomials
	uint8_t r = (uint8_t)(9 * (1 - t)*pow(t, 3) * 255); //And of course multiply by the max value of a single channel 
	uint8_t g = (uint8_t)(15 * pow((1 - t), 2)*pow(t, 2) * 255);
	uint8_t b = (uint8_t)(8.5 * pow((1 - t), 3) * t * 255);

	return std::make_tuple(r, g, b);
}


int image_handler::write_image(window<int>& screen, vector<int>& colours)
{
	int success = -1;
#ifdef USING_OCV
	int k = 0;
	for (int y = screen.get_y_min(); y < screen.get_y_max(); ++y)
	{
		for (int x = screen.get_x_min(); x < screen.get_x_max(); ++x)
		{
			//Convert from our own RGB tuple to the opencv Vec3b 
			//which is just a vector of uint8_t's
			RGB_T rgb = get_smooth_RGB_from_iter(colours[k]);
			Vec3b ocv_rgb(get<0>(rgb), get<1>(rgb), get<2>(rgb));
			//Then set the RGB values at each pixel
			m_img_mat->at<Vec3b>(Point(x, y)) = ocv_rgb;
		}
	}
	try {
		if (imwrite(m_filename, *m_img_mat))
		{
			success = 0;
		}
	}
	catch (std::exception &e)
	{
		cout << "Exception during image_handler construction: " << e.what() << endl;
	}

#else
	int k = 0;
	for (int y = screen.get_y_min(); y < screen.get_y_max(); ++y)
	{
		for (int x = screen.get_x_min(); x < screen.get_x_max(); ++x)
		{
			//Convert from our own RGB tuple to the bmp image version
			RGB_T rgb = get_smooth_RGB_from_iter(colours[k]);
			rgb_t rgb_bmp;
			rgb_bmp.red = get<0>(rgb);
			rgb_bmp.green = get<1>(rgb);
			rgb_bmp.blue = get<2>(rgb);
			//Then set the RGB values at each pixel
			m_img_bmp->set_pixel(x, y, rgb_bmp);
			k++;
	}
}
	try {
		cout << "Writing bitmap to: " << m_filename << endl;
		m_img_bmp->save_image(m_filename);
		cout << "Image wrote successfully" << endl;
		success = 0;
	}
	catch (std::exception &e)
	{
		cout << "Exception during image_handler construction: " << e.what() << endl;
	}
#endif
	return success;
}




