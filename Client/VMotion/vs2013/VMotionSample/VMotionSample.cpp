#include "VMotion.h"
#include "VMotionController.h"
#include <stdio.h>
#include <signal.h>

bool gStop;

void SigInt_Handler(int n_signal)
{
	VMotionShutdown();
	exit(0);
}

void SigBreak_Handler(int n_signal)
{
	VMotionShutdown();
	exit(0);
}

void OnVMotionControllerConnect(void* sender, VMOTION_RESULT connected)
{
	if (VMOTION_SUCCESS == connected)
	{
		printf("Successfully connected to controller\n");
	}
	else
	{
		printf("Failed to connect to controller. Timeout\n");
		gStop = true;
	}
}

int main(int argc, char* argv[])
{
	gStop = false;

	signal(SIGINT, &SigInt_Handler);
	signal(SIGBREAK, &SigBreak_Handler);

	VMotionInit(&OnVMotionControllerConnect);
	
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	while (!gStop)
	{
		for (int i = 0; i < VMotionGetControllerCount(); i++)
		{
			LARGE_INTEGER start, end, elapsed;
			QueryPerformanceCounter(&start);

			VMOTION_VECTOR state = { 0 };

			if (VMotionGetYPR(i, state, 1000) == VMOTION_SUCCESS)
			{
				QueryPerformanceCounter(&end);
				elapsed.QuadPart = end.QuadPart - start.QuadPart;
				printf("interval: %fms\n", (elapsed.QuadPart * 1000) / (double)freq.QuadPart);
				printf("YPR: x: % 1.5f; y: % 1.5f; z: % 1.5f\n", state.x, state.y, state.z);
			}
			else
			{
				QueryPerformanceCounter(&end);
			}
		}
	}

	VMotionShutdown();

	return 0;
}

