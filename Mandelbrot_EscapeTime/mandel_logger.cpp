/*
	ADD DESCRIPTION
*/

#include <fstream>
#include <iostream>

#include "mandel_logger.hpp"

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#include <stdio.h>
#endif

using namespace std;

const vector<string> sysinfo_tokens
{
	"model name",
	"cpu cores"
};

//Don't need to check the log level as it is enumerated and must be on of the specified values
mandel_logger::mandel_logger(Log_level log_lvl, string altlog_filename)
	: m_log_level(log_lvl), m_permalog_filename(perma_log_filepath), m_using_altlog(false)
{
	if (!altlog_filename.empty())
	{
		m_alternatelog_filename = altlog_filename;
		m_using_altlog = true;
	}

#if defined(__unix__)
	m_plat_is_unix = true;
#elif defined(WIN32) || defined(_WIN32)
	m_plat_is_unix = false;
#endif

}

mandel_logger::~mandel_logger()
{
}

bool mandel_logger::add_logfile_detail(string log_detail)
{
	if (!log_detail.empty())
	{
		m_logfile_details.push_back(log_detail);
		return true;
	}
	else
	{
		false;
	}
}

bool mandel_logger::write_logdetails_to_path(string logpath)
{
	bool success = false;
	if (!logpath.empty())
	{
		//Open output stream in output/append mode
		ofstream logfile(logpath, ios::out | ios::app);

		if (logfile.is_open())
		{
			cout << "Writing details to " << logpath << endl;
			//Divider includes newlines on either side to ensure entries are divided 
			logfile << logfile_entry_divider;

			for (int i = 0; i < m_logfile_details.size(); i++)
			{
				logfile << m_logfile_details[i] << ", ";
			}
			logfile << plat_newline;
			logfile.close();
			success = true;
		}
		else
		{
			cout << "Unable to open path to write logdetails";
		}
		
	}
	else
	{
		if (!m_using_altlog)
		{
			//Open output stream in output/append mode
			ofstream logfile(perma_log_filepath, ios::out | ios::app);

			if (logfile.is_open())
			{
				cout << "Writing details to permalog only" << endl;
				//Divider includes newlines on either side to ensure entries are divided 
				logfile << logfile_entry_divider;

				for (int i = 0; i < m_logfile_details.size(); i++)
				{
					logfile << m_logfile_details[i] << ", ";
				}
				logfile << plat_newline;
				logfile.close();
				success = true;
			}
			else
			{
				cout << "Unable to open path to write logdetails";
			}
		}
		else
		{
			//Open output stream in output/append mode
			ofstream logfile(perma_log_filepath, ios::out | ios::app);
			ofstream alt_logfile(m_alternatelog_filename, ios::out | ios::app);

			if (logfile.is_open() && alt_logfile.is_open())
			{
				cout << "Writing details to alt & perma logs" << endl;
				//Divider includes newlines on either side to ensure entries are divided 
				logfile << logfile_entry_divider;
				alt_logfile << logfile_entry_divider;

				for (int i = 0; i < m_logfile_details.size(); i++)
				{
					logfile << m_logfile_details[i] << ", ";
					alt_logfile << m_logfile_details[i] << ", ";
				}
				logfile << plat_newline;
				alt_logfile << plat_newline;
				logfile.close();
				alt_logfile.close();
				success = true;
			}
			else
			{
				cout << "Unable to open path to write logdetails";
			}
		}
	}
	return success;
}

string mandel_logger::extract_info_from_sysstring(string extraction_string, vector<string> tokens_to_match)
{
	if (tokens_to_match.empty() || extraction_string.empty())
	{
		//return empty string 
		return string("");
	}
	else
	{
		for (int i = 0; i < tokens_to_match.size(); i++)
		{
			//Search for token, if found return information after colon
			size_t found = extraction_string.find_last_of(tokens_to_match[i]);
			//Npos indicates we didn't find the character
			if (string::npos != found)
			{
				found = extraction_string.find_last_of(":");
				if (string::npos != found)
				{
					return extraction_string.substr(found);
				}
			}
		}		
	}
	return "";
}

/*
	This pulls information from the platform specific method 
	Unix = /proc/cpuinfo
	Windows = getsysinfo
*/
string mandel_logger::get_sysinfo_string(void)
{
	string return_string;
	//IF UNIX
	if (m_plat_is_unix)
	{
		//Create input filestream using sysinfo path
		ifstream sysinfo_strm(sysinfo_path); 

		//create a vector to hold the information from the sysinfo call 
		vector<string> sysinfo_tempvec;
		string sysinfo_tempstring;

		//Read stream while not EOF
		while (!sysinfo_strm.eof())
		{
			//Get each line and add the string to the vector to be filtered later
			getline(sysinfo_strm, sysinfo_tempstring);
			if (!sysinfo_tempstring.empty())
			{
				sysinfo_tempvec.push_back(sysinfo_tempstring);
			}
		}

		for (int i = 0; i < sysinfo_tempvec.size(); i++)
		{
			return_string += extract_info_from_sysstring(sysinfo_tempvec[i], sysinfo_tokens);
			return_string += ", ";
		}
	}
	//ELSE WINDOWS
	else
	{
		SYSTEM_INFO siSysInfo;

		// Copy the hardware information to the SYSTEM_INFO structure. 
		GetSystemInfo(&siSysInfo);


	}
	return "";
}




