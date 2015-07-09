#include "VMotionController.h"
#include "internals\debughelpers.h"
#include "internals\endianhelpers.h"

VMotionController::VMotionController(const wchar_t* device_path)
	: IVMotionController()
	, mServiceHandle(INVALID_HANDLE_VALUE)
	, mNumCharacteristics(0)
	, mCharacteristics(nullptr)
	, mEventHandle(INVALID_HANDLE_VALUE)
	, mValueChangedEvent(nullptr)
{
	size_t len = wcslen(device_path) + 1;
	mDevicePath = new wchar_t[len];
	memcpy(mDevicePath, device_path, len * sizeof(wchar_t));

	connect();
}

VMotionController::~VMotionController()
{
	disconnect();
	delete mDevicePath;
}

bool VMotionController::doConnect()
{
	mServiceHandle = CreateFile(mDevicePath,
		FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

	if (mServiceHandle == INVALID_HANDLE_VALUE)
	{
		debug_print("Could not open port\n");
		return false;
	}

	// Query the required size for the structures.
	USHORT required_size = 0;
	BluetoothGATTGetCharacteristics(mServiceHandle, nullptr, 0, nullptr, &required_size, BLUETOOTH_GATT_FLAG_NONE);

	// HRESULT will never be S_OK here, so just check the size.
	if (required_size == 0)
		return false;

	// Allocate the characteristic structures.
	mCharacteristics = (PBTH_LE_GATT_CHARACTERISTIC) malloc(required_size * sizeof(BTH_LE_GATT_CHARACTERISTIC));

	// Get the characteristics offered by this service.
	HRESULT hr = BluetoothGATTGetCharacteristics(mServiceHandle, nullptr, required_size, mCharacteristics, &mNumCharacteristics, BLUETOOTH_GATT_FLAG_NONE);

	if (SUCCEEDED(hr))
	{
		// Configure the characteristics.
		for (int i = 0; i < mNumCharacteristics; i++)
		{
			if (mCharacteristics[i].CharacteristicUuid.Value.LongUuid == VMOTION_DATA_CH_UUID)
			{
				configureCharacteristic(&mCharacteristics[i], true, false);
			}
			else if (mCharacteristics[i].CharacteristicUuid.Value.LongUuid == VMOTION_CALIB_ACC_CH_UUID)
			{
				readCharacteristic(&mCharacteristics[i], &mAccCalibReport, sizeof(MAXMIN_CALIB_REPORT));
			}
			else if (mCharacteristics[i].CharacteristicUuid.Value.LongUuid == VMOTION_CALIB_MAG_CH_UUID)
			{
				readCharacteristic(&mCharacteristics[i], &mMagCalibReport, sizeof(MAXMIN_CALIB_REPORT));
			}
			else if (mCharacteristics[i].CharacteristicUuid.Value.LongUuid == VMOTION_CALIB_GYRO_CH_UUID)
			{
				readCharacteristic(&mCharacteristics[i], &mGyroCalibReport, sizeof(OFFSET_CALIB_REPORT));
			}
			
			this->loadCalibrationFromReport();
		}

		// Allocate the value changed structures. 2 because 2 characteristics??
		mValueChangedEvent = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION)
			malloc(sizeof(PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION) + VMOTION_BLE_NUM_CH * sizeof(BTH_LE_GATT_CHARACTERISTIC));

		// Register for event callbacks on the characteristics. Yep.
		mValueChangedEvent->NumCharacteristics = VMOTION_BLE_NUM_CH;
		memcpy(&mValueChangedEvent->Characteristics, mCharacteristics, VMOTION_BLE_NUM_CH * sizeof(BTH_LE_GATT_CHARACTERISTIC));
		HRESULT hr = BluetoothGATTRegisterEvent(mServiceHandle, CharacteristicValueChangedEvent, mValueChangedEvent,
			VMotionController::OnCharacteristicChanged, this, &mEventHandle, BLUETOOTH_GATT_FLAG_NONE);

		return SUCCEEDED(hr);
	}

	return false;
}

void VMotionController::doDisconnect()
{
	if (mEventHandle != INVALID_HANDLE_VALUE)
		BluetoothGATTUnregisterEvent(mEventHandle, BLUETOOTH_GATT_FLAG_NONE);
	mEventHandle = INVALID_HANDLE_VALUE;

	if (mValueChangedEvent != nullptr)
		free(mValueChangedEvent);
	mValueChangedEvent = nullptr;

	if (mServiceHandle != INVALID_HANDLE_VALUE)
		CloseHandle(mServiceHandle);
	mServiceHandle = INVALID_HANDLE_VALUE;

	if (mCharacteristics != nullptr)
		free(mCharacteristics);
	mCharacteristics = nullptr;
}

void VMotionController::OnCharacteristicChanged(BTH_LE_GATT_EVENT_TYPE event_type, void* event_out, void* context)
{
	VMotionController* controller = (VMotionController*)context;

	// Normally we would get this parameter from event_out, but it looks like it is an invalid pointer.
	// However it seems the event struct we allocated is being kept up-to-date, so we'll just use that.
	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION changed_event = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION)controller->mValueChangedEvent;

	std::lock_guard<std::mutex> lk(controller->mDataMutex);

	// Read all characteristics we're monitoring.
	for (int i = 0; i < changed_event->NumCharacteristics; i++)
	{
		PBTH_LE_GATT_CHARACTERISTIC characteristic = &changed_event->Characteristics[i];

		if (characteristic->CharacteristicUuid.Value.LongUuid == VMOTION_DATA_CH_UUID)
		{
			controller->readCharacteristic(characteristic, &controller->mSensorReport, sizeof(SENSOR_REPORT));
		}
	}

	controller->updateState();
	controller->mDataBlock.notify_all();
}

void VMotionController::saveCalibrationReport()
{

}

//uint8_t Glove::GetFlags()
//{
//	return m_calib.flags;
//}
//
//void Glove::SetFlags(uint8_t flags)
//{
//	m_calib.flags = flags;
//
//	for (int i = 0; i < m_num_characteristics; i++)
//	{
//		if (m_characteristics[i].CharacteristicUuid.Value.ShortUuid == BLE_UUID_MANUS_GLOVE_CALIB)
//			WriteCharacteristic(&m_characteristics[i], &m_calib, sizeof(CALIB_REPORT));
//	}
//}

bool VMotionController::readCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* dest, size_t length)
{
	// Query the required size for the structure.
	USHORT required_size = 0;
	BluetoothGATTGetCharacteristicValue(mServiceHandle, characteristic, 0, nullptr, &required_size, BLUETOOTH_GATT_FLAG_NONE);

	// HRESULT will never be S_OK here, so just check the size.
	if (required_size == 0)
		return false;

	// Allocate the characteristic value structure.
	PBTH_LE_GATT_CHARACTERISTIC_VALUE value = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(required_size);

	// Read the characteristic value.
	USHORT actual_size = 0;
	HRESULT hr = BluetoothGATTGetCharacteristicValue(mServiceHandle, characteristic, required_size, value, &actual_size, BLUETOOTH_GATT_FLAG_NONE);

	// Ensure there is enough room in the buffer.
	if (SUCCEEDED(hr) && length >= value->DataSize)
		memcpy(dest, &value->Data, value->DataSize);

	free(value);
	return SUCCEEDED(hr);
}

bool VMotionController::writeCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, void* src, size_t length)
{
	// Allocate the characteristic value structure.
	PBTH_LE_GATT_CHARACTERISTIC_VALUE value = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)
		malloc(length + sizeof(PBTH_LE_GATT_CHARACTERISTIC_VALUE));

	// Initialize the value structure.
	value->DataSize = length;
	memcpy(value->Data, src, length);

	// Write the characteristic value.
	USHORT actual_size = 0;
	HRESULT hr = BluetoothGATTSetCharacteristicValue(mServiceHandle, characteristic, value, 0, BLUETOOTH_GATT_FLAG_NONE);

	free(value);
	return SUCCEEDED(hr);
}

bool VMotionController::configureCharacteristic(PBTH_LE_GATT_CHARACTERISTIC characteristic, bool notify, bool indicate)
{
	// Query the required size for the structure.
	USHORT required_size = 0;
	BluetoothGATTGetDescriptors(mServiceHandle, characteristic, 0, nullptr, &required_size, BLUETOOTH_GATT_FLAG_NONE);

	// HRESULT will never be S_OK here, so just check the size.
	if (required_size == 0)
		return false;

	// Allocate the descriptor structures.
	PBTH_LE_GATT_DESCRIPTOR descriptors = (PBTH_LE_GATT_DESCRIPTOR)malloc(required_size * sizeof(BTH_LE_GATT_DESCRIPTOR));

	// Get the descriptors offered by this characteristic.
	USHORT actual_size = 0;
	HRESULT hr = BluetoothGATTGetDescriptors(mServiceHandle, characteristic, required_size, descriptors, &actual_size, BLUETOOTH_GATT_FLAG_NONE);
	if (SUCCEEDED(hr))
	{
		for (int i = 0; i < actual_size; i++)
		{
			// Look for the client configuration.
			if (descriptors[i].DescriptorType == ClientCharacteristicConfiguration)
			{
				// Configure this characteristic.
				BTH_LE_GATT_DESCRIPTOR_VALUE value;
				memset(&value, 0, sizeof(value));
				value.DescriptorType = ClientCharacteristicConfiguration;
				value.ClientCharacteristicConfiguration.IsSubscribeToNotification = notify;
				value.ClientCharacteristicConfiguration.IsSubscribeToIndication = indicate;

				HRESULT hr = BluetoothGATTSetDescriptorValue(mServiceHandle, &descriptors[i], &value, BLUETOOTH_GATT_FLAG_NONE);

				return SUCCEEDED(hr);
			}
		}
	}

	return false;
}