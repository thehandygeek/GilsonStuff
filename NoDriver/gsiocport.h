#define GSIOC_DEV_BUSY ('#')
#define GSIOC_EOT_MASK 0x80

class CGsiocPort
{
	public:
		CGsiocPort();
		~CGsiocPort();

		enum {CMD_IMMEDIATE, CMD_BUFFERED};

		virtual CString& GetCommID();
		virtual void SetCommID(CString& zCommID);
		virtual unsigned long Send(int unit, int cmd_type, CString& cmd, CString& resp);
		virtual unsigned long SendImmediate(int unit, CString& cmd, CString& resp);
		virtual unsigned long SendBuffered(int unit, CString& cmd, CString& resp);
		virtual BOOL IsValid();

	protected:
		void StrSendAndAck(CString& cmd);
		void StrSendAndAck(CString& cmd, unsigned char *ucReply);
		void PrepPort();
		BOOL SelectUnit(int unit);
		BOOL StrSend(CString& cmd);
		BOOL StrSend(unsigned char cmd);
		BOOL GetChar(unsigned char *bValue);
		void DisconnectDevices();

	private:
		BOOL IsEndOfTransmission(unsigned char ucValue) { return (ucValue & GSIOC_EOT_MASK); }
		BOOL IsBusy(unsigned char ucValue) { return (ucValue == GSIOC_DEV_BUSY); }
		void FixupLastChar(unsigned char *ucReply) { *ucReply &= ~GSIOC_EOT_MASK ; }

		CString zTmpRet;
		CString m_CommID;
		HANDLE m_CommHandle;
		BOOL m_fFirstTime;
};
