#include "internals/SensorFusion.h"
#include <iostream>

SensorFusion::SensorFusion()
{
	dcmMatrix = Eigen::Matrix3f::Identity();
	temporaryMatrix = Eigen::Matrix3f::Zero();
	Gyro_Vector = Eigen::Vector3f::Zero();
	Accel_Vector = Eigen::Vector3f::Zero();
	Omega_Vector = Eigen::Vector3f::Zero();
	Omega_P = Eigen::Vector3f::Zero();
	Omega_I = Eigen::Vector3f::Zero();
	Omega = Eigen::Vector3f::Zero();
	errorYaw = Eigen::Vector3f::Zero();
	errorRollPitch = Eigen::Vector3f::Zero();
	updateMatrix << 0, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f;
}

SensorFusion::~SensorFusion() {}

void SensorFusion::run(const Eigen::Vector3f& accel, const Eigen::Vector3f& magnetom, const Eigen::Vector3f& gyro, float ypr[3])
{
	timestampPrevious = timestamp;
	timestamp = highResClock.now();

	d = timestamp - timestampPrevious;
	G_Dt = (float)std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / 1000.0f; // Real time of loop run. We use this on the DCM algorithm (gyro integration time)

	compassHeading(magnetom);
	matrixUpdate(accel, gyro);
	normalize();
	driftCorrection();
	eluerAngles();

	ypr[0] = yaw;
	ypr[1] = pitch;
	ypr[2] = roll;

}

void SensorFusion::reset(const Eigen::Vector3f& accel, const Eigen::Vector3f& magnetom)
{
	G_Dt = 0;

	Eigen::Vector3f temp1;
	Eigen::Vector3f temp2;
	Eigen::Vector3f xAxis;
	xAxis << 1.0f, 0.0f, 0.0f;

	timestamp = highResClock.now();

	// GET PITCH
	// Using y-z-plane-component/x-component of gravity vector
	pitch = -atan2f(accel[0], sqrt(accel[1] * accel[1] + accel[2] * accel[2]));

	// GET ROLL
	// Compensate pitch of gravity vector
	temp1 = xAxis.cross(accel);
	temp2 = xAxis.cross(temp1);

	// Normally using x-z-plane-component/y-component of compensated gravity vector
	// roll = atan2(temp2[1], sqrt(temp2[0] * temp2[0] + temp2[2] * temp2[2]));
	// Since we compensated for pitch, x-z-plane-component equals z-component:
	roll = atan2(temp2[1], temp2[2]);

	// GET YAW
	compassHeading(magnetom);
	yaw = magHeading;

	// Init rotation matrix
	init_rotation_matrix(dcmMatrix, yaw, pitch, roll);
}

void SensorFusion::matrixUpdate(const Eigen::Vector3f& accel, const Eigen::Vector3f& gyro)
{
	Gyro_Vector[0] = GYRO_SCALED_RAD(gyro[0]); //gyro x roll
	Gyro_Vector[1] = GYRO_SCALED_RAD(gyro[1]); //gyro y pitch
	Gyro_Vector[2] = GYRO_SCALED_RAD(gyro[2]); //gyro z yaw

	Accel_Vector[0] = accel[0];
	Accel_Vector[1] = accel[1];
	Accel_Vector[2] = accel[2];

	Omega = Gyro_Vector + Omega_I;
	Omega_Vector = Omega + Omega_P;

	updateMatrix(0) = 0;
	updateMatrix(1) = -G_Dt*Omega_Vector[2];//-z
	updateMatrix(2) = G_Dt*Omega_Vector[1];//y
	updateMatrix(3) = G_Dt*Omega_Vector[2];//z
	updateMatrix(4) = 0;
	updateMatrix(5) = -G_Dt*Omega_Vector[0];//-x
	updateMatrix(6) = -G_Dt*Omega_Vector[1];//-y
	updateMatrix(7) = G_Dt*Omega_Vector[0];//x
	updateMatrix(8) = 0;

	temporaryMatrix = dcmMatrix * updateMatrix;

	dcmMatrix += temporaryMatrix;
}

void SensorFusion::normalize()
{
	float error = 0;
	Eigen::Matrix3f temporary;
	float renorm = 0;

	error = -dcmMatrix.row(0).dot(dcmMatrix.row(1)) * 0.5f; //eq.19

	temporary.row(0) = dcmMatrix.row(1) * error;
	temporary.row(1) = dcmMatrix.row(0) * error;

	temporary.row(0) += dcmMatrix.row(0);
	temporary.row(1) += dcmMatrix.row(1);

	temporary.row(2) = temporary.row(0).cross(temporary.row(1)); // c= a x b //eq.20

	renorm = 0.5f *(3.0f - temporary.row(0).dot(temporary.row(0))); //eq.21

	dcmMatrix.row(0) = temporary.row(0) * renorm;

	renorm = 0.5f *(3.0f - temporary.row(1).dot(temporary.row(1))); //eq.21
	dcmMatrix.row(1) = temporary.row(1) * renorm;

	renorm = 0.5f *(3.0f - temporary.row(2).dot(temporary.row(2))); //eq.21
	dcmMatrix.row(2) = temporary.row(2) * renorm;
}

void SensorFusion::driftCorrection()
{
	float mag_heading_x;
	float mag_heading_y;
	float errorCourse;
	//Compensation the Roll, Pitch and Yaw drift. 
	static Eigen::Vector3f Scaled_Omega_P;
	static Eigen::Vector3f Scaled_Omega_I;
	float Accel_magnitude;
	float Accel_weight;


	//*****Roll and Pitch***************

	// Calculate the magnitude of the accelerometer vector
	Accel_magnitude = sqrt(Accel_Vector[0] * Accel_Vector[0] + Accel_Vector[1] * Accel_Vector[1] + Accel_Vector[2] * Accel_Vector[2]);
	Accel_magnitude = Accel_magnitude / GRAVITY; // Scale to gravity.
	// Dynamic weighting of accelerometer info (reliability filter)
	// Weight for accelerometer info (<0.5G = 0.0, 1G = 1.0 , >1.5G = 0.0)
	Accel_weight = constrain(1 - 2 * abs(1 - Accel_magnitude), 0, 1);  //  

	errorRollPitch = Accel_Vector.cross(dcmMatrix.row(2)); //adjust the ground of reference
	Omega_P = errorRollPitch * Kp_ROLLPITCH * Accel_weight;

	Scaled_Omega_I = errorRollPitch * Ki_ROLLPITCH * Accel_weight;
	Omega_I += Scaled_Omega_I;

	//*****YAW***************
	// We make the gyro YAW drift correction based on compass magnetic heading

	mag_heading_x = cos(magHeading);
	mag_heading_y = sin(magHeading);
	errorCourse = (dcmMatrix(0) * mag_heading_y) - (dcmMatrix(3) * mag_heading_x);  //Calculating YAW error
	errorYaw = dcmMatrix.row(2) * errorCourse; //Applys the yaw correction to the XYZ rotation of the aircraft, depeding the position.

	Scaled_Omega_P = errorYaw * Kp_YAW;//.01proportional of YAW.
	Omega_P += Scaled_Omega_P;//Adding  Proportional.

	Scaled_Omega_I = errorYaw * Ki_YAW;//.00001Integrator
	Omega_I += Scaled_Omega_I;//adding integrator to the Omega_I
}

void SensorFusion::eluerAngles()
{
	pitch = -asinf(dcmMatrix(6));
	roll = atan2f(dcmMatrix(7), dcmMatrix(8));
	yaw = atan2f(dcmMatrix(3), dcmMatrix(0));
}

void SensorFusion::compassHeading(const Eigen::Vector3f& magnetom)
{
	float mag_x;
	float mag_y;
	float cos_roll;
	float sin_roll;
	float cos_pitch;
	float sin_pitch;

	cos_roll = cos(roll);
	sin_roll = sin(roll);
	cos_pitch = cos(pitch);
	sin_pitch = sin(pitch);

	// Tilt compensated magnetic field X
	mag_x = magnetom[0] * cos_pitch + magnetom[1] * sin_roll * sin_pitch + magnetom[2] * cos_roll * sin_pitch;
	// Tilt compensated magnetic field Y
	mag_y = magnetom[1] * cos_roll - magnetom[2] * sin_roll;
	// Magnetic Heading
	magHeading = atan2(-mag_y, mag_x);
}