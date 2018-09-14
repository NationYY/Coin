#pragma once
class CFuncCommon
{
public:
	CFuncCommon();
	~CFuncCommon();
	static int EncodeConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);
	static std::string Double2String(double value, int decimal);
	static bool CheckEqual(double a, double b);
	static char* ToString(unsigned __int64 number);
	static char* ToString(__int64 number);
	static char* ToString(int number);
	static char* ToString(unsigned int number);
	static double Round(double value, int decimal);
};

