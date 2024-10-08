/*
 * midi_device.c
 *
 *  Created on: 2020/03/25
 *      Author: higuchi
 */

#include "midi_device.h"

#include "midi_message.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* MIDI 受信関連 */
enum eExclRxStatus {
	ExclRxSts_WaitF0,

	ExclRxSts_WaitPidSid,
	ExclRxSts_Develop,
	ExclRxSts_TestMode,

	ExclRxSts_WaitFunction,
	ExclRxSts_WaitNumOfData,
	ExclRxSts_WaitExtendNumOfData,
	ExclRxSts_DumpRx,
	ExclRxSts_WaitF7
};
static uint8_t	s_ExclRxStats;
static uint8_t	s_ExclFunc;

#define	RXDMPBUF_SIZE	37	/// @note MIDI Dataのformat
static uint8_t	s_ExclRxBuff[RXDMPBUF_SIZE];
static uint8_t	s_ExclRxBuffPos;
static uint16_t	s_ExclRxNumOfData;

static uint8_t	TmpData[8];

/**
 * 暫定(VX II)
 */
#if TEST_VXII_SYSEX

#define	MODE_DATA_MODE_USER      0x00
#define	MODE_DATA_MODE_PRESET    0x01
#define	MODE_DATA_MODE_MANUAL    0x02
#define	MODE_DATA_MODE_Mask      0x03

#endif	// #if TEST_VXII_SYSEX

/*******************************************************************************
 * Code
 ******************************************************************************/
/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static void SendExclHeader(uint8_t Cmd)
{
	MIDI_SysExHeader	DumpHeaderData;

	DumpHeaderData.ExclBegin		= EXCL_BEGIN;
	DumpHeaderData.KorgID			= EXCL_KORGID;
	DumpHeaderData.MidiCh			= (uint8_t)(SFID_USER | AMP_MIDI_CH);

	DumpHeaderData.ProdId1st		= 0;
	DumpHeaderData.ProdId2nd		= AMP_PRODUCT_ID_2ND;
	DumpHeaderData.ProdId3rd		= AMP_PRODUCT_ID_3RD;

	DumpHeaderData.Cmmand			= Cmd;

	midi_SendExclData((uint8_t*)&DumpHeaderData, sizeof(DumpHeaderData));
}

static void SendExclResult(uint8_t Result)
{
#define	SEND_SIZE_RSLT	1
	SendExclHeader(Result);
	TmpData[0]	= EXCL_END;

	midi_SendExclData(TmpData, SEND_SIZE_RSLT);
}

static void MidiRxProc_SysExclError(void)
{
	switch (s_ExclFunc) {
	case SFC_PDUMP_REQ:
	case SFC_EQDUMP_REQ:
		SendExclResult(SFC_LOAD_ERR);
		break;
	}
	s_ExclFunc	= SFC_STANDBY;
}

static void MidiRxProc_CheckExclStatus(void)
{
	if (s_ExclRxStats != ExclRxSts_WaitF0) {
		MidiRxProc_SysExclError();
		s_ExclRxStats = ExclRxSts_WaitF0;
	}
}

void midi_AMP_RxInit()
{
	s_ExclRxStats	= ExclRxSts_WaitF0;
	s_ExclFunc		= SFC_STANDBY;
}

void midi_AMP_RxProc_SysEx(uint8_t Data1, uint8_t Data2, uint8_t Data3)
{
	if (Data1 == EXCL_BEGIN) {
		/* F0は、必ずUSB-MIDIパケット(0x04)で入ってくる */

		MidiRxProc_CheckExclStatus();	/* "s_ExclRxStats"が"ExclRxSts_WaitF0"でなかったら、必要なエラー処理をする */

		s_ExclFunc		= SFC_STANDBY;
		s_ExclRxBuffPos	= 0;

		switch (Data2) {
		case EXCL_KORGID:
			if (Data3 == (SFID_USER | AMP_MIDI_CH)) {
				/* F0 42 3g */
				s_ExclRxStats	= ExclRxSts_WaitPidSid;
			}
			else if (Data3 == SFID_DEVELOP) {
				/* F0 42 21 */
				s_ExclRxStats	= ExclRxSts_Develop;
			}
			else if (Data3 == SFID_TEST) {
				/* F0 42 22 */
				s_ExclRxStats	= ExclRxSts_Develop;
			}
			break;
		case NONREALMSG:
			if ((Data3 == AMP_MIDI_CH) || (Data3==0x7F)) {
				/* F0 7E gg */
				s_ExclFunc		= INQUIRYMSG;
				s_ExclRxStats	= ExclRxSts_WaitFunction;
			}
			break;
		}
	}
	else {
		/* SysExclデータの続きを受信 */
		midi_AMP_RxProc_ReceiveByte(Data1);
		midi_AMP_RxProc_ReceiveByte(Data2);
		midi_AMP_RxProc_ReceiveByte(Data3);
	}
}

static void SendModeData(void)
{
#define	SEND_SIZE_MDDT	3

	SendExclHeader(SFC_MODE_DATA);
#if TEST_VXII_SYSEX
	switch (globalbuf.modeid) {
	case MODEID_MANUAL:
		TmpData[0] = MODE_DATA_MODE_MANUAL;
		TmpData[1] = 0x00;
		break;
	case MODEID_PRESET:
		TmpData[0] = MODE_DATA_MODE_PRESET;
		TmpData[1] = GetPresetNo();
		break;
	default:
		TmpData[0] = MODE_DATA_MODE_USER;
		TmpData[1] = globalbuf.progno;
		break;
	}
#else
	TmpData[0] = 0;
	TmpData[1] = 0;
#endif
	TmpData[2]	= EXCL_END;

	midi_SendExclData(TmpData, SEND_SIZE_MDDT);
}

static void SendDeviceInquiryReply(void)
{
	MIDI_DevInquiryReply	DevInqRepData;

	DevInqRepData.ExclBegin			= EXCL_BEGIN;
	DevInqRepData.NRealMsg			= NONREALMSG;
	DevInqRepData.MidiCh			= AMP_MIDI_CH;
	DevInqRepData.InqMsg			= INQUIRYMSG;
	DevInqRepData.InqReply			= INQUIRYREP;
	DevInqRepData.KorgID			= EXCL_KORGID;

	DevInqRepData.ProdIdLsb			= AMP_PRODUCT_ID_3RD;
	DevInqRepData.ProdIdMsb			= AMP_PRODUCT_ID_2ND;

	DevInqRepData.MemCodeLsb		= 0;
	DevInqRepData.MemCodeMsb		= 0;

	DevInqRepData.MinorVersLsb 		= 0;
	DevInqRepData.MinorVersMsb		= 0;

	DevInqRepData.MajorVersLsb 		= 0;
	DevInqRepData.MajorVersMsb		= 0;

	DevInqRepData.ExclEnd			= EXCL_END;

	midi_SendExclData((unsigned char*)&DevInqRepData, sizeof(MIDI_DevInquiryReply));
}

/*
static void SendCurrentProgramDataDump(void)
{
	SendExclHeader(SFC_CDUMP);

	midi_Send8BitTo7BitData(&editbuf.amptype, EDITBUF_SIZE);
}*/

/*
static void SendProgramDataDump(void)
{
	SendExclHeader(SFC_PDUMP);

#if TEST_VXII_SYSEX
	TmpData[0] = (globalbuf.modeid == MODE_DATA_MODE_PRESET) ? MODE_DATA_MODE_PRESET : MODE_DATA_MODE_USER;
	TmpData[1] = globalbuf.progno;
#else
	TmpData[0] = 0;	// modeId
	TmpData[1] = 0; // Program no
#endif
	midi_SendExclData(TmpData, 2);

	midi_Send8BitTo7BitData(&editbuf.amptype, EDITBUF_SIZE);
}*/

static void SendWriteResult(uint8_t result, uint8_t userProgramNo)
{
	SendExclHeader(result);

	TmpData[0] = 0x00;
	TmpData[1] = userProgramNo;
	TmpData[2] = EXCL_END;
	midi_SendExclData(TmpData, 3);
}

static void SendProgramWriteResult(uint8_t programNo)
{
	SendWriteResult(SFC_WRITE_CMP, programNo);
}

static void ExecModeChange(uint8_t mode, uint8_t programNo)
{
#if TEST_VXII_SYSEX
	SendExclResult(SFC_LOAD_CMP);
#else
	/// @note not support
	SendExclResult(SFC_LOAD_ERR);
#endif
}

static void ExecParameterChange(uint8_t* paramBuf)
{
#if TEST_VXII_SYSEX
	SendExclResult(SFC_LOAD_CMP);
#else
	/// @note not support
	SendExclResult(SFC_LOAD_ERR);
#endif
}

#if TEST_VXII_SYSEX
static uint8_t testModeId;
static uint8_t testProgNo;
static void SendModeChange(void)
{
	uint8_t execModeChange = 1;

	testProgNo++;
	switch (testModeId) {
	case MODE_DATA_MODE_USER:
		if (testProgNo >= 8) {
			testProgNo = 0;
			testModeId = MODE_DATA_MODE_PRESET;
		}
		break;
	case MODE_DATA_MODE_PRESET:
		if (testProgNo >= 11) {
			testProgNo = 0;
			testModeId = MODE_DATA_MODE_USER;
		}
		break;
	default:
		execModeChange = 0;
		break;
	}
	if (execModeChange == 1) {
		SendExclHeader(SFC_MODE_CHG);

		TmpData[0] = testModeId;
		TmpData[1] = testProgNo;
		TmpData[2] = EXCL_END;
		midi_SendExclData(TmpData, 3);
	}
}
#endif

static void MidiRxProc_EndOfExcl(void)
{
	if (s_ExclRxStats == ExclRxSts_WaitF7) {
		switch (s_ExclFunc) {
		case INQUIRYMSG:	/* Device Inquiry Message Request */
			if ((s_ExclRxBuff[0] == INQUIRYMSG) && (s_ExclRxBuff[1] == INQUIRYREQ)) {
				SendDeviceInquiryReply();
			}
#if TEST_VXII_SYSEX
			/// @note i.MX RT1020ボード専用  Mode Changeのテストデータを送信する
			// F0 7E 00 06 03 F7
			else if ((s_ExclRxBuff[0] == INQUIRYMSG) && (s_ExclRxBuff[1] == 3)) {
				SendModeChange();
			}
#endif
			break;
		case SFC_MODE_REQ:		/* Mode Request */
			SendModeData();
			break;
		case SFC_CDUMP_REQ:		/* Current Program Data Dump Request */
			SendCurrentProgramDataDump();
			break;
		case SFC_CDUMP:			/* Current Program Data Dump (R) */
			{
				/* s_ExclRxBuff[0]からdump dataがある */

				// current bufferにsaveして, result messageを送信
				// AMP_App_CurrentProgramDataDumpAfterProc(TempData);
				SendExclResult(SFC_LOAD_CMP);
			}
			break;
		case SFC_PDUMP_REQ:		/* Program Data Dump Request */
			if ((s_ExclRxBuff[1] <= 0x01) && (s_ExclRxBuff[2] <= 0x07)) {
				SendProgramDataDump();
			}
			else {
				SendExclResult(SFC_LOAD_ERR);
			}
			break;
		case SFC_PDUMP:			/* Program Data Damp (R) */
			{
				/* s_ExclRxBuff[0]->ModeId, s_ExclRxBuff[1]->ProgramNo, s_ExclRxBuff[2]～ -> Dump Data */

				// Internal Memoryにsaveして, result messageを送信
				// AMP_App_ProgramDataDumpAfterProc(TempData);
				SendExclResult(SFC_LOAD_CMP);
			}
			break;
		case SFC_WRITE_REQ:		/* Program Write Request */
			if ((s_ExclRxBuff[1] <= 0x01) && (s_ExclRxBuff[2] <= 0x07)) {
				// s_ExclRxBuff[2] : 00000ppp (User Program No)
				SendProgramWriteResult(s_ExclRxBuff[2]);
			}
			else {
				SendExclResult(SFC_LOAD_ERR);
			}
			break;
		case SFC_MODE_CHG:		/* Mode Change */
			if ((s_ExclRxBuff[1] <= 0x01) && (s_ExclRxBuff[2] <= 0x07))
			{
				ExecModeChange(s_ExclRxBuff[1], s_ExclRxBuff[2]);
			}
			else {
				SendExclResult(SFC_LOAD_ERR);
			}
			break;
		case SFC_PARA_CHG:		/* Parameter Change */
			{
				// s_ExclRxBuff[1]->Parameter ID, s_ExclRxBuff[2]->Paramaeter SUB ID
				// s_ExclRxBuff[3]->Value (LSB bit 6~0), s_ExclRxBuff[4]->Value (MSB bit 13~7)
				ExecParameterChange(&s_ExclRxBuff[1]);
			}
			break;
		case SFC_MODE_DATA:		/* Mode Data */
			{
				if ((s_ExclRxBuff[1] <= 0x01) && (s_ExclRxBuff[2] <= 0x07))
				{
					/// @note not support i.MX RT1010 EVK board
				}
				else {
					SendExclResult(SFC_LOAD_ERR);
				}
			}
			break;
		default:
			break;
		}
	}
	else {
		MidiRxProc_SysExclError();
	}

	s_ExclRxStats	= ExclRxSts_WaitF0;
}

static void MidiRxProc_DumpSetup(uint8_t Func)
{
	switch (Func) {
	case SFC_CDUMP:
		{
			midi_Cnv7BitTo8BitInitialize((uint8_t*)s_ExclRxBuff);
			s_ExclRxNumOfData = RXDMPBUF_SIZE;
			s_ExclRxStats	= ExclRxSts_DumpRx;
			s_ExclFunc		= Func;
		}
	case SFC_PDUMP:
		{
			/* s_ExclRxBuff[0]->ModeId, s_ExclRxBuff[1]->Program No */
			midi_Cnv7BitTo8BitInitialize((uint8_t*)&s_ExclRxBuff[2]);
			s_ExclRxNumOfData = RXDMPBUF_SIZE;
			s_ExclRxStats	= ExclRxSts_DumpRx;
			s_ExclFunc		= Func;
		}
		break;
	default:	/* format error */
		SendExclResult(SFC_FORM_ERR);
		midi_AMP_RxInit();
		break;
	}
}

static void MidiRxProc_AnalyzeSysExclFunc(uint8_t Func)
{
	switch (Func) {
	case SFC_MODE_REQ:
	case SFC_CDUMP_REQ:
		s_ExclFunc		= Func;
		s_ExclRxStats	= ExclRxSts_WaitF7;
		break;
	case SFC_CDUMP:
		s_ExclRxBuffPos = 0;
		MidiRxProc_DumpSetup(Func);
		break;
	case SFC_PDUMP:
		s_ExclRxBuffPos = 0;
		/* through */
	case SFC_WRITE_REQ:
	case SFC_PDUMP_REQ:
	case SFC_PARA_CHG:
	case SFC_MODE_DATA:
	case SFC_MODE_CHG:
		s_ExclFunc		= Func;
		s_ExclRxStats	= ExclRxSts_WaitNumOfData;
		break;
	default:	/* format error */
		SendExclResult(SFC_FORM_ERR);
		midi_AMP_RxInit();
		break;
	}
}

static void MidiRxProc_AnalyzeID(void)
{
	if ((s_ExclRxBuff[0] == AMP_PRODUCT_ID_1ST) &&
		(s_ExclRxBuff[1] == AMP_PRODUCT_ID_2ND) &&
		(s_ExclRxBuff[2] == AMP_PRODUCT_ID_3RD)) {
		s_ExclFunc		= SFC_REC;

		s_ExclRxBuffPos = 0;

		s_ExclRxStats	= ExclRxSts_WaitFunction;
	}
	else {
		/* ProductID及びSubIDが違う */
		s_ExclRxStats = ExclRxSts_WaitF0;
	}
}

void midi_AMP_RxProc_ReceiveByte(uint8_t Data)
{
	if (Data >= 0xF8/*MIDI CLock*/) {
		/* MIDIリアルタイムメッセージ受信処理 */
	}
	else {
		if (Data == EXCL_END) {
			MidiRxProc_EndOfExcl();
		}
		else if (Data & 0x80) {
			/* SysExcl受信中にMSBが立っているデータを受けた場合、必要なエラー処理をする */
			MidiRxProc_CheckExclStatus();
		}
		else {
			if (s_ExclRxBuffPos < RXDMPBUF_SIZE) {
				s_ExclRxBuff[s_ExclRxBuffPos++]	= Data;
			}

			switch (s_ExclRxStats) {
			case ExclRxSts_WaitF7:
				MidiRxProc_SysExclError();
				s_ExclRxStats = ExclRxSts_WaitF0;
				break;
			case ExclRxSts_WaitFunction:
				{
					if (s_ExclFunc == SFC_REC) {
						MidiRxProc_AnalyzeSysExclFunc(s_ExclRxBuff[0]);
					}
					else if (s_ExclFunc == INQUIRYMSG) {
						if (s_ExclRxBuffPos == 2) {
							s_ExclRxStats	= ExclRxSts_WaitF7;
						}
					}
					else {
						s_ExclRxStats	= ExclRxSts_WaitF7;
					}
				}
				break;
			case ExclRxSts_WaitPidSid:
				if (s_ExclRxBuffPos == NUM_OF_PRODUCT_ID) {
					MidiRxProc_AnalyzeID();
				}
				break;
			case ExclRxSts_WaitNumOfData:
				if (s_ExclRxBuffPos == 2) {
					if (s_ExclFunc == SFC_PDUMP) {
						MidiRxProc_DumpSetup(SFC_PDUMP);
					}
				}
				else if (s_ExclRxBuffPos == 3) {
					switch (s_ExclFunc) {
					case SFC_PDUMP_REQ:
					case SFC_WRITE_REQ:
					case SFC_MODE_DATA:
					case SFC_MODE_CHG:
						s_ExclRxStats	= ExclRxSts_WaitF7;
						break;
					default:
						break;
					}
				}
				else if (s_ExclRxBuffPos == 5) {
					if (s_ExclFunc == SFC_PARA_CHG) {
						s_ExclRxStats	= ExclRxSts_WaitF7;
					}
				}
				break;
			case ExclRxSts_DumpRx:
				midi_Cnv7BitTo8BitPutData(Data);
				s_ExclRxNumOfData--;
				if (s_ExclRxNumOfData == 0) {
					s_ExclRxStats	= ExclRxSts_WaitF7;
				}
				break;
			default:
				break;
			}
		}
	}
}

