#pragma once

//TODO - Add an openMP version to the sysinfo log

#ifndef _MANDEL_LOGGER_HPP
#define _MANDEL_LOGGER_HPP

#include <fstream>
#include <chrono>
#include <string>
#include <vector>

using namespace std;

#if defined(__unix__)
const string sysinfo_path("/proc/cpuinfo");
const string perma_log_filepath("../resources/logs/test_log.txt");
#elif defined(_WIN32) || (WIN32)
const string perma_log_filepath("..\\resources\\logs\\test_log.txt");
#endif


//Divides every new entry to the logfile (between runs)
#if defined(__unix__)
const string logfile_entry_divider("\\n#NEW_ENTRY\\n");
#elif defined(_WIN32) || (WIN32)
const string logfile_entry_divider("\\r\\n#NEW_ENTRY\\r\\n");
#endif

enum Log_level{
	NONE = 0,
	MINIMUM = 1,
	DEFAULT = 2,
	HIGH = 3
};

class mandel_logger 
{
private:

	/////////////////////
	//PRIVATE MEMBERS  //
	/////////////////////

	//Stores the permanent log filename. Always uses Log_level HIGH
	string m_permalog_filename;

	//May be empty 
	string m_alternatelog_filename;

	//Quick access for determining whether to also write to the alternate log
	bool m_using_altlog;

	//Contains the details to be written to the logfile, vectored to reduce 
	//the need for constant file IO. we'll open + write all of it in one go. 
	//All entries will be comma seperated in a single entry, that way in the 
	//Future, maybe just use Excel to handle them 
	vector<string> m_logfile_details;

	//Determines the amount of information to be logged to the system
	//For this project will only check if NONE otherwise will just log all
	Log_level m_log_level;

	//Quick access for platform type 
	bool m_plat_is_unix;

	/////////////////////
	//PRIVATE FUNCTIONS//
	/////////////////////

	//Get the sysinfo string from either unix or windows, automatically determined
	string get_sysinfo_string(void);

	//extract information from a unix sysinfo string given a set of tokens check /proc/cpuinfo 
	string extract_info_from_sysstring(string extraction_string, vector<string> tokens_to_match);

	bool m_details_outstanding;

public:

	/************************************************

					CTOR + DTOR

	*************************************************/

	//If altlog_filename provided writes the details to a seperate logfile as well
	//otherwise uses default permalog path only
	mandel_logger(Log_level log_lvl, string altlog_filename = "");

	~mandel_logger();

	/************************************************

					UTILITY
	
	*************************************************/

	//Adds a single string detail to m_logfile_details, used for most of the fractal 
	//generation details that we don't need to store in this class 
	void add_logfile_detail(string log_detail);
	

	/************************************************

					CORE

	*************************************************/

	//Write the m_logfile_details to the provided path, if path is not provided 
	//Writes instead to the permalog & altlog if there is one
	bool write_logdetails_to_path(string logpath = "");
};

#endif

