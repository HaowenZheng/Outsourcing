#pragma once
//#include "C:\Users\hzheng17\OneDrive - kochind.com\haowenz\Haowenz\AutoMation\Project\AM166\AM166\Interface\MeterInterface.h"
class CSBT951Meter :
	public IPowerMeter
{
	HANDLE hCom;

protected:
	bool m_bOpFlag = false;
	std::mutex read_locker;
	std::mutex read_locker2;

public:
	CSBT951Meter();
	~CSBT951Meter();

	// Inherited via IPowerMeter
	virtual std::string GetServiceName() override;
	virtual bool IsEnable() override;
	virtual std::string GetServiceType() override;
	virtual void SendCmd(char * cmd, int cmdLength, char * ret, int & retLength) override;
	virtual void Reset() override;
	virtual BOOL Init(LPSTR lpINSTR = (LPSTR)NULL, HWND hWnd = NULL, BOOL bHasNP1830 = TRUE) override;
	virtual int GetUnits() override;
	virtual double ReadOnce(BYTE bChannelIndex, BOOL bHasNP1830, double * m_dValue2nd = NULL, BOOL isTwoChannels = FALSE) override;
	virtual double ReadPower(BYTE bChannelIndex, BOOL bHasNP1830, int nAvgSamples = 1) override;
	virtual void SetWaveLength(BYTE bChannelIndex, double dCWL, BOOL bHasNP1830) override;
	virtual double GetPower_PDL(BYTE bChannelIndex, HWND hWndDisplay, int maxSamples, double dblref, double * dMaxOut, double * dMinOut, double * d3Segma, BOOL bCanStop = TRUE, BOOL bHasNP1830 = TRUE) override;
	virtual void ClearBuffer() override;

	BOOL CWrite(const char * comm, int len);
	BOOL CRead(char * out, int len, int & curLen);
	virtual double ReadOrignPower();
};

