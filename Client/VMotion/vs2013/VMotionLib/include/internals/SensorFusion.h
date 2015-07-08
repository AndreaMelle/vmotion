#ifndef __SENSOR_FUSION_H__
#define __SENSOR_FUSION_H__

#include <chrono>
#include "Eigen/Dense"
#include "Eigen/Geometry"

#define GRAVITY 256.0f // "1G reference" used for DCM filter and accelerometer calibration
#define GYRO_GAIN 0.06957f // Same gain on all axes
#define GYRO_SCALED_RAD(x) (x * TO_RAD(GYRO_GAIN)) // Calculate the scaled gyro readings in radians per second
#define TO_RAD(x) (x * 0.01745329252f)  // *pi/180
#define TO_DEG(x) (x * 57.2957795131f)  // *180/pi
#define Kp_ROLLPITCH 0.02f
#define Ki_ROLLPITCH 0.00002f
#define Kp_YAW 1.2f
#define Ki_YAW 0.00002f

class SensorFusion
{
public:
	SensorFusion();
	virtual ~SensorFusion();

	// only call after the first data has arrived
	void reset(const Eigen::Vector3f& accel, const Eigen::Vector3f& magnetom);
	void run(const Eigen::Vector3f& accel, const Eigen::Vector3f& magnetom, const Eigen::Vector3f& gyro, float ypr[3]);

private:
	std::chrono::high_resolution_clock::time_point timestamp;
	std::chrono::high_resolution_clock::time_point timestampPrevious;
	std::chrono::duration<float> d;
	std::chrono::high_resolution_clock highResClock;

	float yaw;
	float pitch;
	float roll;

	float magHeading;
	Eigen::Vector3f Gyro_Vector; // Store the gyros turn rate in a vector
	Eigen::Vector3f Accel_Vector; // Store the acceleration in a vector
	Eigen::Vector3f Omega_Vector; // Corrected Gyro_Vector data
	Eigen::Vector3f Omega_P; // Omega Proportional correction
	Eigen::Vector3f Omega_I; // Omega Integrator
	Eigen::Vector3f Omega;
	Eigen::Vector3f errorRollPitch;
	Eigen::Vector3f errorYaw;

	Eigen::Matrix3f dcmMatrix;
	Eigen::Matrix3f updateMatrix;
	Eigen::Matrix3f temporaryMatrix;

	float G_Dt;

	void compassHeading(const Eigen::Vector3f& magnetom);
	void matrixUpdate(const Eigen::Vector3f& accel, const Eigen::Vector3f& gyro);
	void normalize();
	void driftCorrection();
	void eluerAngles();
};

inline void init_rotation_matrix(Eigen::Matrix3f& m, float yaw, float pitch, float roll)
{
	float c1 = cos(roll);
	float s1 = sin(roll);
	float c2 = cos(pitch);
	float s2 = sin(pitch);
	float c3 = cos(yaw);
	float s3 = sin(yaw);

	// Euler angles, right-handed, intrinsic, XYZ convention
	// (which means: rotate around body axes Z, Y', X'') 
	m(0) = c2 * c3;
	m(1) = c3 * s1 * s2 - c1 * s3;
	m(2) = s1 * s3 + c1 * c3 * s2;

	m(3) = c2 * s3;
	m(4) = c1 * c3 + s1 * s2 * s3;
	m(5) = c1 * s2 * s3 - c3 * s1;

	m(6) = -s2;
	m(7) = c2 * s1;
	m(8) = c1 * c2;
}

inline float constrain(const float& x, const float& a, const float& b)
{
	if (x < a)
	{
		return a;
	}
	else if (x > b)
	{
		return b;
	}

	return x;
}

#endif //__SENSOR_FUSION_H__