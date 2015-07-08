#ifndef __V_MOTION_CONTROLLER_H__
#define __V_MOTION_CONTROLLER_H__

#include "internals\commonheaders.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <chrono>
#include <condition_variable>
#include "VMotion.h"
#include "IVMotionController.h"

static const GUID VMOTION_SERVICE_UUID;//TODO = { 0x1bc50001, 0x0200, 0xeca1, { 0xe4, 0x11, 0x20, 0xfa, 0xc0, 0x4a, 0xfa, 0x8f } };

class VMotionController : public IVMotionController
{
public:

	VMotionController(const wchar_t* device_path);
	virtual ~VMotionController();

	const wchar_t* getDevicePath() const { return mDevicePath; }

	/*uint8_t GetFlags();
	void SetFlags(uint8_t flags);*/

private:

	wchar_t* mDevicePath;

	HANDLE mServiceHandle;
	USHORT mNumCharacteristics;
	PBTH_LE_GATT_CHARACTERISTIC mCharacteristics;
	BLUETOOTH_GATT_EVENT_HANDLE mEventHandle;
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION mValueChangedEvent;

	static void OnCharacteristicChanged(BTH_LE_GATT_EVENT_TYPE event_type, void* event_out, void* context);
	bool readCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* dest, size_t length);
	bool writeCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* src, size_t length);
	bool configureCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, bool notify, bool indicate);

	virtual void saveCalibrationReport();
	virtual bool doConnect();
	virtual void doDisconnect();

};

#endif //__V_MOTION_CONTROLLER_H__