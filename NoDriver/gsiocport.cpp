/////////////////////////////////////////////////////////////////////////////
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gsiocport.h"

CGsiocPort::CGsiocPort()
{
	m_CommHandle = INVALID_HANDLE_VALUE;
	m_fFirstTime = TRUE;
}

// --------------------------------------------------------------------------
CGsiocPort::~CGsiocPort()
{
	// Check if the Win32 COM port has been opened, if so free the port
	// before this object ceases to exist.
	if (IsValid())
	{
		CloseHandle(m_CommHandle);
		m_CommHandle = INVALID_HANDLE_VALUE;
	}
}

// --------------------------------------------------------------------------
CString& CGsiocPort::GetCommID()
{
	return m_CommID;
}

// --------------------------------------------------------------------------
void CGsiocPort::SetCommID(CString& zCommID)
{
	// If the Win32 COM port has already been opened, close it.
	if (IsValid())
	{
		CloseHandle(m_CommHandle);
		m_CommHandle = INVALID_HANDLE_VALUE;
	}

	// Store the new COM value and open the Win32 COM port
	m_CommID = zCommID;
	m_CommHandle = CreateFile(	m_CommID,
								GENERIC_READ | GENERIC_WRITE,
								0,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

	// On success, set the default GSIOC communcation parameters
	if (m_CommHandle != INVALID_HANDLE_VALUE)
	{
		// --- Set the port parameters to 19200,8,e,2

		DCB	dcbInfoBlock;

		::GetCommState(m_CommHandle, &dcbInfoBlock);

		dcbInfoBlock.BaudRate = 19200;
		dcbInfoBlock.fBinary = 1;
		dcbInfoBlock.fParity = 1;
		dcbInfoBlock.fErrorChar = 0;
		dcbInfoBlock.fNull = 1;
		dcbInfoBlock.fRtsControl = 0;
		dcbInfoBlock.fAbortOnError = 0;
		dcbInfoBlock.ByteSize = 8;
		dcbInfoBlock.Parity = 2;
		dcbInfoBlock.StopBits = 2;
		dcbInfoBlock.ErrorChar = 0;

		SetCommState(m_CommHandle, &dcbInfoBlock);

		// --- Adjust the port timeout values

		COMMTIMEOUTS gCommTimeOuts;

		gCommTimeOuts.ReadIntervalTimeout = MAXDWORD;
		gCommTimeOuts.ReadTotalTimeoutMultiplier = MAXDWORD;
		gCommTimeOuts.ReadTotalTimeoutConstant = 100;
		gCommTimeOuts.WriteTotalTimeoutConstant = 0;
		gCommTimeOuts.WriteTotalTimeoutMultiplier = 1000;

		SetCommTimeouts(m_CommHandle,&gCommTimeOuts);
	}
}

// --------------------------------------------------------------------------
// StrSend(char) -- Send a single byte out the port
//
BOOL CGsiocPort::StrSend(unsigned char cmdbyte)
{
	DWORD dwBytesWritten;
	BOOL fTest1,fTest2;

	fTest1 = WriteFile(m_CommHandle, &cmdbyte, 1, &dwBytesWritten, NULL);
	fTest2 = (dwBytesWritten == 1);
	return (fTest1 && fTest2);
}

// --------------------------------------------------------------------------
// StrSend(CString& ) -- Send a string of bytes
//
BOOL CGsiocPort::StrSend(CString& cmdstr)
{
	DWORD dwBytesWritten;
	BOOL fTest1,fTest2;

	fTest1 = WriteFile(m_CommHandle, cmdstr, strlen(cmdstr), &dwBytesWritten, NULL);
	fTest2 = (dwBytesWritten == strlen(cmdstr));
	return (fTest1 && fTest2);
}

// --------------------------------------------------------------------------
// GetChar() -- Get/Wait for a character
//
BOOL CGsiocPort::GetChar(unsigned char *bValue)
{
	DWORD dwBytesRead;
	BOOL fTest1,fTest2;
	
	fTest1 = ReadFile(m_CommHandle, bValue, 1, &dwBytesRead, NULL);
	fTest2 = (dwBytesRead > 0);
	return (fTest1 && fTest2);
}


// --------------------------------------------------------------------------
// Disconnect() -- Send the GSIOC Device Disconnect sequence
//
void CGsiocPort::DisconnectDevices() { StrSend(0xff); }


// --------------------------------------------------------------------------
// Send the GSIOC "select device" and check for the appropriate reply
BOOL CGsiocPort::SelectUnit(int unit)
{
	unsigned char ucReply,ucDevSelect;

	BOOL fReturnVal = FALSE;

	ucDevSelect = 0x80 | (unsigned char)unit;		// Create GSIOC 'select' command by
													// setting bit-8 of unit ID byte value
	if (StrSend(ucDevSelect))
	{
		if (GetChar(&ucReply))						// If a reply is received, a valid
			fReturnVal = (ucDevSelect == ucReply);	// select is indicated by an echo
	}												// of the command we sent.

	return fReturnVal;
}

// --------------------------------------------------------------------------
// PrepPort() -- Prepare the port for a command operation
//
void CGsiocPort::PrepPort()
{
	// Flush (discard) any old, unsent, charachers from the transmit buffer
	PurgeComm(m_CommHandle, PURGE_TXCLEAR | PURGE_RXCLEAR);

	// On first use of the port, send a "disconnect all devices", to ensure no
	// devices think they are conneceted through this port.
	if (m_fFirstTime)
	{
		DisconnectDevices();
		m_fFirstTime = FALSE;
	}
}

// --------------------------------------------------------------------------
// StrSendAndAck() -- Send a string character by character, reading (and
// discarding) the ACKs for each byte sent.  The last ACK received is returned
// to the caller.
//
void CGsiocPort::StrSendAndAck(CString& cmd, unsigned char *ucReply)
{
	int j, iCmdLen;

	iCmdLen = cmd.GetLength();

	for (j=0; j<iCmdLen; j++)
	{
		StrSend(cmd.GetAt(j));
		GetChar(ucReply);
	}
}

// --------------------------------------------------------------------------
// A simplier form of StrSendAndAck for callers that do not need
// the returned Ack value
//
void CGsiocPort::StrSendAndAck(CString& cmd)
{
	unsigned char ucDummy;
	StrSendAndAck(cmd,&ucDummy);
}

// --------------------------------------------------------------------------
// SendBuffered() -- Send a GSIOC "buffered" command (a command that sends data
// to the target device).
//
//	Return value:
//		nTicks - An unsigned long value representing the number of system
//		clock ticks (the time) that has elapsed while processing this command
//
//	Inputs:
//		unit - An INT value representing the target GSIOC device for this command
//		cmd  - An CString reference to a CString containing the command to be sent.
//
//	Outputs:
//		resp - A CString reference to a CString to hold any response from the device.
//		(No response is normally expected from a buffered command, but this string
//		is used in this implementation to hold any error text, or other diagnostic
//		information).
//
unsigned long CGsiocPort::SendBuffered(int unit, CString& cmd, CString& resp)
{
	unsigned long nTicks1;
	unsigned char ucReply;
	CString zSTXsequence("\x0a");
	CString zEOTsequence("\x0d");

	resp.Empty();

	if (IsValid())
	{
		// Clear previous error conditions, if any
		PrepPort();

		// Record the time just before issuing the desired command so we can
		// track performance
		nTicks1 = GetTickCount();

		// Must select device before sending the desired command.
		if (!SelectUnit(unit))
		{
			resp = CString("Device Select, failed.");
			return (GetTickCount() - nTicks1);
		}

		// Send Start-of-Transmission (STX) sequence to begin buffered command
		StrSendAndAck(zSTXsequence,&ucReply);
		if (IsBusy(ucReply))
			resp = CString("Sorry, device is BUSY.");
		else
		{
			// Send each character of command check for returned ack
			StrSendAndAck(cmd);
			// Followup command with End-of-Transmission (EOT) sequence.
			StrSendAndAck(zEOTsequence);
		}

		// Disconnect device to free the port and device for other use.
		DisconnectDevices();

		// Compute the elapsed time in case we want to track performance.
		return (GetTickCount() - nTicks1);
	}
	else
	{
		return 0;
	}
}

// --------------------------------------------------------------------------
// SendImmediate() -- Send a GSIOC "immediate" command (a command that retrieves
// data from the target device).
//
//	Return value:
//		nTicks - An unsigned long value representing the number of system
//		clock ticks (the time) that has elapsed while processing this command
//
//	Inputs:
//		unit - An INT value representing the target GSIOC device for this command
//		cmd  - An CString reference to a CString containing the command to be sent.
//
//	Outputs:
//		resp - A CString reference to a CString to hold the response received 
//      from the device.  In case of error, this string will hold the error message
//		either internal, or the error string received from the device.
//
unsigned long CGsiocPort::SendImmediate(int unit, CString& cmd, CString& resp)
{
	unsigned long nTicks1;
	unsigned char vAckChar = '\x06';
	unsigned char ucReply;
	resp.Empty();

	if (IsValid())
	{
		// Clear previous error conditions, if any
		PrepPort();

		// Record the time just before issuing the desired command so we can
		// track performance
		nTicks1 = GetTickCount();

		// Must select device before sending the desired command.
		if (!SelectUnit(unit))
		{
			resp = CString("Device Select, failed.");
			return (GetTickCount() - nTicks1);
		}

		// Send the command and await the reply.
		StrSend(cmd.GetAt(0));	// Send only first character if caller accidentally
								// provided a longer string.

		// Read characters and build response string, until EOT is received.
		GetChar(&ucReply);

		while ( !IsEndOfTransmission(ucReply) )
		{
			resp += CString((char)ucReply,(int)1);
			StrSend(vAckChar);	// Ack recipt
			GetChar(&ucReply);	// Get next character of reply
		}

		// Remove EOT marker and append last character to response string
		FixupLastChar(&ucReply);
		resp += CString((char)ucReply,1);

		// Disconnect device to free the port and device for other use.
		DisconnectDevices();

		// Compute the elapsed time in case we want to track performance.
		return (GetTickCount() - nTicks1);
	}
	else
	{
		return 0;
	}
}

// --------------------------------------------------------------------------
// Send() -- A combined send command to support previously written applications
//
unsigned long CGsiocPort::Send(int unit, int cmd_type, CString& cmd, CString& resp)
{
	unsigned long ulRetValue;

	if (cmd_type == CGsiocPort::CMD_IMMEDIATE)
		ulRetValue = SendImmediate(unit, cmd, resp);
	else
		ulRetValue = SendBuffered(unit, cmd, resp);

	return ulRetValue;
}

// --------------------------------------------------------------------------
// IsValid() -- Verify that this object has been properly initialized and is
// usable.  (It's primary purpose is to assists the object's destructor in
// determining if the Win32 handle to the COM port needs to be released.)
//
BOOL CGsiocPort::IsValid()
{
	return (m_CommHandle != INVALID_HANDLE_VALUE);
}

