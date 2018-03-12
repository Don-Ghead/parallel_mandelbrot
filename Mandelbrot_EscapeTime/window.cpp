#include "window.hpp"

/*
	Simple Window Implementation to define a domain both for the max image 
	size as well as the upper and lower limits for the drawing
*/

template<typename T>
window<T>::window(T x_min, T x_max, T y_min, T y_max)
	: m_x_min(x_min) ,m_x_max(x_max), m_y_min(y_min), m_y_max(y_max)
{
}

template<typename T>
void window<T>::define_window(T x_min, T x_max, T y_min, T y_max)
{
	m_x_min = x_min;
	m_x_max = x_max;
	m_y_min = y_min;
	m_y_max = y_max;
}

// Window dimensions utility 
template<typename T>
T window<T>::size()
{
	return (width() * height());
}

template<typename T>
T window<T>::width()
{
	return (m_x_max - m_x_min);
}

template<typename T>
T window<T>::height()
{
	return (m_y_max - m_y_min);
}

// Getters & setters

template<typename T>
T window<T>::get_x_min()
{
	return (m_x_min);
}

template<typename T>
T window<T>::get_x_max(void)
{
	return (m_x_max);
}

template<typename T>
T window<T>::get_y_min(void)
{
	return (m_y_min);
}

template<typename T>
T window<T>::get_y_max(void)
{
	return (m_y_max);
}