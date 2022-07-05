#include "stdafx.h"
#include "SBT951Meter.h"
#include "Meter.h"

REGISTER_METER_SERVICE(CSBT951Meter)
CSBT951Meter::CSBT951Meter()
{
}


CSBT951Meter::~CSBT951Meter()
{
	CloseHandle(hCom);
}

std::string CSBT951Meter::GetServiceName()
{
	return std::string(GETCLASSNAME(CSBT951Meter));
}

bool CSBT951Meter::IsEnable()
{
	return this->m_bOpFlag;
}

std::string CSBT951Meter::GetServiceType()
{
	return std::string(TYPE_GENERAL);
}

void CSBT951Meter::SendCmd(char * cmd, int cmdLength, char * ret, int & retLength)
{
	std::lock_guard<std::mutex> lcx(this->read_locker);
	this->CWrite(cmd, cmdLength);
	Sleep(200);
	int cur_recv_len = 0;
	this->CRead(ret, 128, cur_recv_len);
	retLength = cur_recv_len;
}

void CSBT951Meter::Reset()
{
	std::lock_guard<std::mutex> lcx(this->read_locker);
	char readbuf[32] = { 0 };
	int readlength;
	char cmd[32] = { 0 };
	cmd[0] = 0xFE;
	cmd[1] = 0x01;
	cmd[2] = 0x56;
	cmd[3] = 0x00;
	cmd[4] = 0xCF;
	cmd[5] = 0xFC;
	cmd[6] = 0xCC;
	cmd[7] = 0xFF;

	CWrite(cmd, 8);
	Sleep(50);
	CRead(readbuf, sizeof(readbuf), readlength);
}

BOOL CSBT951Meter::Init(LPSTR lpINSTR, HWND hWnd, BOOL bHasNP1830)
{
	std::string mComStr = lpINSTR;
	DWORD baud = (DWORD)hWnd;

	const char* find_com = "COM";
	auto find_index = strstr(mComStr.c_str(), find_com);
	if (find_index == nullptr)
		return false;

	find_index += strlen(find_com);
	auto com_number_str = atoi(find_index);
	std::string actually_com_str = mComStr;
	if (com_number_str > 9)
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
		throw std::exception("压力传感器打开串口失败");

	COMMTIMEOUTS TimeOuts = { 0 }; //设定读超时
	GetCommTimeouts(hCom, &TimeOuts);

	TimeOuts.ReadIntervalTimeout = 3;
	TimeOuts.ReadTotalTimeoutMultiplier = 2;
	TimeOuts.ReadTotalTimeoutConstant = 3; //设定写超时
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(hCom, &TimeOuts); //设置超时

	this->hCom = handle;
	this->m_bOpFlag = true;

	return m_bOpFlag;
}

int CSBT951Meter::GetUnits()
{
	return 0;
}

double CSBT951Meter::ReadOnce(BYTE bChannelIndex, BOOL bHasNP1830, double * m_dValue2nd, BOOL isTwoChannels)
{
	std::lock_guard<std::mutex> lcx(this->read_locker);

	double value = 0.0;
	value = this->ReadOrignPower();

	//return -3.99;
	return value;
}

double CSBT951Meter::ReadPower(BYTE bChannelIndex, BOOL bHasNP1830, int nAvgSamples)
{
	std::lock_guard<std::mutex> lcx(this->read_locker);

	double value = 0.0;
	value = this->ReadOrignPower();
	//return -3.99;
	return value;
}

void CSBT951Meter::SetWaveLength(BYTE bChannelIndex, double dCWL, BOOL bHasNP1830)
{
}

double CSBT951Meter::GetPower_PDL(BYTE bChannelIndex, HWND hWndDisplay, int maxSamples, double dblref, double * dMaxOut, double * dMinOut, double * d3Segma, BOOL bCanStop, BOOL bHasNP1830)
{
	return 0.0;
}

void CSBT951Meter::ClearBuffer()
{
}

BOOL CSBT951Meter::CWrite(const char * comm, int len)
{
	DWORD dwErrorFlags = 0;
	COMSTAT ComStat = { 0 };
	ClearCommError(hCom, &dwErrorFlags, &ComStat);

	PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	PurgeComm(hCom, PURGE_TXABORT | PURGE_TXCLEAR);

	DWORD curWrite = 0;
	return WriteFile(hCom, comm, len, &curWrite, nullptr);
}

BOOL CSBT951Meter::CRead(char * out, int len, int & curLen)
{
	DWORD curRead = 0;
	auto ret = ReadFile(hCom, out, len, &curRead, nullptr);
	curLen = curRead;
	return ret;
}

double CSBT951Meter::ReadOrignPower()
{

	std::lock_guard<std::mutex> lcx(this->read_locker2);
	char readbuf[32] = { 0 };
	int readlength;
	char cmd[32] = { 0 };
	cmd[0] = 0xFE;
	cmd[1] = 0x01;
	cmd[2] = 0x50;
	cmd[3] = 0x00;
	cmd[4] = 0xCF;
	cmd[5] = 0xFC;
	cmd[6] = 0xCC;
	cmd[7] = 0xFF;

	CWrite(cmd, 8);
	Sleep(20);
	CRead(readbuf, sizeof(readbuf), readlength);

	union
	{
		int v;
		char a[4];
	}u;

	u.a[0] = readbuf[7];
	u.a[1] = readbuf[6];
	u.a[2] = readbuf[5];
	u.a[3] = readbuf[4];

	//auto i = u.v;
	//std::cout << i << std::endl;

	return u.v / 10.0;
}
