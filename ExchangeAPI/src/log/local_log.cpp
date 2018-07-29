#include "stdafx.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "local_log.h"
#include "string.h"
#include <stdarg.h> 
#include <stdio.h>


std::string LocalLogger::s_LogTypeTitle[LOG_MAX_TYPE] = {
	"ERROR", "WARNING", "INFO", "FUNCTION"
};

LocalLogger::LocalLogger() :m_batchMode(false), m_quitFlag(false), m_completeFlag(true)
{

}

LocalLogger::~LocalLogger()
{
	if (!m_quitFlag)
	{
		Close();
	}

	while (!m_completeFlag)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		continue;
	}

	if (m_outputFile)
	{
		m_outputFile.close();
	}
}


void LocalLogger::PushLogText(LOG_TYPE log_type, const char* log_text)
{

	if (log_type < 0 || log_type >= LOG_MAX_TYPE)
	{
		log_type = LOG_ERROR;
	}

	time_t now_t = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &now_t);
	char context[MAX_LOCAL_LOG_CONTENT_LEN + 128];

	_snprintf(context, sizeof(context)-1, "%d-%02d-%02d %02d:%02d:%02d %s %s \n",
		(int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday, (int)(nowtm.tm_hour), nowtm.tm_min, nowtm.tm_sec,
		s_LogTypeTitle[log_type].c_str(), log_text);
	context[sizeof(context)-1] = '\0';

	if (m_batchMode)
	{
		m_vecFront.push_back(context);
	}
	else
	{
		boost::mutex::scoped_lock sl(m_logMutex);
		m_logQueue.push_back(context);
		m_hasLogData.notify_one(); 
	}
}

void LocalLogger::SwapFront2Middle()
{
	boost::mutex::scoped_lock sl(m_logMutex);
	if (m_vecFront.size() && m_vecMiddle.empty())
	{
		m_vecMiddle.swap(m_vecFront);
		m_hasLogData.notify_one();
	}
}

void LocalLogger::Start()
{
	if (m_logFilePath.empty())
	{
		m_logFilePath = "./log/";
	}
	m_nDay = 0;

	LoadFile();
	boost::thread t(boost::bind(&LocalLogger::ThreadWorker, this));
	t.detach();
}

void LocalLogger::Close()
{
	m_quitFlag = true;
	{
		boost::mutex::scoped_lock sl(m_logMutex);
		m_hasLogData.notify_all();
	}
		
}

void LocalLogger::LoadFile()
{
	time_t now_t = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &now_t);
	if (m_nDay != nowtm.tm_mday)
	{
		char file_name[512];
		_snprintf(file_name, sizeof(file_name)-1, "%s%d%02d%02d.txt", m_logFilePath.c_str(), (int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday);
		file_name[sizeof(file_name)-1] = '\0';

		if (m_outputFile)
		{
			m_outputFile.close();
		}
		m_outputFile.open(file_name, std::ios::app);
		m_nDay = nowtm.tm_mday;
	}
}

void LocalLogger::WriteLog()
{
	while (!m_quitFlag)
	{ 
		std::string logStr = "";
		{
			boost::mutex::scoped_lock sl(m_logMutex);
			if (m_logQueue.empty())
			{
				m_hasLogData.wait(sl);
				continue;
			}
			else
			{
				logStr = m_logQueue.front();
				m_logQueue.pop_front();
			}
		}

		LoadFile();
		m_outputFile << logStr << std::flush;
	}

}

void LocalLogger::BatchWriteLog()
{
	std::string str_log;
	while (!m_quitFlag)
	{
		{
			boost::mutex::scoped_lock sl(m_logMutex);
			if (m_vecMiddle.size() == 0)
			{
				m_hasLogData.wait(sl);
				continue;
			}
			else
			{
				m_vecBack.swap(m_vecMiddle);
			}
		}

		for (size_t i = 0; i < m_vecBack.size(); ++i)
		{
			LoadFile();
			m_outputFile << m_vecBack[i] << std::flush;
		}
		m_vecBack.clear();
	}
}

void LocalLogger::ThreadWorker()
{ 
	//LOCAL_INFO("local_log start");

	m_completeFlag = false; 

	if (IsBatchMode())
	{
		BatchWriteLog();
	}
	else
	{
		WriteLog();
	}

	m_completeFlag = true;
}

void LocalLogger::SetLogPath(const char* path)
{
	if (NULL == path)
		return;

	m_logFilePath = path;
}

void LocalLogger::LogError(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN];
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);

	PushLogText(LOG_ERROR, context);
}

void LocalLogger::LogWarning(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN];
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);

	PushLogText(LOG_WARNING, context);
}

void LocalLogger::LogInfo(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN] = "";
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);

	PushLogText(LOG_INFO, context);
}

void LocalLogger::LogFunction(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN];
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);

	PushLogText(LOG_FUNCTION, context);
}