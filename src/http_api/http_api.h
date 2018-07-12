#pragma once
class CHttpAPI
{
public:
	CHttpAPI();
	~CHttpAPI();
	void SetKey(string strAPIKey, string strSecretKey);
	void SetURL(string strURL);
	void Run(int threadNums);
private:
	boost::thread_group m_paymentWorkers;
	string m_strAPIKey;			//�û������apiKey
	string m_strSecretKey;		//�������ǩ����˽Կ
	string m_strURI;
};

