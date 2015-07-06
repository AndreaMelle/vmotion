// SimpleSerialExample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "enumser.h"
#include <vector>
#include <sstream>
#include <chrono>
#include "EndianHelpers.h"

int _tmain(int argc, _TCHAR* argv[])
{

	//Initialize COM (Required by CEnumerateSerial::UsingWMI)
	HRESULT hr = CoInitialize(NULL);

	if (FAILED(hr))
	{
		_tprintf(_T("Failed to initialize COM, Error:%x\n"), hr);
		return -1;
	}

	//Initialize COM security (Required by CEnumerateSerial::UsingWMI)
	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hr))
	{
		_tprintf(_T("Failed to initialize COM security, Error:%x\n"), hr);
		CoUninitialize();
		return -1;
	}

	/*
	* 1. Print available serial ports
	*/
	std::vector<UINT> ports;

	size_t i = 0;
	UNREFERENCED_PARAMETER(i);

	_tprintf(_T("CreateFile method reports\n"));

	if (CEnumerateSerial::UsingCreateFile(ports))
	{
		for (i = 0; i < ports.size(); i++)
		{
			_tprintf(_T("COM%u\n"), ports[i]);
		}
	}
	else
	{
		_tprintf(_T("CEnumerateSerial::UsingCreateFile failed, Error:%u\n"), GetLastError());
	}
		

	/*
	* 2. Open a serial port
	*/

	if (ports.size() <= 0)
	{
		_tprintf(_T("No COM port found\n"));
		return -1;
	}

	TCHAR *pcCommPort = TEXT("COM5");

	HANDLE hComm;
	hComm = CreateFile(pcCommPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (hComm == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("Could not open port\n"));
		return -1;
	}

	/*
	* 3. Configure the serial port
	*/

	DWORD baudRate = 9600;
	BYTE stopBits = 0;
	BYTE parity = 0;
	BYTE byteSize = 8;

	DCB config;
	SecureZeroMemory(&config, sizeof(DCB));
	config.DCBlength = sizeof(DCB);

	if (GetCommState(hComm, &config) == 0)
	{
		_tprintf(_T("Could not get port configuration\n"));
		return -1;
	}

	config.BaudRate = baudRate;
	config.StopBits = stopBits;
	config.Parity = parity;
	config.ByteSize = byteSize;

	if (SetCommState(hComm, &config) == 0)
	{
		_tprintf(_T("Could not set port configuration\n"));
		return -1;
	}

	// instance an object of COMMTIMEOUTS. check http://www.codeproject.com/Articles/3061/Creating-a-Serial-communication-on-Win
	COMMTIMEOUTS comTimeOut;
	// Specify time-out between charactor for receiving.
	comTimeOut.ReadIntervalTimeout = 3;
	// Specify value that is multiplied 
	// by the requested number of bytes to be read. 
	comTimeOut.ReadTotalTimeoutMultiplier = 3;
	// Specify value is added to the product of the 
	// ReadTotalTimeoutMultiplier member
	comTimeOut.ReadTotalTimeoutConstant = 2;
	// Specify value that is multiplied 
	// by the requested number of bytes to be sent. 
	comTimeOut.WriteTotalTimeoutMultiplier = 3;
	// Specify value is added to the product of the 
	// WriteTotalTimeoutMultiplier member
	comTimeOut.WriteTotalTimeoutConstant = 2;
	// set the time-out parameter into device control.
	SetCommTimeouts(hComm, &comTimeOut);

	/*
	* Writing to serial port
	*/

	/*
	* Reading from serial port
	*/

	DWORD max_bytes_to_read = 1;
	DWORD bytes_read = 0;
	BYTE* read_buffer = new BYTE[1];

	size_t num_float_expected = 1;
	float* _input_buf = new float[num_float_expected];
	size_t _input_pos = 0;

	assert(sizeof(float) == 4);
	assert(sizeof(char) == 1);

	bool isBigEndian = _big_endian();

	BYTE* output_buffer = reinterpret_cast<BYTE*>("hello");
	DWORD bytes_to_write = 5 * sizeof(char);
	DWORD bytes_written = 0;

	std::chrono::time_point<std::chrono::system_clock> current = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> last = current;
	std::chrono::duration<double> elapsed_seconds;
	std::chrono::duration<double> write_freq = std::chrono::duration<double>(1.0f);

	while (true)
	{
		std::chrono::time_point<std::chrono::system_clock> current = std::chrono::system_clock::now();
		elapsed_seconds = current - last;
		
		if (elapsed_seconds >= write_freq)
		{
			if (!WriteFile(hComm, output_buffer, bytes_to_write, &bytes_written, NULL))
			{
				_tprintf(_T("Could not write to serial port\n"));
				return FALSE;
			}

			if (bytes_written != bytes_to_write)
			{
				_tprintf(_T("Not all data written to serial port\n"));
			}

			last = current;
		}

		if (!ReadFile(hComm, read_buffer, max_bytes_to_read, &bytes_read, NULL))
		{
			_tprintf(_T("Could not read from port\n"));
		}

		if (bytes_read > 0)
		{
			(reinterpret_cast<char*> (_input_buf))[_input_pos++] = read_buffer[0];

			if (_input_pos == (num_float_expected * sizeof(float))) // we received a full frame
			{
				printf("Received float %f\n", _input_buf[0]);
				_input_pos = 0;
			}

			//printf("%c", read_buffer[0]);
		}

		
	}

	

	/*
	* 4. Close the serial port
	*/

	delete[] read_buffer;
	delete[] _input_buf;

	if (CloseHandle(hComm) == 0)    // Call this function to close port.
	{
		_tprintf(_T("Could not close port\n"));
		return FALSE;
	}

	//Close down COM
	CoUninitialize();

	return 0;
}

