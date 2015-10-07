// SerialCommsDll.cpp : Defines the exported functions for the DLL application.
//

#include "SerialCommsDll.h"
#include <stdio.h>

// This is the constructor of a class that has been exported.
// see SerialCommsDll.h for the class definition
SerialCommPort::SerialCommPort(unsigned int portNum, BaudRate baud)
{
	m_termChar='\r';
	char sPort[20];
	DCB dcb;
	COMMTIMEOUTS CommTimeouts;

	// create a string like "\\.\COM1, leave space for \\.\COM256"
	//   Note that we used zero based channel numbers
	#ifdef OLD_COMPILER
	sprintf(sPort, "\\\\.\\COM%d", chan+1);
	#else
	sprintf_s(sPort, 20, "\\\\.\\COM%d", portNum);
	#endif

	// Open port
	m_handle = CreateFile(sPort,
		GENERIC_READ | GENERIC_WRITE,
		0,             // comm devices must be opened w/exclusive-access
		0,          // no security attributes
		OPEN_EXISTING, // comm devices must use OPEN_EXISTING
		0,             // not overlapped I/O
		0);         // hTemplate must be NULL for comm devices

	if(m_handle == INVALID_HANDLE_VALUE)
	{
		printf("Failed to open COM port:  %u\n", GetLastError());
		m_handle = INVALID_HANDLE_VALUE;
	}// if the port failed to open

	// Set buffer size
    if (!SetupComm(m_handle, 1024, 1024))
	{
		CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
	}// failure!, close and get out

	// Configure port, start by filling dcb structure
	if (!GetCommState(m_handle, &dcb))
	{
		CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
	}// failure!, close and get out

	// update serial comm parameters
	dcb.BaudRate		=	CBR_19200;				// Set baud rate
	dcb.fBinary			=	TRUE;					// Binary mode (must be true in Windows)
	dcb.fParity			=	TRUE;
	dcb.Parity			=	NOPARITY;				// No Parity
	dcb.ByteSize		=	(BYTE)8;				// Set number of bits (8)
	dcb.StopBits		=	ONESTOPBIT;				// Set number of stop bits (1)	

	// Now use the new parameters
	if (!SetCommState(m_handle, &dcb))
	{
		CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
	}// failure!, close and get out

	// Fill comm timeout structure
	if (!GetCommTimeouts(m_handle, &CommTimeouts))
	{
		CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
	}// failure!, close and get out

	//	Set timeouts for immediate timeout, i.e. when reading from the port,
	//  only read what is in the buffer, then return without waiting for
	//  more data
	//	FROM MSDN:  A value of MAXDWORD, combined with zero values for both 
	//	the ReadTotalTimeoutConstant and ReadTotalTimeoutMultiplier members, 
	//	specifies that the read operation is to return immediately with the 
	//	bytes that have already been received, even if no bytes have been received.
	DWORD dwTimeOutInMilliSec=35;	//Timeout
    CommTimeouts.ReadIntervalTimeout		=	MAXDWORD;
    CommTimeouts.ReadTotalTimeoutMultiplier =	dwTimeOutInMilliSec;
    CommTimeouts.ReadTotalTimeoutConstant	=	0;

	// Write data to buffer immediately, don't wait for data to be moved
	//   out the communications channel
	CommTimeouts.WriteTotalTimeoutConstant		=	dwTimeOutInMilliSec;
	CommTimeouts.WriteTotalTimeoutMultiplier	=	0;

	// use the new comm timeout values
	if (!SetCommTimeouts(m_handle, &CommTimeouts))
	{
		CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
	}// failure!, close and get out

}

// This is the constructor of a class that has been exported.
// see SerialCommsDll.h for the class definition
SerialCommPort::~SerialCommPort(){
	if(m_handle!=INVALID_HANDLE_VALUE)
		CloseHandle(m_handle);
}

bool SerialCommPort::isValid()
{
	return m_handle!=INVALID_HANDLE_VALUE;
}

/*! Read a block of data from the serial port.  This function does not block.
	If the amount of data waiting to be read is less that the amount requested
	then all the available data is read and the actual amount read is returned.
	\param pData points to space to receive the array of bytes
	\param Size is the number of bytes to read.
	\return The actual amount of data read.*/
unsigned int SerialCommPort::readBlock(char* pData, unsigned int Size)
{
	unsigned long ulReadCount = 0;

	if(m_handle != INVALID_HANDLE_VALUE){
		if(!ReadFile(m_handle, pData, Size, &ulReadCount, NULL)){
			printf("\nReceive error %u\n", GetLastError());
		}
	}
	return (int)ulReadCount;

}// readBlock

/*! Write a block of data to a platform specific serial port.  This function
	will not block even if the transmit buffer is full, instead the byte(s)
	will be lost.
	\param Handle is the serial port handle returned from openCOMM().
	\param pData points to a buffer of data.
	\param Size is the number of bytes in the buffer to send.
	\return the number of bytes written.*/
unsigned int SerialCommPort::writeBlock(const char* pData, unsigned int Size)
{
	unsigned long  Count = 0;

	if(m_handle!= INVALID_HANDLE_VALUE)
	{
		if(!WriteFile(m_handle, pData, Size, &Count, NULL)){
			printf("\nTransmit error %u\n", GetLastError());
		}
	}// If this port has been opened

	return Count;

}// writeBlock

bool SerialCommPort::purgeBuffers()
{
	return purgeSendBuffer() && purgeRecvBuffer();
} //purgeBuffers

/*	Purge all contents from serial send buffer */
bool SerialCommPort::purgeSendBuffer()
{

	bool success=false;
	if(m_handle != INVALID_HANDLE_VALUE){
		success=PurgeComm(m_handle, PURGE_TXCLEAR);
		if(!success)
			printf("\nTxPurge error %u\n", GetLastError());
	}
	return success;
} //purgeSendBuffer

/*	Purge all contents from serial receive buffer */
bool SerialCommPort::purgeRecvBuffer()
{
	bool success=false;
	if(m_handle != INVALID_HANDLE_VALUE){
		success=PurgeComm(m_handle, PURGE_RXCLEAR);
		if(!success)
			printf("\nTxPurge error %u\n", GetLastError());
	}
	return success;
} //purgeRecvBuffer

/*	The sendSerial command takes in a string and 
	automatically appends a carriage return (\r) before 
	sending to the COM port identified by Handle.  Handle
	is set up when the openCOMM function is called.  */
bool SerialCommPort::sendString(string command)
{
	command+=m_termChar;
	printf("Begin Send: ");
	printf(command.c_str());
	
	const char *pszBuf=command.c_str();
	DWORD dwSize=strlen(pszBuf);
	unsigned long ulNumberOfBytesSent = 0;
	unsigned long ulNumberOfBytesWritten;

	while(ulNumberOfBytesSent < dwSize)
	{
		if(WriteFile(m_handle, &pszBuf[ulNumberOfBytesSent], 1, &ulNumberOfBytesWritten, NULL) != 0)
		{
			if(ulNumberOfBytesWritten > 0)
				++ulNumberOfBytesSent;
			else
			{
				printf("\nWrite error %u\n",GetLastError());
				break;
			}
		}
		else
		{
			printf("\nWrite error %u\n",GetLastError());
			break;
		}
	}

	printf("\nEnd Send\n");
	return ulNumberOfBytesSent==command.length();
} //sendString

/*	The recvSerial command searches for the  line feed character (\n)
	which indicates the end of a return from the Vector20.
	Handle is set up when the openCOMM function is called.  */
bool SerialCommPort::getResponse(string *response)
{
	printf("Begin Reply:\n");
	*response="";

	int num=0;
	int iter=0;
	
	if(m_handle != INVALID_HANDLE_VALUE){
		char result[512]={'\0'};
		char buf='\0';
		int bytesRead=0;

		//Vector20Dev has all replies ending with a line feed character
		while(buf!='\n'){
			bytesRead=readBlock(&buf,1);
			result[num]=buf;
			num+=bytesRead;
			if(iter++>100){
				printf("Reply Timeout: Buffer = %s\n",(string)result);
				return false;
			}
		}
		printf(result);
		*response=result;
	}
	printf("\nEnd Reply\n");
	return num>1;
} //getResponse