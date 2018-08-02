#include "stdafx.h"
#include "compress.h"
#include "zlib/zlib.h"

bool ZlibDecompress(unsigned char * pdata, int in_size, unsigned char *& out_data, int &out_size)
{
	uLongf size = out_size;
	if(size == 0)
		return false;
	out_data = (unsigned char*)malloc(size);
	if(out_data == NULL)
		return false;
	int status = uncompress(out_data, &size, (unsigned char*)pdata, in_size);
	out_size = (int)size;
	return status == Z_OK;
}


bool GZlibDecompress(unsigned char* zdata, int nzdata, unsigned char* data, int* ndata)
{
	int err = 0;
	z_stream d_stream = { 0 }; /* decompression stream */
	static char dummy_head[2] = {
		0x8 + 0x7 * 0x10,
		(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
	};
	d_stream.zalloc = NULL;
	d_stream.zfree = NULL;
	d_stream.opaque = NULL;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	//只有设置为MAX_WBITS + 16才能在解压带header和trailer的文本
	err = inflateInit2(&d_stream, MAX_WBITS + 16);
	if(err != Z_OK)
		return false;
	//if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
	while(d_stream.total_out < (uLong)*ndata && d_stream.total_in < (uLong)nzdata) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
			break;
		if(err != Z_OK)
		{
			if(err == Z_DATA_ERROR)
			{
				d_stream.next_in = (Bytef*)dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) {
					return false;
				}
			}
			else
				return false;
		}
	}
	if(inflateEnd(&d_stream) != Z_OK) return false;
	*ndata = d_stream.total_out;
	return true;
}