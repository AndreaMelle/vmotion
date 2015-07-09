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

#define VMOTION_AXIS_DIM 3

#pragma pack(push, 1) //temporarily disable padding

typedef struct VMOTION_VECTOR
{
	float x, y, z;
} VMOTION_VECTOR;

typedef struct VMOTION_DATA
{
	VMOTION_VECTOR orientation; // the yaw, pitch roll after sensor fusion
	VMOTION_VECTOR acceleration; //the calibrated acceleration
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

	VMOTION_API VMOTION_RESULT VMotionInit();
	VMOTION_API VMOTION_RESULT VMotionShutdown();
	VMOTION_API int VMotionGetControllerCount();
	VMOTION_API int VMotionGetState(unsigned int idx, VMOTION_STATE* state, unsigned int blocking = 0);

	// Useful for Unity, for instance
	VMOTION_API int VMotionGetStateUnstruct(unsigned int idx,
		float* orientation,
		float* acceleration,
		unsigned int* button,
		unsigned int* packetNumber,
		unsigned int blocking = 0);

}

#endif //__V_MOTION__H_