#include "stdafx.h"
#include "http_api.h"


CHttpAPI::CHttpAPI()
{
}


CHttpAPI::~CHttpAPI()
{
}

void CHttpAPI::SetKey(string strAPIKey, string strSecretKey)
{
	m_strAPIKey = strAPIKey;
	m_strSecretKey = strSecretKey;
}

void SetURL(string strURL)
{
}

void CHttpAPI::Run(int threadNums)
{


}