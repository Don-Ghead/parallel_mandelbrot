#pragma once

#ifndef _IMAGE_HANDLER_HPP
#define _IMAGE_HANDLER_HPP

//#define USING_OCV

#include <string>
#include <tuple>
#include "window.hpp"

#ifdef USING_OCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2\opencv.hpp>
using namespace cv;
#else
#include "bitmap_image.hpp"
#endif

using namespace std;

typedef tuple<uint8_t, uint8_t, uint8_t> RGB_T;

/***************************************************************

					IMAGE_HANDLER

****************************************************************/

class image_handler
{
private:
	string m_filename;
	int m_max_iter;
#ifdef USING_OCV
	Mat* m_img_mat;
#else
	bitmap_image* m_img_bmp;
#endif

public:

	//Constructor & Destructor
	image_handler(string filename, int max_iter, int dimension_x, int dimension_y); //Can extend this constructor to also take a bit-depth value later

	~image_handler();

	//Utility funcs	
	int set_filename(string filename);

	RGB_T get_smooth_RGB_from_iter(int iterations);

	//Core handler work
	int write_image(window<int>& screen, vector<int>& colours);

};

#endif