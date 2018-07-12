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
	string m_strAPIKey;			//用户申请的apiKey
	string m_strSecretKey;		//请求参数签名的私钥
	string m_strURI;
};

