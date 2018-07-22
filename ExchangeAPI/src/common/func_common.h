#pragma once
class CFuncCommon
{
public:
	CFuncCommon();
	~CFuncCommon();
	static int EncodeConvert(char *from_charset, char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);

};

