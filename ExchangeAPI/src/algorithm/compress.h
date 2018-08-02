#pragma once
bool ZlibDecompress(unsigned char* pdata, int in_size, unsigned char*& out_data, int &out_size);
bool GZlibDecompress(unsigned char* zdata, int nzdata, unsigned char* data, int* ndata);
