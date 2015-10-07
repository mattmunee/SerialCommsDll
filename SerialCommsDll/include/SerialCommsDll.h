// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SERIALCOMMSDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SERIALCOMMSDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SERIALCOMMSDLL_EXPORTS
#define SERIALCOMMSDLL_API __declspec(dllexport)
#else
#define SERIALCOMMSDLL_API __declspec(dllimport)
#endif

#include <Windows.h>
#include <string>

using namespace std;

enum SERIALCOMMSDLL_API BaudRate{
	BR110		=	CBR_110,
	BR300		=	CBR_300,
	BR1200		=	CBR_1200,
	BR2400		=	CBR_2400,
	BR4800		=	CBR_4800,
	BR9600		=	CBR_9600,
	BR14400		=	CBR_14400,
	BR19200		=	CBR_19200,
	BR38400		=	CBR_38400,
	BR56000		=	CBR_56000,
	BR57600		=	CBR_57600,
	BR115200	=	CBR_115200,
	BR128000	=	CBR_128000,
	BR256000	=	CBR_256000
};

// This class is exported from the SerialCommsDll.dll
class SerialCommPort {
public:
	/*
	Opens comm port with standard settings:
	Data bits: 8
	Stop bits: 1
	Parity: none
	Flow control: none
	*/
	SERIALCOMMSDLL_API SerialCommPort(unsigned int portNum, BaudRate baud);
	SERIALCOMMSDLL_API ~SerialCommPort(void);
	bool SERIALCOMMSDLL_API isValid(void);
	unsigned int SERIALCOMMSDLL_API readBlock(char* pData, unsigned int Size);
	unsigned int SERIALCOMMSDLL_API writeBlock(const char* pData, unsigned int Size);
	bool SERIALCOMMSDLL_API purgeSendBuffer(void);
	bool SERIALCOMMSDLL_API purgeRecvBuffer(void);
	bool SERIALCOMMSDLL_API purgeBuffers(void);
	bool SERIALCOMMSDLL_API sendString(string command);
	bool SERIALCOMMSDLL_API getResponse(string *response);
private:
	HANDLE m_handle;
	char m_termChar;
};

extern SERIALCOMMSDLL_API int nSerialCommsDll;

SERIALCOMMSDLL_API int fnSerialCommsDll(void);
