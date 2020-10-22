#include "stdafx.h"
#include "func_common.h"
#include "iconv/iconv.h"
#include <sstream>
#include <iomanip>
#include <io.h>
#include <iostream>
#include "boost/thread.hpp"
#include "objbase.h"
#include <sys/timeb.h>
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
	char szBuff[64] = {0};
	if(decimal == 0)
		sprintf(szBuff, "%d", (int)value);
	else if(decimal == 1)
		sprintf(szBuff, "%.2f", value);
	else if(decimal == 2)
		sprintf(szBuff, "%.3f", value);
	else if(decimal == 3)
		sprintf(szBuff, "%.4f", value);
	else if(decimal == 4)
		sprintf(szBuff, "%.5f", value);
	else if(decimal == 5)
		sprintf(szBuff, "%.6f", value);
	else if(decimal == 6)
		sprintf(szBuff, "%.7f", value);
	else if(decimal == 7)
		sprintf(szBuff, "%.8f", value);
	else if(decimal == 8)
		sprintf(szBuff, "%.9f", value);
	else if(decimal == 9)
		sprintf(szBuff, "%.10f", value);
	if(decimal > 0)
	{
		szBuff[strlen(szBuff)-1] = '\0';
		int nPos = strlen(szBuff)-1;
		while(szBuff[nPos] == '0')
		{
			if(szBuff[nPos-1] == '.')
				break;
			szBuff[nPos] = '\0';
			nPos = strlen(szBuff)-1;
			if(szBuff[nPos] == '.')
			{
				szBuff[nPos] = '\0';
				break;
			}
		}
	}
	return szBuff;
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
	sprintf(szBuff, "%u", number);
	return szBuff;
}

char* CFuncCommon::ToString(unsigned long number)
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

std::string CFuncCommon::FormatDateStr(__int64 time)
{
	tm _tm;
	localtime_s(&_tm, (const time_t*)&time);
	char szBuff[128] = {0};
	_snprintf(szBuff, 128, "%d-%02d-%02d %02d:%02d:%02d", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	return szBuff;
}

std::string CFuncCommon::FormatTimeStr(__int64 time)
{
	tm _tm;
	localtime_s(&_tm, (const time_t*)&time);
	char szBuff[128] = { 0 };
	_snprintf(szBuff, 128, "%02d:%02d:%02d", _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
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

time_t CFuncCommon::ToTime(const char *sz)
{
	time_t ret = 0;
	sscanf(sz, "%lld", &ret);
	return ret;
}

char* CFuncCommon::GenUUID()
{
	static __declspec(thread) char buffer[64] = "";
	GUID guid;

	if(CoCreateGuid(&guid))
		return "";
	_snprintf(buffer, sizeof(buffer),
			  "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
			  guid.Data1, guid.Data2, guid.Data3,
			  guid.Data4[0], guid.Data4[1], guid.Data4[2],
			  guid.Data4[3], guid.Data4[4], guid.Data4[5],
			  guid.Data4[6], guid.Data4[7]);
	return buffer;
	/*
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
	*/
}

static const unsigned int crc32tab[] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
	0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
	0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
	0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
	0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
	0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
	0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
	0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
	0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
	0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
	0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
	0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
	0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
	0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
	0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
	0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
	0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
	0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
	0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
	0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
	0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
	0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
	0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
	0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
	0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
	0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
	0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
	0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
	0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
	0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
	0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
	0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
	0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
	0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
	0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
	0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
	0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
	0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
	0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
	0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
	0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
	0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
	0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
	0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
	0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
	0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
	0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
	0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
	0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
	0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
	0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
	0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};


int CFuncCommon::crc32(const unsigned char *buf, unsigned int size)
{
	unsigned int i, crc;
	crc = 0xFFFFFFFF;


	for(i = 0; i < size; i++)
		crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);

	return int(crc^0xFFFFFFFF);
}

unsigned long CFuncCommon::GetUNIXTime()
{
	timeb t;
	ftime(&t);
	return unsigned long(t.time * 1000 + t.millitm);
}