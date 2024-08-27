/*
	triggerParameterIO.c
*/
#include <stddef.h>
#include <stdint.h>
#include "triggerParameterIO.h"

extern void triggerPadSetFunc(int num, int id, int data);
extern void triggerCCPadSetFunc(int num, int id, int data);
extern void triggerExtPadSetFunc(int num, int id, int data);
extern void triggerSwitchSetFunc(int num, int id, int data);
extern void triggerVolumeSetFunc(int num, int id, int data);
extern void triggerVersionSetFunc(int num, int data);
extern void triggerRawDateSetFunc(int num, int data);
extern int triggerVelocityCurveSetFunc(int num, int ofs, int data);

extern void triggerVersionSendFunc(void);
extern void triggerAllDumpSendFunc(void);
extern void triggerRawDataSendFunc(int num);
extern void triggerVelocityCurveSendFunc(int num);

static uint8_t	phase = 0;
static uint8_t	head = 0;
static uint8_t num = 0;
static uint8_t ofs = 0;
static uint8_t	lsb = 0;
static uint8_t (*func)(uint8_t) = NULL;

static uint8_t pad_decode(uint8_t data)
{
	uint8_t ret = phase;

	switch (phase)
	{
	case 2:	// num & ofs
		num = data >> 4;
		ofs = data & 0xf;
		ret++;
		break;
	case 3:
		switch (ofs)
		{
		case 0:	// dis/ena
			triggerPadSetFunc(num, 0, data);
			break;
		case 1:	// onCnt
			triggerPadSetFunc(num, 1, data);
			break;
		case 2:	// velWnd
			triggerPadSetFunc(num, 2, data);
			break;
		case 3:	// mskTim
			triggerPadSetFunc(num, 3, data);
			break;
		case 4:	// padOff(LSB)
		case 6:	// velSmin(LSB)
		case 8:	// velSmax(LSB)
			lsb = data & 0x7f;
			break;
		case 5:	// padOff(MSB)
		case 7:	// velSmin(MSB)
		case 9:	// velSmax(MSB)
			{
				int dt = data;
				int id = (ofs - 5) / 2 + 4;	// 5,7,9 -> 0,2,4 -> 0,1,2 -> 4,5,6

				dt = (dt << 7) | lsb;
				triggerPadSetFunc(num, id, dt);
			}
			if (ofs != 9)
			{
				break;
			}
		default:	// error or end
			ret = 0;
			break;
		}
		ofs++;
		break;
	default:	// error
		ret = 0;
		break;
	}

	return ret;
}

static uint8_t ccpad_decode(uint8_t data)
{
	uint8_t ret = phase;

	switch (phase)
	{
	case 2:	// num & ofs
		num = data >> 4;
		ofs = data & 0xf;
		ret++;
		break;
	case 3:
		switch (ofs)
		{
		case 0:	// dis/ena
			triggerCCPadSetFunc(num, 0, data);
			break;
		case 1:	// onCnt
			triggerCCPadSetFunc(num, 1, data);
			break;
		case 2:	// velWnd
			triggerCCPadSetFunc(num, 2, data);
			break;
		case 3:	// mskTim
			triggerCCPadSetFunc(num, 3, data);
			break;
		case 4:	// ribbonOff(LSB)
		case 6:	// cnvSmin(LSB)
		case 8:	// cnvSmax(LSB)
		case 10:	// velSmin(LSB)
		case 12:	// velSmax(LSB)
			lsb = data & 0x7f;
			break;
		case 5:	// ribbonOff(MSB)
		case 7:	// cnvSmin(MSB)
		case 9:	// cnvSmax(MSB)
		case 11:	// velSmin(MSB)
		case 13:	// velSmax(MSB)
			{
				int dt = data;
				int id = (ofs - 5) / 2 + 4;	// 5,7,9,11,13 -> 0,2,4,6,8 -> 0,1,2,3,4 -> 4,5,6,7,8

				dt = (dt << 7) | lsb;
				triggerCCPadSetFunc(num, id, dt);
			}
			if (ofs != 13)
			{
				break;
			}
		default:	// error or end
			ret = 0;
			break;
		}
		ofs++;
		break;
	default:	// error
		ret = 0;
		break;
	}

	return ret;
}

static uint8_t extpad_decode(uint8_t data)
{
	uint8_t ret = phase;

	switch (phase)
	{
	case 2:	// num & ofs
		num = data >> 4;
		ofs = data & 0xf;
		ret++;
		break;
	case 3:
		switch (ofs)
		{
		case 0:	// dis/ena
			triggerExtPadSetFunc(num, 0, data);
			break;
		case 1:	// onCnt
			triggerExtPadSetFunc(num, 1, data);
			break;
		case 2:	// velWnd
			triggerExtPadSetFunc(num, 2, data);
			break;
		case 3:	// mskTim
			triggerExtPadSetFunc(num, 3, data);
			break;
		case 4:	// crsCan
			triggerExtPadSetFunc(num, 4, data);
			break;
		case 5:	// onLvl(LSB)
		case 7:	// velSmin(LSB)
		case 9:	// velSmax(LSB)
			lsb = data & 0x7f;
			break;
		case 6:	// onLvl(MSB)
		case 8:	// velSmin(MSB)
		case 10:	// velSmax(MSB)
			{
				int dt = data;
				int id = (ofs - 6) / 2 + 5;	// 6,8,10 -> 0,2,4 -> 0,1,2 -> 5,6,7

				dt = (dt << 7) | lsb;
				triggerExtPadSetFunc(num, id, dt);
			}
			if (ofs != 10)
			{
				break;
			}
		default:	// error or end
			ret = 0;
			break;
		}
		ofs++;
		break;
	default:	// error
		ret = 0;
		break;
	}

	return ret;
}

static uint8_t switch_decode(uint8_t data)
{
	uint8_t ret = phase;

	switch (phase)
	{
	case 2:	// num & ofs
		num = data >> 4;
		ofs = data & 0xf;
		ret++;
		break;
	case 3:
		switch (ofs)
		{
		case 0:	// dis/ena
			triggerSwitchSetFunc(num, 0, data);
			break;
		case 1:	// onCnt
			triggerSwitchSetFunc(num, 1, data);
			break;
		case 2:	// offCnt
			triggerSwitchSetFunc(num, 2, data);
			break;
		case 3:	// swOff(LSB)
			lsb = data & 0x7f;
			break;
		case 4:	// swOff(MSB)
			{
				int dt = data;
				int id = (ofs - 4) / 2 + 3;	// 4 -> 0 -> 0 -> 3

				dt = (dt << 7) | lsb;
				triggerSwitchSetFunc(num, id, dt);
			}
			if (ofs != 4)
			{
				break;
			}
		default:	// error or end
			ret = 0;
			break;
		}
		ofs++;
		break;
	default:	// error
		ret = 0;
		break;
	}

	return ret;
}

static uint8_t volume_decode(uint8_t data)
{
	uint8_t ret = phase;

	switch (phase)
	{
	case 2:	// num & ofs
		num = data >> 4;
		ofs = data & 0xf;
		ret++;
		break;
	case 3:
		switch (ofs)
		{
		case 0:	// dis/ena
			triggerVolumeSetFunc(num, 0, data);
			break;
		case 1:	// pow
			triggerVolumeSetFunc(num, 1, data);
			break;
		case 2:	// flt1
			triggerVolumeSetFunc(num, 2, data);
			break;
		case 3:	// flt2
			triggerVolumeSetFunc(num, 3, data);
			break;
		case 4:	// hysWnd
			triggerVolumeSetFunc(num, 4, data);
			break;
		case 5:	// posA
			triggerVolumeSetFunc(num, 5, data);
			break;
		case 6:	// posB
			triggerVolumeSetFunc(num, 6, data);
			break;
		case 7:	// mskTim
			triggerVolumeSetFunc(num, 7, data);
			break;
		case 8:	// volSmin(LSB)
		case 10:	// volSmax(LSB)
		case 12:	// velSmin(LSB)
		case 14:	// velSmax(LSB)
			lsb = data & 0x7f;
			break;
		case 9:	// volSmin(MSB)
		case 11:	// volSmax(MSB)
		case 13:	// velSmin(MSB)
		case 15:	// velSmax(MSB)
			{
				int dt = data;
				int id = (ofs - 9) / 2 + 8;	// 9,11,13,15 -> 0,2,4,6 -> 0,1,2,3 -> 8,9,10,11

				dt = (dt << 7) | lsb;
				triggerVolumeSetFunc(num, id, dt);
			}
			if (ofs != 15)
			{
				break;
			}
		default:	// error or end
			ret = 0;
			break;
		}
		ofs++;
		break;
	default:	// error
		ret = 0;
		break;
	}

	return ret;
}

static uint8_t velocityCurve_decode(uint8_t data)
{
	uint8_t ret = phase;

	if (!triggerVelocityCurveSetFunc(num - CMDVELCRV, ofs++, data))
	{	// end
		ret = 0;
	}

	return ret;
}

static uint8_t cmdans_decode(uint8_t data)
{
	uint8_t ret = phase;

	switch (phase)
	{
	case 2:
		num = data;	// cmd
		if ((num >= CMDVELCRV) && (num < CMDRAWDMP))
		{
			func = velocityCurve_decode;
		}
		ofs = 0;
		ret++;
		break;
	case 3:
		switch (ofs)
		{
		case 0:	// lsb
			lsb = data;
			break;
		case 1:	// msb
			switch (num)
			{
			case CMDVERREQ:
				triggerVersionSetFunc(data, lsb);
				break;
			default:
				if (num >= CMDRAWDMP)
				{
					int dt = data;

					dt = (dt << 7) | (lsb & 0x7f);
					triggerRawDateSetFunc(num - CMDRAWDMP, dt);
				}
				break;
			}
		default:	// error or end
			ret = 0;
			break;
		}
		ofs++;
		break;
	default:	// error
		ret = 0;
		break;
	}

	return ret;
}

void triggerParameterIOEntry(uint8_t data)
{
	if (data >= HEADCMD)
	{	// header
		head = data;
		phase = 1;	// cmd
		switch (head)
		{
		case HEADCMD:
			break;
		case HEADPAD:
			func = pad_decode;
			phase++;	// data
			break;
		case HEADCCPAD:
			func = ccpad_decode;
			phase++;	// data
			break;
		case HEADEXTPAD:
			func = extpad_decode;
			phase++;	// data
			break;
		case HEADSWITCH:
			func = switch_decode;
			phase++;	// data
			break;
		case HEADVOLUME:
			func = volume_decode;
			phase++;	// data
			break;
		case HEADCMDANS:
			func = cmdans_decode;
			phase++;	// data
			break;
		case HEADRESET:	// reset
		default:
			func = NULL;
			phase = 0;
			break;
		}
	}
	else
	{
		switch (phase)
		{
		case 0:	// idle
			break;
		case 1:	// cmd or ?
			switch (head)
			{
			case HEADCMD:
				switch (data)
				{
				case CMDVERREQ:
					triggerVersionSendFunc();
					break;
				case CMDALLDMP:
					triggerAllDumpSendFunc();
					break;
				default:
					if (data >= CMDRAWDMP)
					{
						triggerRawDataSendFunc(data - CMDRAWDMP);
					}
					else if (data >= CMDVELCRV)
					{
						triggerVelocityCurveSendFunc(data - CMDVELCRV);
					}
					break;
				}
				phase = 0;
				break;
			default:			// error
				func = NULL;
				phase = 0;
				break;
			}
			break;
		case 2:	// data(1st)
		case 3:	// data(2nd~)
			if (func)
			{
				phase = func(data);
				if (!phase)
				{
					func = NULL;
				}
			}
			else
			{
				phase = 0;
			}
			break;
		default:	// error
			phase = 0;
			break;
		}
	}

	return;
}
