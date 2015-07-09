#include "IVMotionController.h"
#include "internals\debughelpers.h"
#include "internals\endianhelpers.h"

IVMotionController::IVMotionController()
	: mConnected(false)
	, mSensorFusionNeedsReset(true)
{
	memset(&mSensorReport, 0, sizeof(SENSOR_REPORT));
	memset(&mAccCalibReport, 0, sizeof(MAXMIN_CALIB_REPORT));
	memset(&mMagCalibReport, 0, sizeof(MAXMIN_CALIB_REPORT));
	memset(&mGyroCalibReport, 0, sizeof(OFFSET_CALIB_REPORT));
	mState.packetNumber = 0;

	memset(&mState.data, 0, sizeof(VMOTION_DATA));
}

IVMotionController::~IVMotionController()
{
	disconnect();
}

void IVMotionController::connect()
{
	if (isConnected())
	{
		disconnect();
	}

	mConnected = doConnect();

	mSensorFusionNeedsReset = true;
}

void IVMotionController::disconnect()
{
	mConnected = false;
	doDisconnect();
}

void IVMotionController::updateState()
{
	//1. cast the report to float
	Eigen::Vector3f rawAcc;
	Eigen::Vector3f rawMag;
	Eigen::Vector3f rawGyro;
	Eigen::Vector3f calibAcc;
	Eigen::Vector3f calibMag;
	Eigen::Vector3f calibGyro;
	Eigen::Vector3f ypr;

	for (register uint8_t i = 0; i < VMOTION_AXIS_DIM; ++i)
	{
		rawAcc(i) = static_cast<float>(mSensorReport.accelerometer[i]);
		rawMag(i) = static_cast<float>(mSensorReport.magnetometer[i]);
		rawGyro(i) = static_cast<float>(mSensorReport.gyroscope[i]);
	}
	
	//2. apply calibration

	calibAcc(0) = (rawAcc(0) - mCalibration.accOffset.x) * mCalibration.accScale.x;
	calibAcc(1) = (rawAcc[1] - mCalibration.accOffset.y) * mCalibration.accScale.y;
	calibAcc(2) = (rawAcc(2) - mCalibration.accOffset.z) * mCalibration.accScale.z;

#if CALIBRATION__MAGN_USE_EXTENDED
	for (int i = 0; i < 3; i++)
		magnetom_tmp[i] = magnetom[i] - magn_ellipsoid_center[i];
	Matrix_Vector_Multiply(magn_ellipsoid_transform, magnetom_tmp, magnetom);
#else
	calibMag(0) = (rawMag(0) - mCalibration.magOffset.x) * mCalibration.magScale.x;
	calibMag(1) = (rawMag(1) - mCalibration.magOffset.y) * mCalibration.magScale.y;
	calibMag(2) = (rawMag(2) - mCalibration.magOffset.z) * mCalibration.magScale.z;
#endif

	calibGyro(0) -= mCalibration.gyroOffset.x;
	calibGyro(1) -= mCalibration.gyroOffset.y;
	calibGyro(2) -= mCalibration.gyroOffset.z;

	//3. run DCM sensor fusion
	if (mSensorFusionNeedsReset)
	{	
		mSensorFusion.reset(calibAcc, calibMag);
	}

	mSensorFusion.run(calibAcc, calibMag, calibGyro, ypr);

	//4. update state
	mState.data.button = (mSensorReport.btn > 0) ? true : false;

	mState.data.acceleration.x = calibAcc(0);
	mState.data.acceleration.y = calibAcc(1);
	mState.data.acceleration.z = calibAcc(2);

	mState.data.orientation.x = ypr(0);
	mState.data.orientation.y = ypr(1);
	mState.data.orientation.z = ypr(2);

	mState.packetNumber++;

}

void IVMotionController::loadCalibrationFromReport()
{
	mCalibration.fromReport(mAccCalibReport, mMagCalibReport, mGyroCalibReport);
}

bool IVMotionController::getState(VMOTION_STATE* state, unsigned int timeout)
{
	// Wait until the thread is done writing a packet
	std::unique_lock<std::mutex> lk(mDataMutex);

	// Optionally wait until the next package is sent
	if (timeout > 0)
	{
		mDataBlock.wait_for(lk, std::chrono::milliseconds(timeout));
		if (!mConnected)
		{
			lk.unlock();
			return false;
		}
	}

	*state = mState;

	lk.unlock();

	return mState.packetNumber > 0;
}

VMotionCalibration::VMotionCalibration()
{
	accOffset.x = accOffset.y = accOffset.z = 0;
	magOffset.x = magOffset.y = magOffset.z = 0;
	gyroOffset.x = gyroOffset.y = gyroOffset.z = 0;
	accScale.x = accScale.y = accScale.z = 1.0f;
	magScale.x = magScale.y = magScale.z = 1.0f;
}

VMotionCalibration::~VMotionCalibration()
{

}

void VMotionCalibration::fromReport(const MAXMIN_CALIB_REPORT& accReport,
	const MAXMIN_CALIB_REPORT& magReport,
	const OFFSET_CALIB_REPORT& gyroReport)
{
	accOffset.x = (accReport.min[0] + accReport.max[0]) / 2.0f;
	accOffset.y = (accReport.min[1] + accReport.max[1]) / 2.0f;
	accOffset.z = (accReport.min[2] + accReport.max[2]) / 2.0f;

	accScale.x = GRAVITY / (accReport.max[0] - accOffset.x);
	accScale.y = GRAVITY / (accReport.max[1] - accOffset.y);
	accScale.z = GRAVITY / (accReport.max[2] - accOffset.z);

	magOffset.x = (magReport.min[0] + magReport.max[0]) / 2.0f;
	magOffset.y = (magReport.min[1] + magReport.max[1]) / 2.0f;
	magOffset.z = (magReport.min[2] + magReport.max[2]) / 2.0f;

	magScale.x = 100.0f / (magReport.max[0] - magOffset.x);
	magScale.y = 100.0f / (magReport.max[1] - magOffset.y);
	magScale.z = 100.0f / (magReport.max[2] - magOffset.z);

	gyroOffset.x = gyroReport.offset[0];
	gyroOffset.y = gyroReport.offset[1];
	gyroOffset.z = gyroReport.offset[2];
}

void VMotionCalibration::toReport(MAXMIN_CALIB_REPORT& accReport,
	MAXMIN_CALIB_REPORT& magReport,
	OFFSET_CALIB_REPORT& gyroReport)
{
	// TODO: when we implement calibration logic....
}
