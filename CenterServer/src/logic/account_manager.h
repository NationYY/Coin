#pragma once
#include <map>
#include <set>
#include "net/nIterface.h"
#include "net/session.h"
class CAccountManager
{
public:
	CAccountManager();
	~CAccountManager();
	void Init();
	bool CheckAccount(std::string strAccount, time_t& tDate, clib::psession __c);
	void Update();
private:
	std::map<std::string, time_t> m_mapAccountDate;
	std::set<std::string> m_setWhiteAccount;

	std::map<std::string, clib::psession> m_mapAccountSession;
};

