#pragma once
#include "common/thread_safe_queue.h"
#include <fstream>
#include <iostream>
using namespace std;
class CActionLog
{
public:
	CActionLog(const char* type, const char* format, ...);
	~CActionLog() {};
};


struct sActionLog
{
	std::string m_strTableName;
	std::string m_strText;
};
class CLocalActionLog
{
	DECLARE_SINGLETONPT(CLocalActionLog)
public:
	CLocalActionLog();
	~CLocalActionLog();
public:
	//push log 到容器中。
	void push_log_text(std::string file_name, const char* log_text);

	void loadFile(std::string logTableName);

	//线程启动函数
	void start();

	void set_log_path(const char * path);
	void server_start();
	void server_stop();
private:
	//线程逻辑,取出log并且写入文件。
	void _thread();
private:
	boost::condition_variable m_hasLogData;
	boost::mutex m_logMutex;
	std::deque<sActionLog> log_queue;

	int m_nDay;
	std::string m_path;

	std::map<std::string, ofstream*> map_file;
	boost::thread* m_pThread;
	bool m_bExit;
};