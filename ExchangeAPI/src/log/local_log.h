#pragma once
#include "common/thread_safe_queue.h"
#include <fstream>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#define LOCAL_ERROR		LocalLogger::GetInstancePt()->LogError
#define LOCAL_WARNING	LocalLogger::GetInstancePt()->LogWarning
#define LOCAL_INFO		LocalLogger::GetInstancePt()->LogInfo
#define LOCAL_FUNCTION		LocalLogger::GetInstancePt()->LogFunction 
#define MAX_UUID_CACHE_COUNT 10000


#define MAX_LOCAL_LOG_CONTENT_LEN 2048

//----------------------------------------------------------
//���ƣ� ��־����д�������,
//��ע��������һ���߳���д��log�����ļ�,����push_log_text�������һ��������,��_thread�߳���ȡ��д��.
//�����ˣ�����
//---------------------------------------------------------- 
enum LOG_TYPE
{
	LOG_ERROR,//����log
	LOG_WARNING,//����log
	LOG_INFO, //��ͨ��¼
	LOG_FUNCTION, //���ܣ���������Ҫ�Ĳ���ʹ�ô����ͼ�¼log
	LOG_MAX_TYPE,
};
class LocalLogger
{
	DECLARE_SINGLETONPT(LocalLogger)
	static std::string s_LogTypeTitle[LOG_MAX_TYPE];
public:
	LocalLogger();
	~LocalLogger();
public:
	void SetBatchMode(bool mode){ m_batchMode = mode; };
	bool IsBatchMode(){ return m_batchMode; } 

	void SetLogPath(const char* path);
	void LogError(const char * format, ...);
	void LogWarning(const char * format, ...);
	void LogInfo(const char * format, ...); 
	void LogFunction(const char * format, ...); 

	void PushLogText(LOG_TYPE log_type, const char* log_text); 

	void LoadFile(); 
	void Start();
	void Close();
	void SwapFront2Middle();

	bool IsComplete(){ return m_completeFlag; };
private:
	void ThreadWorker();
	void BatchWriteLog();
	void WriteLog();
private:
	bool m_quitFlag;
	bool m_completeFlag;

	boost::condition_variable m_hasLogData;
	boost::mutex m_logMutex; 

	bool m_batchMode;
	std::vector<std::string> m_vecFront;
	std::vector<std::string> m_vecMiddle;
	std::vector<std::string> m_vecBack; 

	std::list<std::string> m_logQueue; 

	std::ofstream m_outputFile;
	int m_nDay;
	std::string m_logFilePath;
	bool m_bFirst;
};