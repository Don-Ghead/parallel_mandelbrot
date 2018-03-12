#pragma once

#ifndef _WINDOW_H
#define _WINDOW_H

template <typename T>
class window
{
private:
	T m_x_min, m_x_max, m_y_min, m_y_max;
public:
	window(T x_min, T x_max, T y_min, T y_max)
		: m_x_min(x_min), m_x_max(x_max), m_y_min(y_min), m_y_max(y_max)
	{
	}

	void define_window(T x_min, T x_max, T y_min, T y_max)
	{
		m_x_min = x_min;
		m_x_max = x_max;
		m_y_min = y_min;
		m_y_max = y_max;
	}

	// Window dimensions utility 
	T size(void)
	{
		return (width() * height());
	}

	T width(void)
	{
		return (m_x_max - m_x_min);
	}

	T height(void)
	{
		return (m_y_max - m_y_min);
	}

	// Getters & Setters 

	T get_x_min(void)
	{
		return (m_x_min);
	}

	T get_x_max(void)
	{
		return (m_x_max);
	}

	T get_y_min(void)
	{
		return (m_y_min);
	}

	T get_y_max(void)
	{
		return (m_y_max);
	}
	
};

#endif