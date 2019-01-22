#include "stdafx.h"
#include "account_manager.h"
#include <string>
#include <fstream>
#include <sstream>
#include "net/nmsg_server_iml.h"
CAccountManager::CAccountManager()
{
}


CAccountManager::~CAccountManager()
{
}

void CAccountManager::Init()
{
	m_mapAccountDate.clear();
	m_setWhiteAccount.clear();
	//先读有效期
	{
		std::string strFilePath = "./account.txt";
		std::ifstream stream(strFilePath);
		if(!stream.is_open())
			return;
		char lineBuffer[4096] = {0};
		if(stream.fail())
			return;
		time_t tNow = time(NULL);
		tm _tm = *localtime(&tNow);
		while(!stream.eof())
		{
			stream.getline(lineBuffer, sizeof(lineBuffer));
			if(*lineBuffer == 0 || (lineBuffer[0] == '/' && lineBuffer[1] == '/') || (lineBuffer[0] == '-' && lineBuffer[1] == '-'))
				continue;
			std::stringstream lineStream(lineBuffer, std::ios_base::in);
			char szAccount[512] = {};
			char szDate[512] = {};
			lineStream >> szAccount >> szDate;
			std::string strDate = szDate;
			int offset = 0;
			int begin = 0;
			int year = 0;
			int month = 0;
			int day = 0;
			int hour = 0;
			int min = 0;
			int sec = 0;
			while(offset != std::string::npos)
			{
				offset++;
				offset = strDate.find_first_of(".", offset);
				std::string sub = strDate.substr(begin, offset-begin);
				if(year == 0)
					year = stoi(sub);
				else if(month == 0)
					month = stoi(sub);
				else if(day == 0)
					day = stoi(sub);
				else if(hour == 0)
					hour = stoi(sub);
				else if(min == 0)
					min = stoi(sub);
				else if(sec == 0)
					sec = stoi(sub);
				begin = offset+1;
			}
			_tm.tm_year = year-1900;
			_tm.tm_mon = month-1;
			_tm.tm_mday = day;
			_tm.tm_hour = hour;
			_tm.tm_min = min;
			_tm.tm_sec = sec;
			time_t tDate = mktime(&_tm);
			m_mapAccountDate[szAccount] = tDate;
		}
		stream.close();
	}
	//白名单
	{
		std::string strFilePath = "./white.txt";
		std::ifstream stream(strFilePath);
		if(!stream.is_open())
			return;
		char lineBuffer[4096] = {0};
		if(stream.fail())
			return;
		while(!stream.eof())
		{
			stream.getline(lineBuffer, sizeof(lineBuffer));
			if(*lineBuffer == 0 || (lineBuffer[0] == '/' && lineBuffer[1] == '/') || (lineBuffer[0] == '-' && lineBuffer[1] == '-'))
				continue;
			std::stringstream lineStream(lineBuffer, std::ios_base::in);
			char szAccount[512] = {};
			lineStream >> szAccount;
			m_setWhiteAccount.insert(szAccount);
		}
		stream.close();
	}
}

bool CAccountManager::CheckAccount(std::string strAccount, time_t& tDate, clib::psession __c)
{
	std::set<std::string>::iterator it = m_setWhiteAccount.find(strAccount);
	if(it != m_setWhiteAccount.end())
	{
		tDate = 0;
		return true;
	}
	time_t tNow = time(NULL);
	std::map<std::string, time_t>::iterator _it = m_mapAccountDate.find(strAccount);
	if(_it != m_mapAccountDate.end())
	{
		tDate = _it->second;
		if(tNow < _it->second)
		{
			std::map<std::string, clib::psession>::iterator __it = m_mapAccountSession.find(strAccount);
			if(__it != m_mapAccountSession.end())
			{
				nmsg_server_iml::gc_account_invalid(__it->second);
				__it->second->del_ref();
			}
			m_mapAccountSession[strAccount] = __c;
			__c->add_ref();
			return true;
		}
	}
	return false;
}

void CAccountManager::Update()
{
	time_t tNow = time(NULL);
	std::map<std::string, clib::psession>::iterator itB = m_mapAccountSession.begin();
	std::map<std::string, clib::psession>::iterator itE = m_mapAccountSession.end();
	while(itB != itE)
	{
		std::map<std::string, time_t>::iterator _it = m_mapAccountDate.find(itB->first);
		if(_it != m_mapAccountDate.end())
		{
			if(tNow >= _it->second)
			{
				nmsg_server_iml::gc_account_invalid(itB->second);
				itB->second->del_ref();
				itB = m_mapAccountSession.erase(itB);
				continue;
			}
		}
		else
		{
			nmsg_server_iml::gc_account_invalid(itB->second);
			itB->second->del_ref();
			itB = m_mapAccountSession.erase(itB);
			continue;
		}
		++itB;
	}
}