#ifndef __V_MOTION__H_
#define __V_MOTION__H_

#include "internals\commonheaders.h"
#include <functional>

#ifdef VMOTION_EXPORT
#define VMOTION_API __declspec(dllexport)
#else
#define VMOTION_API __declspec(dllimport)
#endif


typedef int VMOTION_RESULT;

#define VMOTION_INSUCCESS 0 //not as critical as error. data invalid and stuff
#define VMOTION_ERROR -1
#define VMOTION_SUCCESS 1
#define VMOTION_INVALID_ARGUMENT -2
#define VMOTION_OUT_OF_RANGE -3
#define VMOTION_DISCONNECTED -4

#define DEFAULT_COM "COM5"

#define VMOTION_VECTOR_SZ 3

#pragma pack(push, 1) //temporarily disable padding

typedef struct VMOTION_VECTOR
{
	float x, y, z;
} VMOTION_VECTOR;

typedef struct VMOTION_DATA
{
	VMOTION_VECTOR ypr;
	bool button;
} VMOTION_DATA;

typedef struct
{
	unsigned int packetNumber;
	VMOTION_DATA data;
} VMOTION_STATE;

#pragma pack(pop)

extern "C"
{
	typedef void(*VMotionControllerConnectionCB)(void* sender, VMOTION_RESULT connected);

	VMOTION_API VMOTION_RESULT VMotionInit(VMotionControllerConnectionCB cb = NULL, char* device_path = DEFAULT_COM);
	VMOTION_API VMOTION_RESULT VMotionShutdown();
	VMOTION_API int VMotionGetControllerCount();

	// Helpers to get data from state...

	VMOTION_API VMOTION_RESULT VMotionGetYPR(unsigned int idx, VMOTION_VECTOR& result, unsigned int timeout = 0);
	VMOTION_API VMOTION_RESULT VMotionGetYPR_Ptr(unsigned int idx, float* result, unsigned int timeout = 0);
}

#endif //__V_MOTION__H_