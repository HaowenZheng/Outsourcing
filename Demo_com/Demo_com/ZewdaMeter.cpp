#include "stdafx.h"
#include "ZewdaMeter.h"


ZewdaMeter::ZewdaMeter()
{
}


ZewdaMeter::~ZewdaMeter()
{
	CloseHandle(hCom);
}

bool ZewdaMeter::Init(LPSTR portName, DWORD dwBaudRate, BYTE bByteSize, BYTE bParity, BYTE bStopBits)
{
	std::string mComStr = portName;
	
	const char* find_com = "COM";
	auto find_index = strstr(mComStr.c_str(), find_com);
	if (find_index == nullptr)
		return m_bOpFlag;

	find_index += strlen(find_com);
	auto com_number = atoi(find_index);
	std::string actually_com_str = mComStr;
	if (com_number > 9)
	{
		const char* insert_str = "\\\\.\\";
		actually_com_str.insert(0, insert_str);
	}

	auto handle = CreateFileA(actually_com_str.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	auto opFlag = handle != INVALID_HANDLE_VALUE;
	if (!opFlag)
		return m_bOpFlag;

	this->hCom = handle;
	DCB PortDCB = { 0 };
	auto readyFlag = GetCommState(this->hCom, &PortDCB);
	if (!readyFlag)
		return readyFlag;

	PortDCB.BaudRate = dwBaudRate;
	PortDCB.ByteSize = bByteSize;
	PortDCB.Parity = bParity;
	PortDCB.StopBits = bStopBits;
	PortDCB.fAbortOnError = TRUE;

	readyFlag = SetCommState(this->hCom, &PortDCB);
	if (!readyFlag)
		return readyFlag;


	COMMTIMEOUTS TimeOuts = { 0 }; //设定读超时
	GetCommTimeouts(this->hCom, &TimeOuts);

	TimeOuts.ReadIntervalTimeout = 3;
	TimeOuts.ReadTotalTimeoutMultiplier = 2;
	TimeOuts.ReadTotalTimeoutConstant = 3; //设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(this->hCom, &TimeOuts); //设置超时

	this->m_bOpFlag = true;
	return m_bOpFlag;
}

void ZewdaMeter::Reset()
{
	char szCmd[64] = { 0 };
	char readbuf[64] = { 0 };
	int readlength;
	memset(szCmd, 0, sizeof(szCmd));
	strcpy_s(szCmd, "CAL:ZERO:AUTO 1");

	std::lock_guard<std::mutex> mt(m_readLocker);

	this->CWrite(szCmd, strlen(szCmd));
	Sleep(m_waittime);
	this->CRead(readbuf, sizeof(readbuf), readlength);

}

double ZewdaMeter::ReadPower(int chIndex)
{
	char szCmd[64] = { 0 };
	char readbuf[64] = { 0 };
	int readlength;
	memset(szCmd, 0, sizeof(szCmd));
	//strcpy(szCmd, "SOUR1:POW?");
	sprintf_s(szCmd, "SOUR%d:POW?", chIndex);

	std::lock_guard<std::mutex> mt(m_readLocker);
	this->CWrite(szCmd, strlen(szCmd));

	Sleep(m_waittime);

	this->CRead(readbuf, sizeof(readbuf), readlength);

	return atof(readbuf);
}

void ZewdaMeter::ReadAll(double & dbValue1, double & dbValue2)
{
	char szCmd[64] = { 0 };
	char readbuf[64] = { 0 };
	int readlength;
	memset(szCmd, 0, sizeof(szCmd));
	strcpy_s(szCmd, "SOUR:POW?");

	std::lock_guard<std::mutex> mt(m_readLocker);
	this->CWrite(szCmd, strlen(szCmd));

	Sleep(m_waittime);

	this->CRead(readbuf, sizeof(readbuf), readlength);

	if (0 != readlength)
	{
		std::string str = readbuf;

		dbValue1 = atof(str.substr(0, str.find(',')).c_str());
		dbValue2 = atof(str.substr(str.find(',') + 1).c_str());
	}

	return;
}

void ZewdaMeter::SetWaveLength(int chIndex, double dCWL)
{
	char szCmd[64] = { 0 };
	char readbuf[64] = { 0 };
	int readlength;
	memset(szCmd, 0, sizeof(szCmd));
	sprintf_s(szCmd, "SOUR%d:WAV %.f", chIndex, dCWL);

	std::lock_guard<std::mutex> mt(m_readLocker);
	this->CWrite(szCmd, strlen(szCmd));

	Sleep(m_waittime);

	this->CRead(readbuf, sizeof(readbuf), readlength);

	return;
}

double ZewdaMeter::GetWaveLength(int chIndex)
{
	char szCmd[64] = { 0 };
	char readbuf[64] = { 0 };
	int readlength;
	memset(szCmd, 0, sizeof(szCmd));
	sprintf_s(szCmd, "SOUR%d:WAV?", chIndex);

	std::lock_guard<std::mutex> mt(m_readLocker);
	this->CWrite(szCmd, strlen(szCmd));

	Sleep(m_waittime);

	this->CRead(readbuf, sizeof(readbuf), readlength);

	return atof(readbuf);
}

bool ZewdaMeter::SetUnit(int chIndex, int nType)
{
	char szCmd[64] = { 0 };
	char readbuf[64] = { 0 };
	int readlength;
	memset(szCmd, 0, sizeof(szCmd));
	sprintf_s(szCmd, "SOUR%d:POW:UNIT ", chIndex);

	if (0 == nType)
	{
		sprintf_s(szCmd, "SOUR%d:POW:UNIT dBm", chIndex);
	}
	else if (1 == nType)
	{
		sprintf_s(szCmd, "SOUR%d:POW:UNIT mW", chIndex);
	}
	else
		return false;

	std::lock_guard<std::mutex> mt(m_readLocker);
	this->CWrite(szCmd, strlen(szCmd));

	Sleep(m_waittime);

	this->CRead(readbuf, sizeof(readbuf), readlength);

	return 0 != readlength;
}

bool ZewdaMeter::CWrite(const char * command, int len)
{
	DWORD dwErrorFlags = 0;
	COMSTAT ComStat = { 0 };
	ClearCommError(hCom, &dwErrorFlags, &ComStat);

	PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR);

	DWORD curWrite = 0;
	return WriteFile(this->hCom, command, len, &curWrite, nullptr);
}

bool ZewdaMeter::CRead(char * out, int len, int & curlen)
{
	DWORD curRead = 0;
	auto ret = ReadFile(this->hCom, out, len, &curRead, nullptr);
	curlen = curRead;
	return ret;
}
