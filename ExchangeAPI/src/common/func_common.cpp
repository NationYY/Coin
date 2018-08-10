#include "stdafx.h"
#include "func_common.h"
#include "iconv/iconv.h"
#include <sstream>
#include <iomanip>
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

bool CFuncCommon::CheckEqual(double a, double b)
{
	return (fabs(a - b) < 0.000001) ? true : false;
}