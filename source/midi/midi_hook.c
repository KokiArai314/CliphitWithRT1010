/*
 * midi_hook.c
 *
 *  Created on: 2020/09/03
 *      Author: akino
 */

#include <stdint.h>
#include "circularbuffer.h"
#include "midi_hook.h"

#include "../midi_debug_monitor/midi_debug_monitor.h"

/*
 * uart --- rxbuf --- MIDI_IF_IDLE() --- dbgrxbuf
 *
 * usb --- USB_MIDI_IDLE --- s_MidiIfTxBuff --- midi_IF_TxInit --- dbgusbrxbuf
 *
 */

#define RXBUFSIZ 512
#define USBRXBUFSIZ	512

static uint8_t dbgrxbuf[RXBUFSIZ];
static uint8_t dbgusbrxbuf[USBRXBUFSIZ];

static CCRBUF_t dbgrxccrbuf = {{0}, sizeof(dbgrxbuf), dbgrxbuf};
static CCRBUF_t dbgusbrxccrbuf = {{0}, sizeof(dbgusbrxbuf), dbgusbrxbuf};

static const unsigned char header[] = {0xf0,0x42,0x21,0x78,0x7f,0x72,0x7f};

void midi_hook_entry(uint8_t data, void (*func)(uint8_t), uint8_t attach)
{	// デバッグモニタフック
	static int pos = 0;	// -1:in monitor, 0~:check position
	static int cnt = 0;	// '@'counter
	static int snd = 0;	// 1:send enable

	if (pos < 0)
	{	// デバッグモニタメッセージ中
		if (data != 0xf7)
		{	// not EOX
			if (data != 0)
			{	// not end
				if (data == '#')
				{	// in/out command
					if (++cnt == 4)
					{	// exit
						cnt = 0;
						putccrbuf(&dbgrxccrbuf, '@');
						putccrbuf(&dbgrxccrbuf, 0);
//						dprintf(SDIR_USBMIDI, "\n exit");
						pos = 0;
					}
					else if (cnt == 3)
					{	// enter
						putccrbuf(&dbgrxccrbuf, '@');
						putccrbuf(&dbgrxccrbuf, '@');
						putccrbuf(&dbgrxccrbuf, '@');
//						dprintf(SDIR_USBMIDI, "\n enter");
					}
				}
				else if (cnt != 3)
				{	// in/out command count reset
					cnt = 0;
				}
				else
				{	// now entered
					if (data == '@')
					{	// data change for blocking exiting !
						data = '#';
					}
					putccrbuf(&dbgrxccrbuf, data);
				}
			}
			else if (cnt == 3)
			{	// string end mark
				putccrbuf(&dbgrxccrbuf, data);
			}
			if ((cnt == 0) && (pos < 0))
			{	// other thru
				if (!snd)
				{	// now not send
					snd = 1;
					if (attach)
					{
						for (int i = 0; i < sizeof(header); i++)
						{
							func(header[i]);
						}
					}
				}
			}
		}
		else
		{	// end
			pos = 0;
		}
	}
	else if (header[pos] == data)
	{	// 検出データ列一致
		snd = 0;
		if (++pos == sizeof(header))
		{	// 検出データ列一致終了
			pos = -1;
		}
	}
	else if (pos)
	{	// 途中まで一致
		if (attach)
		{
			for (int i = 0; i < pos; i++)
			{	// 途中までを吐き出し
				func(header[i]);
			}
		}
		pos = 0;
		snd = 1;
	}
	else
	{	// 不一致
		snd = 1;
	}

	if (snd)
	{	// 送信
		if (attach)
		{
			func(data);
		}
	}

	return;
}

int midi_hook_usb_entry(uint8_t *bp, uint16_t size)
{
	static int pos = 0;	// -1:in monitor, 0~:check position
	static int cnt = 0;	// '@'counter
	static int snd = 0;	// 1:send enable
	int retcnt = 0;

	for (int i = 0; i < size; i++)
	{
		uint8_t data = bp[i];

		if (pos < 0)
		{	// デバッグモニタメッセージ中
			if (data != 0xf7)
			{	// not EOX
				if (data != 0)
				{	// not end
					if (data == '#')
					{	// in/out command
						if (++cnt == 4)
						{	// exit
							cnt = 0;
							putccrbuf(&dbgusbrxccrbuf, '@');
							putccrbuf(&dbgusbrxccrbuf, 0);
//x							dprintf(SDIR_UARTMIDI, "\n exit");	// ここでは使えない！
							pos = 0;
						}
						else if (cnt == 3)
						{	// enter
							putccrbuf(&dbgusbrxccrbuf, '@');
							putccrbuf(&dbgusbrxccrbuf, '@');
							putccrbuf(&dbgusbrxccrbuf, '@');
//x							dprintf(SDIR_UARTMIDI, "\n enter");	// ここでは使えない！
						}
					}
					else if (cnt != 3)
					{	// in/out command count reset
						cnt = 0;
					}
					else
					{	// now entered
						if (data == '@')
						{	// data change for blocking exiting !
							data = '#';
						}
						putccrbuf(&dbgusbrxccrbuf, data);
					}
				}
				else if (cnt == 3)
				{	// string end mark
					putccrbuf(&dbgusbrxccrbuf, data);
				}
				if ((cnt == 0) && (pos < 0))
				{	// other thru
					if (!snd)
					{	// now not send
						snd = 1;
						for (int i = 0; i < sizeof(header); i++)
						{
							bp[retcnt++] = header[i];
						}
					}
				}
			}
			else
			{	// end
				pos = 0;
			}
		}
		else if (header[pos] == data)
		{	// 検出データ列一致
			snd = 0;
			if (++pos == sizeof(header))
			{	// 検出データ列一致終了
				pos = -1;
			}
		}
		else if (pos)
		{	// 途中まで一致
			for (int i = 0; i < pos; i++)
			{	// 途中までを吐き出し
				bp[retcnt++] = header[i];
			}
			pos = 0;
			snd = 1;
		}
		else
		{	// 不一致
			snd = 1;
		}

		if (snd)
		{	// 送信
			bp[retcnt++] = data;
		}
	}

	return retcnt;
}

void midi_hook_exec(void)
{
	//static volatile int uart_exec = 0;
	static volatile int usb_exec = 0;
/*
	if (!uart_exec)
	{
		uart_exec = 1;	// 再入禁止
		{
			int count = getccrcnt(&dbgrxccrbuf);

			while (count--)
			{
				int data = getccrbuf(&dbgrxccrbuf);

				if (data < 0)
				{
					break;
				}
				MidiDebugMonitorCommandIn(data, eSrcUartMIDI);
			}
		}
		uart_exec = 0;	// 再入許可
	}*/
	if (!usb_exec)
	{
		usb_exec = 1;	// 再入禁止
		{
			int count = getccrcnt(&dbgusbrxccrbuf);

			while (count--)
			{
				int data = getccrbuf(&dbgusbrxccrbuf);

				if (data < 0)
				{
					break;
				}
				MidiDebugMonitorCommandIn(data, eSrcUsbMIDI);
			}
		}
		usb_exec = 0;	// 再入許可
	}

	return;
}
