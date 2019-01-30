#include "stdafx.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "local_action_log.h"
#include "string.h"
#include <stdarg.h> 
#include <stdio.h>
#include<Windows.h>
CActionLog::CActionLog(const char* type, const char* format, ...)
{
	time_t tNow = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &tNow);
	char context[6144] = { 0 };
	_snprintf(context, sizeof(context)-1, "%d-%02d-%02d %02d:%02d:%02d ",
			  (int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday, (int)(nowtm.tm_hour), nowtm.tm_min, nowtm.tm_sec);
	char _context[6144] = { 0 };
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(_context, sizeof(_context), format, args);
	va_end(args);
	strcat(context, _context);
	CLocalActionLog::GetInstancePt()->push_log_text(type, context);
}

CFixActionLog::CFixActionLog(const char* type, const char* format, ...)
{
	time_t tNow = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &tNow);
	char context[6144] = { 0 };
	_snprintf(context, sizeof(context)-1, "%d-%02d-%02d %02d:%02d:%02d ",
			  (int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday, (int)(nowtm.tm_hour), nowtm.tm_min, nowtm.tm_sec);
	char _context[6144] = { 0 };
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(_context, sizeof(_context), format, args);
	va_end(args);
	strcat(context, _context);
	CLocalActionLog::GetInstancePt()->push_log_text(type, context, true);
}

CLocalActionLog::CLocalActionLog():m_pThread(NULL), m_bExit(false)
{

}

void CLocalActionLog::set_log_path(const char * path)
{
	if (NULL == path)
		return;

	m_path = path;
}

void CLocalActionLog::push_log_text(std::string file_name, const char* log_text, bool bDayFix)
{ 
	sActionLog str_log;
	str_log.m_strTableName = file_name;
	str_log.m_strText = log_text;
	str_log.m_bDayFix = bDayFix;
	boost::mutex::scoped_lock sl(m_logMutex);
	log_queue.push_back(str_log);
	m_hasLogData.notify_one();
	OutputDebugString("[");
	OutputDebugString(file_name.c_str());
	OutputDebugString("] ");
	OutputDebugString(log_text);
	OutputDebugString("\n");
}
void CLocalActionLog::start()
{
	if (m_path.empty())
	{
		m_path = "./log/";
	} 
	m_nDay = 0;
		
	m_pThread = new boost::thread(boost::bind(&CLocalActionLog::_thread, this));
}
void CLocalActionLog::loadFile(std::string logTableName, bool bDayFix)
{
	time_t now_t = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &now_t);
	if (m_nDay != nowtm.tm_mday)
	{
		for (std::map<std::string, ofstream*>::iterator iter = map_file.begin(); iter != map_file.end(); ++iter)
		{
			iter->second->close();
			delete iter->second;
		}
		map_file.clear();
		m_nDay = nowtm.tm_mday;
	}
	if(bDayFix)
	{
		if(map_fix_file.find(logTableName) == map_fix_file.end())
		{
			char file_name[512];
			_snprintf(file_name, sizeof(file_name)-1, "%s%s.txt", m_path.c_str(), logTableName.c_str());
			file_name[sizeof(file_name)-1] = '\0';
			ofstream* outfile = new ofstream;
			outfile->open(file_name, ios::app);
			map_fix_file[logTableName] = outfile;
		}
	}
	else
	{
		if(map_file.find(logTableName) == map_file.end())
		{
			char file_name[512];
			_snprintf(file_name, sizeof(file_name)-1, "%s%s%d%d%d.txt", m_path.c_str(), logTableName.c_str(), (int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday);
			file_name[sizeof(file_name)-1] = '\0';
			ofstream* outfile = new ofstream;
			outfile->open(file_name, ios::app);
			map_file[logTableName] = outfile;
		}
	}
	
}
void CLocalActionLog::_thread()
{
	sActionLog str_log;
	while(!m_bExit || !log_queue.empty())
    {
		{
			boost::mutex::scoped_lock sl(m_logMutex); 
			if (log_queue.empty())
			{
				m_hasLogData.wait(sl);
				continue;
			}

			str_log = log_queue.front();
			log_queue.pop_front();
		}

		//开始写入文件 
		loadFile(str_log.m_strTableName, str_log.m_bDayFix);
		if(str_log.m_bDayFix)
			*map_fix_file[str_log.m_strTableName] << str_log.m_strText << endl <<flush;
		else
			*map_file[str_log.m_strTableName] << str_log.m_strText << endl <<flush;
    }        
}

CLocalActionLog::~CLocalActionLog()
{
	m_bExit = true;
	{
		boost::mutex::scoped_lock sl(m_logMutex); 
		m_hasLogData.notify_one();
	}
	if(m_pThread)
	{
		m_pThread->join();
		delete m_pThread;
		m_pThread = NULL;
	}
	for (std::map<std::string, ofstream*>::iterator iter = map_file.begin(); iter != map_file.end(); ++iter)
	{
		iter->second->close();
		delete iter->second;
	}
	map_file.clear();

	for(std::map<std::string, ofstream*>::iterator iter = map_fix_file.begin(); iter != map_fix_file.end(); ++iter)
	{
		iter->second->close();
		delete iter->second;
	}
	map_fix_file.clear();
}

void CLocalActionLog::server_start()
{
	time_t now_t = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &now_t);
	char context[512];
	_snprintf(context, sizeof(context)-1, "%d-%02d-%02d %02d:%02d:%02d server started",
		(int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday, (int)(nowtm.tm_hour), nowtm.tm_min, nowtm.tm_sec);
	context[sizeof(context)-1] = '\0';
	push_log_text("server_operate", context);
}

void CLocalActionLog::server_stop()
{
	time_t now_t = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &now_t);
	char context[512];
	_snprintf(context, sizeof(context)-1, "%d-%02d-%02d %02d:%02d:%02d server stoped",
		(int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday, (int)(nowtm.tm_hour), nowtm.tm_min, nowtm.tm_sec);
	context[sizeof(context)-1] = '\0';
	push_log_text("server_operate", context);
}