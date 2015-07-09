#ifndef __I_VMOTION_CONTROLLER_H__
#define __I_VMOTION_CONTROLLER_H__

#include "internals\commonheaders.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <chrono>
#include <condition_variable>
#include "VMotion.h"
#include "internals\SensorFusion.h"

static const GUID VMOTION_SERVICE_UUID = { 0x34a189eb, 0x3b60, 0x4c6d, { 0xbb, 0xbb, 0x54, 0x59, 0xa1, 0xda, 0x4e, 0xf5 } };
static const GUID VMOTION_DATA_CH_UUID = { 0x93f8c6c1, 0x6910, 0x496e, { 0xa3, 0x8f, 0x2e, 0x2f, 0x4f, 0xb2, 0xbf, 0x37 } };
static const GUID VMOTION_CALIB_ACC_CH_UUID = { 0x8f9451b1, 0x60e8, 0x4ff4, { 0xa2, 0x23, 0x8b, 0xde, 0x73, 0x5e, 0xf6, 0x13 } };
static const GUID VMOTION_CALIB_MAG_CH_UUID = { 0xc0cd1bee, 0xdf21, 0x4a59, { 0xbf, 0x80, 0xcf, 0xd9, 0x2e, 0x7a, 0x6c, 0x14 } };
static const GUID VMOTION_CALIB_GYRO_CH_UUID = { 0x23344fae, 0xc0f5, 0x4bef, { 0xb3, 0x6f, 0x64, 0x1a, 0xb2, 0x63, 0x3a, 0xc4 } };

/*
* Defines the VMotionController interface, with common cross-platform code
*/

#define VMOTION_CALIB_DIM 2
#define DEFAULT_QUERY_TIMEOUT 500 //ms
#define VMOTION_BLE_NUM_CH 1

#pragma pack(push, 1) // exact fit - no padding

typedef struct SENSOR_REPORT
{
	int16_t accelerometer[VMOTION_AXIS_DIM];
	int16_t magnetometer[VMOTION_AXIS_DIM];
	int16_t gyroscope[VMOTION_AXIS_DIM];
	uint8_t btn;
} SENSOR_REPORT;

typedef struct MAXMIN_CALIB_REPORT
{
	int16_t min[VMOTION_AXIS_DIM]; //xmin, ymin, zmin
	int16_t max[VMOTION_AXIS_DIM]; //xmax, ymax, zmax
} MAXMIN_CALIB_REPORT;

typedef struct OFFSET_CALIB_REPORT
{
	int16_t offset[VMOTION_AXIS_DIM]; //xoffet, yoffset, zoffset
} OFFSET_CALIB_REPORT;

#pragma pack(pop)

class VMotionCalibration
{
public:
	VMotionCalibration();
	virtual ~VMotionCalibration();
	void fromReport(const MAXMIN_CALIB_REPORT& accReport,
		const MAXMIN_CALIB_REPORT& magReport,
		const OFFSET_CALIB_REPORT& gyroReport);

	void toReport(MAXMIN_CALIB_REPORT& accReport,
		MAXMIN_CALIB_REPORT& magReport,
		OFFSET_CALIB_REPORT& gyroReport);

	VMOTION_VECTOR accOffset;
	VMOTION_VECTOR accScale;
	VMOTION_VECTOR magOffset;
	VMOTION_VECTOR magScale;
	VMOTION_VECTOR gyroOffset;
};

class IVMotionController
{
public:
	IVMotionController();
	virtual ~IVMotionController();

	void connect();
	void disconnect();
	bool isConnected() const { return mConnected; }

	bool getState(VMOTION_STATE* state, unsigned int timeout = DEFAULT_QUERY_TIMEOUT);

protected:
	std::atomic<bool> mConnected;

	// Higher level, interpreted data
	VMOTION_STATE mState;
	VMotionCalibration mCalibration;
	SensorFusion mSensorFusion;
	bool mSensorFusionNeedsReset;

	// Raw data received from the sensor
	SENSOR_REPORT mSensorReport;
	MAXMIN_CALIB_REPORT mAccCalibReport;
	MAXMIN_CALIB_REPORT mMagCalibReport;
	OFFSET_CALIB_REPORT mGyroCalibReport;

	std::mutex mDataMutex;
	std::condition_variable mDataBlock;

	// call after you have received a data charactersitcs update and you've updated the reports accordingly
	void updateState();

	//call after you have read the reports characteristics (which is platform dependent)
	void loadCalibrationFromReport();

	// saving the calibration report is platform dependent
	virtual void saveCalibrationReport() = 0;

	// platform dependent connect and disconnect
	virtual bool doConnect() = 0;
	virtual void doDisconnect() = 0;
};

#endif //__I_VMOTION_CONTROLLER_H__