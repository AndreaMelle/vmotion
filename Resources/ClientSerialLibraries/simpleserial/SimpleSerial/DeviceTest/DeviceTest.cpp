// DeviceTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <Ntddser.h>
#include <Bthdef.h>
#include <Hidclass.h>
#include <memory>

void print_gatt_characteristics(wchar_t* m_device_path)
{
	HANDLE m_service_handle = CreateFile(m_device_path,
		FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);

	if (m_service_handle == INVALID_HANDLE_VALUE)
		return;

	// Query the required size for the structures.
	USHORT required_size = 0;
	BluetoothGATTGetCharacteristics(m_service_handle, nullptr, 0, nullptr,
		&required_size, BLUETOOTH_GATT_FLAG_NONE);

	// HRESULT will never be S_OK here, so just check the size.
	if (required_size == 0)
		return;

	// Allocate the characteristic structures.
	USHORT m_num_characteristics;
	PBTH_LE_GATT_CHARACTERISTIC m_characteristics = (PBTH_LE_GATT_CHARACTERISTIC)malloc(required_size * sizeof(BTH_LE_GATT_CHARACTERISTIC));

	// Get the characteristics offered by this service.
	HRESULT hr = BluetoothGATTGetCharacteristics(m_service_handle, nullptr, required_size, m_characteristics, &m_num_characteristics, BLUETOOTH_GATT_FLAG_NONE);

	if (SUCCEEDED(hr))
	{
		// Configure the characteristics.
		for (int i = 0; i < m_num_characteristics; i++)
		{

			wprintf(L"	Characteristic UUID:[%s]\n", m_characteristics[i].CharacteristicUuid);

			// Query the required size for the structure.
			USHORT required_size = 0;
			BluetoothGATTGetDescriptors(m_service_handle, &m_characteristics[i], 0, nullptr,
				&required_size, BLUETOOTH_GATT_FLAG_NONE);

			// HRESULT will never be S_OK here, so just check the size.
			if (required_size == 0)
				continue;

			// Allocate the descriptor structures.
			PBTH_LE_GATT_DESCRIPTOR descriptors = (PBTH_LE_GATT_DESCRIPTOR) malloc(required_size * sizeof(BTH_LE_GATT_DESCRIPTOR));

			// Get the descriptors offered by this characteristic.
			USHORT actual_size = 0;
			HRESULT hr = BluetoothGATTGetDescriptors(m_service_handle, &m_characteristics[i], required_size, descriptors,
				&actual_size, BLUETOOTH_GATT_FLAG_NONE);
			if (SUCCEEDED(hr))
			{
				for (int i = 0; i < actual_size; i++)
				{
					wprintf(L"		Characteristic Descriptor:[%s]\n", descriptors[i].DescriptorType);
					wprintf(L"		Characteristic Descriptor UUID:[%s]\n", descriptors[i].DescriptorUuid);
				}
			}
		}
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	HDEVINFO hDevInfo;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD i;
	
	GUID_DEVCLASS_BLUETOOTH;
	GUID_BTHPORT_DEVICE_INTERFACE;
	GUID_DEVINTERFACE_HID;

	hDevInfo = SetupDiGetClassDevs(&GUID_BTHPORT_DEVICE_INTERFACE, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// enumerate deviceinterfaces

	SP_DEVICE_INTERFACE_DATA device_interface_data = { 0 };
	device_interface_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	int device_index = 0;
	while (SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &GUID_BTHPORT_DEVICE_INTERFACE, device_index, &device_interface_data))
	{
		DWORD required_size = 0;

		// Query the required size for the structure.
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &device_interface_data, nullptr, 0, &required_size, nullptr);

		// HRESULT will never be S_OK here, so just check the size.
		if (required_size > 0)
		{
			// Allocate the interface detail structure.
			SP_DEVICE_INTERFACE_DETAIL_DATA* device_interface_detail_data = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(required_size);
			device_interface_detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			// Get the detailed device data which includes the device path.
			if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &device_interface_data, device_interface_detail_data, required_size, nullptr, nullptr))
			{
				wprintf(L"Device Path:[%s]\n", device_interface_detail_data->DevicePath);
				print_gatt_characteristics(device_interface_detail_data->DevicePath);
			}

			free(device_interface_detail_data);
		}

		device_index++;
	}	

	// Enumerate through all devices in Set.

	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
	{
		DWORD DataT;
		LPTSTR buffer = NULL;
		DWORD buffersize = 0;

		while (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, &DataT, (PBYTE)buffer, buffersize, &buffersize))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buffer) LocalFree(buffer);
				buffer = (LPTSTR)LocalAlloc(LPTR, buffersize * 2);
			}
			else
			{
				break;
			}
		}

		wprintf(L"Device Description:[%s]\n", buffer);

		if (buffer) LocalFree(buffer);
	}


	if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		return 1;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return 0;
}

