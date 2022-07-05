#pragma once
#include <mutex>
#include <string>

class ZewdaMeter
{
public:
	ZewdaMeter();
	~ZewdaMeter();

	bool IsEnable() { return m_bOpFlag; }
	bool Init(LPSTR portName, DWORD dwBaudRate = 9600,
		BYTE bByteSize = 8,
		BYTE bParity = NOPARITY,
		BYTE bStopBits = ONESTOPBIT);

	void Reset();
	double ReadPower(int chIndex);
	void ReadAll(double& dbValue1, double& dbValue2);
	void SetWaveLength(int chIndex, double dCWL);
	double GetWaveLength(int chIndex);
	bool SetUnit(int chIndex, int nType);//type 0 dBm 1 mW


	bool CWrite(const char* command, int len);
	bool CRead(char* out, int len, int& curlen);
private:
	bool m_bOpFlag = false;
	std::mutex m_readLocker;
	HANDLE hCom;
	int m_waittime = 50;
};

