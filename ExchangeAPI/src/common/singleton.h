#pragma once
#include <iostream>
#include <map>
#define DECLARE_SINGLETONPT(ClassName)\
	public:\
	static ClassName *GetInstancePt(bool bRelease = false)\
{\
	static void* srcMem = NULL; \
	static ClassName* pt = NULL; \
	static bool bFirst = true; \
	static bool bReleased = false; \
if(!bRelease && srcMem == NULL && !bReleased)\
{\
	srcMem = malloc(sizeof(ClassName)); \
	memset(srcMem, 0, sizeof(ClassName)); \
	pt = new(srcMem)ClassName; \
}\
if(bRelease)\
{\
	bReleased = true; \
if(pt)\
{\
	pt->~ClassName(); \
	pt = NULL; \
}\
if(srcMem)\
{\
	free(srcMem); \
	srcMem = NULL; \
}\
}\
if(bFirst && !bRelease)\
{\
	bFirst = false; \
}\
	return pt; \
}\
	static ClassName &GetInstance()\
{\
	ClassName* p = GetInstancePt(); \
	return *p; \
}\
	static void ReleaseInstance()\
{\
	GetInstancePt(true); \
}