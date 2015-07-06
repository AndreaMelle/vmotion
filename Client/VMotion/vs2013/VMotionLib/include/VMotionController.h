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

static const GUID VMOTION_SERVICE_UUID;//TODO = { 0x1bc50001, 0x0200, 0xeca1, { 0xe4, 0x11, 0x20, 0xfa, 0xc0, 0x4a, 0xfa, 0x8f } };

#define VMOTION_BLE_NUM_CH 1

#define VMOTION_BLE_SE 0x0001 //TODO
#define VMOTION_YPRB_BLE_CH 0x0002 //TODO
//#define VMOTION_YPRB_BLE_CH 0x0003 //TODO
//#define VMOTION_YPRB_BLE_CH 0x0004 //TODO

/*
* Defines a VMotionController
* Cannot be instantiate directly, as the device path would be needed
* Use the static functions to query one instead
*/

#define DEFAULT_QUERY_TIMEOUT 500 //ms
#define YPR_DATA_DIVISOR 1.0f

enum VMotionMode
{
	YAW_PITCH_ROLL,
	ACC_MAG_GYR_RAW,
	ACC_MAG_GYR_CALIBRATED
};

#pragma pack(push, 1) // exact fit - no padding
typedef struct YPRB_REPORT
{
	int16_t ypr[VMOTION_VECTOR_SZ];
	uint8_t btn;
} YPRB_REPORT;

typedef struct SENSOR_REPORT
{
	int16_t accelerometer[VMOTION_VECTOR_SZ];
	int16_t magnetometer[VMOTION_VECTOR_SZ];
	int16_t gyroscope[VMOTION_VECTOR_SZ];
} SENSOR_REPORT;

#pragma pack(pop)

class VMotionController
{
public:

	//VMotionControllerConnectionCB fn = NULL
	//, VMotionMode mode = YAW_PITCH_ROLL, double connect_timeout_s = 5.0
	VMotionController(const wchar_t* device_path);
	virtual ~VMotionController();

	void connect();
	void disconnect();
	bool isConnected() const { return mConnected; }

	//VMOTION_RESULT getYawPitchRoll(VMOTION_VECTOR& result, unsigned int timeout = DEFAULT_QUERY_TIMEOUT);
	//VMOTION_RESULT getSensorData(VMOTION_SENSOR_DATA& result, unsigned int timeout = DEFAULT_QUERY_TIMEOUT);

	const wchar_t* getDevicePath() const { return mDevicePath; }

	bool getState(VMOTION_STATE* state, unsigned int timeout = DEFAULT_QUERY_TIMEOUT);

	/*uint8_t GetFlags();
	void SetFlags(uint8_t flags);*/

private:
	std::atomic<bool> mConnected;

	//VMOTION_VECTOR mYawPitchRollState;
	//VMOTION_SENSOR_DATA mSensorDataState;
	VMOTION_STATE mState;
	YPRB_REPORT mYPRBReport;
	SENSOR_REPORT mSensorReport;

	wchar_t* mDevicePath;

	HANDLE mServiceHandle;
	USHORT mNumCharacteristics;
	PBTH_LE_GATT_CHARACTERISTIC mCharacteristics;
	BLUETOOTH_GATT_EVENT_HANDLE mEventHandle;
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION mValueChangedEvent;

	std::mutex mDataMutex;
	std::condition_variable mDataBlock;

	static void OnCharacteristicChanged(BTH_LE_GATT_EVENT_TYPE event_type, void* event_out, void* context);
	bool readCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* dest, size_t length);
	bool writeCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* src, size_t length);
	bool configureCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, bool notify, bool indicate);

	void updateState();

	//VMotionMode mMode;

	//std::shared_ptr<std::thread> mDeviceThread;
	//std::atomic<bool> mDeviceThreadRunning;
	

	//std::chrono::duration<double> mConnectTimeoutS;

	//VMotionControllerConnectionCB mConnectionCB;

	/*void device_thread_main();
	bool init_controller_helper();
	SERIAL_READ_RESULT read_byte_helper(char& result);
	bool write_helper(BYTE* output_buffer, DWORD bytes_to_write);
	bool write_helper(const std::string& data);
	bool read_token(const std::string &token, char c);*/


};

#endif //__V_MOTION_CONTROLLER_H__