#include "stdafx.h"
#include "func_common.h"
#include "iconv/iconv.h"
#include <sstream>
#include <iomanip>
#include <io.h>
#include <iostream>
#include "boost/thread.hpp"
CFuncCommon::CFuncCommon()
{
}


CFuncCommon::~CFuncCommon()
{
}

int CFuncCommon::EncodeConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	iconv_t cd;

	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if(cd == 0) return -1;
	memset(outbuf, 0, outlen);
	if(iconv(cd, pin, &inlen, pout, &outlen) == -1) return -1;
	iconv_close(cd);
	return 0;
}

std::string CFuncCommon::Double2String(double value, int decimal)
{
	std::stringstream ss;
	ss.setf(std::ios::fixed);
	ss << std::setprecision(decimal) << value;
	return ss.str();
}

double CFuncCommon::Round(double value, int decimal)
{
	std::stringstream ss;
	ss.setf(std::ios::fixed);
	ss << std::setprecision(decimal) << value;

	double result;
	ss >> result;
	return result;
}




bool CFuncCommon::CheckEqual(double a, double b)
{
	return (fabs(a - b) < 0.000001) ? true : false;
}



char* CFuncCommon::ToString(unsigned __int64 number)
{
	static __declspec(thread) char szBuff[64] = "";
	sprintf(szBuff, "%llu", number);
	return szBuff;
}

char* CFuncCommon::ToString(__int64 number)
{
	static __declspec(thread) char szBuff[64] = "";
	sprintf(szBuff, "%lld", number);
	return szBuff;
}

char* CFuncCommon::ToString(int number)
{
	static __declspec(thread) char szBuff[32] = "";
	sprintf(szBuff, "%d", number);
	return szBuff;
}

char* CFuncCommon::ToString(unsigned int number)
{
	static __declspec(thread) char szBuff[32] = "";
	sprintf(szBuff, "%u", number);
	return szBuff;
}

std::string CFuncCommon::FormatTimeStr(__int64 time)
{
	tm* pTM = localtime((const time_t*)&time);
	char szBuff[128] = {0};
	_snprintf(szBuff, 128, "%d-%02d-%02d %02d:%02d:%02d", pTM->tm_year+1900, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
	return szBuff;
}

void CFuncCommon::GetAllFileInDirectory(const char* szPath, std::set<std::string>& setFiles)
{
	char dirNew[200];
	strcpy(dirNew, szPath);
	strcat(dirNew, "\\*.*");    // 在目录后面加上"\\*.*"进行第一次搜索

	intptr_t handle;
	_finddata_t findData;

	handle = _findfirst(dirNew, &findData);
	if(handle == -1)        // 检查是否成功
		return;
	do
	{
		if(findData.attrib & _A_SUBDIR)
		{
			if(strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				continue;
			strcpy(dirNew, szPath);
			strcat(dirNew, "\\");
			strcat(dirNew, findData.name);

			GetAllFileInDirectory(dirNew, setFiles);
		}
		else
			setFiles.insert(findData.name);
	} while(_findnext(handle, &findData) == 0);

	_findclose(handle);    // 关闭搜索句柄
}

std::string CFuncCommon::LocaltimeToISO8601(time_t time)
{
	//tm* pTM = localtime(&time);
	char buf[36];
	strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000Z", gmtime(&time));
	return buf;
}

time_t CFuncCommon::ISO8601ToTime(std::string& time)
{
	tm time_struct;
	std::istringstream ss(time);
	ss >> std::get_time(&time_struct, "%Y-%m-%dT%H:%M:%SZ");
	std::time_t time_unix = _mkgmtime(&time_struct);
	return time_unix;
}

__int64 CFuncCommon::GenUUID()
{
	static boost::mutex guidMutex;
	static __int64 increment = 0;
	//我们时间只存储从2016年1月1日到现在的时间
	static int targetTime = 1451635200;
	static time_t lastTime = time(NULL) - targetTime;
	time_t now = time(NULL) - targetTime;
	//时间位
	time_t time_flag = 0;
	{
		boost::mutex::scoped_lock lock(guidMutex);
		//不能用不等于  因为lastTime会根据情况跳时间
		if(lastTime < now)
		{
			lastTime = now;
			increment = 0;
		}
		++increment;
		//当前超过自增值了  直接跳到下一秒
		if(increment >= 0x3FFFFFFFF)
		{
			lastTime = lastTime + 1;
			increment = 0;
		}
		// 时间位 高29位
		time_flag = (__int64)lastTime << 35;
	}
	// 数字：时间29，自增34
	//return j + i +server_id + type;
	return (time_flag | increment);

}