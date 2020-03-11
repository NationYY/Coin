#include "stdafx.h"
#include "local_log.h"
#include "string.h"
#include <stdarg.h> 
#include <stdio.h>
#include<Windows.h>
#include <clib/lib/log/log.h>
std::string CLocalLogger::s_LogTypeTitle[LOG_MAX_TYPE] = {
	"ERROR", "WARNING", "INFO", "FUNCTION"
};

CLocalLogger::CLocalLogger() :m_batchMode(false), m_quitFlag(false), m_completeFlag(true), m_bFirst(true),
 m_cbFunc(NULL)
{

}

CLocalLogger::~CLocalLogger()
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


void CLocalLogger::PushLogText(LOG_TYPE log_type, const char* log_text)
{
	if(m_bFirst)
	{
		OutputDebugString("\n");
		m_bFirst = false;
	}
	if (log_type < 0 || log_type >= LOG_MAX_TYPE)
	{
		log_type = LOG_ERROR;
	}

	time_t now_t = time(NULL);
	tm nowtm;
	localtime_s(&nowtm, &now_t);
	char context[MAX_LOCAL_LOG_CONTENT_LEN + 128];

	_snprintf(context, sizeof(context)-1, "%d-%02d-%02d %02d:%02d:%02d %s %s\n",
		(int)(1900 + nowtm.tm_year), (int)(1 + nowtm.tm_mon), nowtm.tm_mday, (int)(nowtm.tm_hour), nowtm.tm_min, nowtm.tm_sec,
		s_LogTypeTitle[log_type].c_str(), log_text);
	context[sizeof(context)-1] = '\0';
	OutputDebugString(context);
	if(m_cbFunc)
		m_cbFunc(log_type, context);
	if(m_batchMode)
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

void CLocalLogger::SwapFront2Middle()
{
	boost::mutex::scoped_lock sl(m_logMutex);
	if (m_vecFront.size() && m_vecMiddle.empty())
	{
		m_vecMiddle.swap(m_vecFront);
		m_hasLogData.notify_one();
	}
}

void CLocalLogger::Start()
{
	if (m_logFilePath.empty())
	{
		m_logFilePath = "./log/";
	}
	m_nDay = 0;

	LoadFile();
	boost::thread t(boost::bind(&CLocalLogger::ThreadWorker, this));
	t.detach();
}

void CLocalLogger::Close()
{
	m_quitFlag = true;
	{
		boost::mutex::scoped_lock sl(m_logMutex);
		m_hasLogData.notify_all();
	}
		
}

void CLocalLogger::LoadFile()
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

void CLocalLogger::WriteLog()
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

void CLocalLogger::BatchWriteLog()
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
			std::cout<<m_vecBack[i];
			m_outputFile << m_vecBack[i] << std::flush;
		}
		m_vecBack.clear();
	}
}

void CLocalLogger::ThreadWorker()
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

void CLocalLogger::SetLogPath(const char* path)
{
	if (NULL == path)
		return;

	m_logFilePath = path;
}

void CLocalLogger::LogError(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN];
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);
	//DEBUG_ERROR(context);
	PushLogText(LOG_ERROR, context);
}

void CLocalLogger::LogWarning(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN];
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);
	//DEBUG_WARNING(context);
	PushLogText(LOG_WARNING, context);
}

void CLocalLogger::LogInfo(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN] = "";
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);
	//DEBUG_LOG(context);
	PushLogText(LOG_INFO, context);
}

void CLocalLogger::LogFunction(const char * format, ...)
{
	char context[MAX_LOCAL_LOG_CONTENT_LEN];
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(context, sizeof(context), format, args);
	va_end(args);

	PushLogText(LOG_FUNCTION, context);
}