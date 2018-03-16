#pragma once

//TODO - Add an openMP version to the sysinfo log

#ifndef _MANDEL_LOGGER_HPP
#define _MANDEL_LOGGER_HPP

#include <fstream>
#include <chrono>
#include <string>
#include <vector>

using namespace std;

const string perma_log_filename("..\\resources\\logs\\perma_log.txt");
const string sysinfo_path("/proc/cpuinfo");

const enum Log_level{
	NONE = 0,
	MINIMUM = 1,
	DEFAULT = 2,
	HIGH = 3
};

class mandel_logger 
{
private:
	//Stores the permanent log filename. Always uses Log_level HIGH
	string m_permalog_filename;

	//May be empty 
	string m_alternatelog_filename;

	//Quick access for determining whether to also write to the alternate log
	bool m_using_altlog;

	//Contains the details to be written to the logfile, vectored to reduce 
	//the need for constant file IO. we'll open + write all of it in one go. 
	vector<string> m_logfile_details;

	//Determines the amount of information to be logged to the system
	Log_level m_log_level;

	//Quick access for platform type 
	bool m_plat_is_unix;

public:

	//CTOR & DTOR
	mandel_logger(Log_level log_lvl, string log_filename);

	~mandel_logger();

	//Utility
	string extract_info_from_sysstring(string extraction_string, vector<string> tokens_to_match);
	
	string get_sysinfo_string(void);

	//Core
};

#endif