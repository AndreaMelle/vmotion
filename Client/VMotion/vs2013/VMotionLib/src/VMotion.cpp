#include "VMotion.h"
#include "internals\debughelpers.h"
#include "internals\enumser.h"
#include <mutex>
#include "VMotionController.h"
#include "internals\Devices.h"
#include "internals\WinDevices.h"

bool gInitialized = false;

std::vector<VMotionController*> gMotionControllers;
std::mutex gControllersMutex;

Devices* gDevices;

VMOTION_STATE *gState = nullptr; //temp copyto state

void DeviceConnected(const wchar_t* device_path)
{
	std::lock_guard<std::mutex> lock(gControllersMutex);

	// Check if the controller already exists
	for (VMotionController* elem : gMotionControllers)
	{
		if (wcscmp(device_path, elem->getDevicePath()) == 0)
		{
			// The controller was previously connected, reconnect it
			elem->connect();
			return;
		}
	}

	// The controller hasn't been connected before, add it to the list of gloves
	gMotionControllers.push_back(new VMotionController(device_path));
}

VMOTION_RESULT VMotionInit()
{
	if (gInitialized)
	{
		debug_print("VMotion initialized already. Shutting down and re-init.");
		VMotionShutdown();
	}

	std::lock_guard<std::mutex> lock(gControllersMutex);

	// Get a list of Services.
	HDEVINFO device_info_set = SetupDiGetClassDevs(&VMOTION_SERVICE_UUID, nullptr, nullptr,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (device_info_set)
	{
		SP_DEVICE_INTERFACE_DATA device_interface_data = { 0 };
		device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		int device_index = 0;
		while (SetupDiEnumDeviceInterfaces(device_info_set, nullptr, &VMOTION_SERVICE_UUID,
			device_index, &device_interface_data))
		{
			DWORD required_size = 0;

			// Query the required size for the structure.
			SetupDiGetDeviceInterfaceDetail(device_info_set, &device_interface_data, nullptr,
				0, &required_size, nullptr);

			// HRESULT will never be S_OK here, so just check the size.
			if (required_size > 0)
			{
				// Allocate the interface detail structure.
				SP_DEVICE_INTERFACE_DETAIL_DATA* device_interface_detail_data = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(required_size);
				device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

				// Get the detailed device data which includes the device path.
				if (SetupDiGetDeviceInterfaceDetail(device_info_set, &device_interface_data, device_interface_detail_data,
					required_size, nullptr, nullptr))
				{
					gMotionControllers.push_back(new VMotionController(device_interface_detail_data->DevicePath));
				}

				free(device_interface_detail_data);
			}

			device_index++;
		}

		SetupDiDestroyDeviceInfoList(device_info_set);
	}

#ifdef _WIN32
	gDevices = new WinDevices();
	gDevices->SetDeviceConnected(DeviceConnected);
#endif

	gState = new VMOTION_STATE();

	gInitialized = true;

	return VMOTION_SUCCESS;
}

VMOTION_RESULT VMotionShutdown()
{
	std::lock_guard<std::mutex> lock(gControllersMutex);

	for (VMotionController* mc : gMotionControllers)
	{
		delete mc;
	}
	
#ifdef _WIN32
	delete gDevices;
#endif

	if (gState)
	{
		delete gState;
		gState = nullptr;
	}

	gInitialized = false;

	return VMOTION_SUCCESS;
}

int VMotionGetControllerCount()
{
	return (int)gMotionControllers.size();
}

VMOTION_RESULT GetVMotionController(unsigned int idx, VMotionController** elem)
{
	std::lock_guard<std::mutex> lock(gControllersMutex);

	if (idx >= gMotionControllers.size())
		return VMOTION_OUT_OF_RANGE;

	if (!gMotionControllers[idx]->isConnected())
		return VMOTION_DISCONNECTED;

	*elem = gMotionControllers[idx];

	return VMOTION_SUCCESS;
}

int VMotionGetState(unsigned int idx, VMOTION_STATE* state, unsigned int blocking)
{
	// Get the glove from the list
	VMotionController* elem;
	int ret = GetVMotionController(idx, &elem);
	if (ret != VMOTION_SUCCESS)
		return ret;

	if (!state)
		return VMOTION_INVALID_ARGUMENT;

	return elem->getState(state, blocking) ? VMOTION_SUCCESS : VMOTION_ERROR;
}

int VMotionGetStateUnstruct(unsigned int idx,
	float* orientation,
	float* acceleration,
	unsigned int* button,
	unsigned int* packetNumber,
	unsigned int blocking)
{
	VMOTION_STATE state = { 0 };
	int res = VMotionGetState(idx, &state, blocking);

	if (res == VMOTION_SUCCESS)
	{
		memcpy(orientation, &state.data.orientation, sizeof(VMOTION_VECTOR));
		memcpy(acceleration, &state.data.acceleration, sizeof(VMOTION_VECTOR));
		button[0] = (state.data.button) ? 1 : 0;
		packetNumber[0] = state.packetNumber;
	}

	return res;

}



