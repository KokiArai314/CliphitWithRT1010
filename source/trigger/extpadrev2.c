/*
 * extpadrev2.c
 *
 *  Created on: 2021/06/03
 *      Author: akino
 */

#include <stdint.h>
#include "../peripheral/p_adc.h"
#include "trigger.h"
#include "extpad.h"
#include "../assigner/assigner.h"
#include "../voice/CLIPHIT2_voice.h"

#include "../midi_debug_monitor/midi_debug_monitor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define INPUTFILTER (1) // 0:none,1:BPF,2:LPF
#define INPUTFILTER600  // INPUTFILTER=1の時に1kHz->600Hzへ
#define ENVMASK         // 定義すると envelope mask 有効

typedef struct {
  uint8_t flag;
  uint16_t maskCount;
  uint16_t triggervalue[EXTPAD_ON_COUNT_MAX];
  int16_t triggermin;
  int16_t triggermax;
#if INPUTFILTER
#if (INPUTFILTER == 1)
  int16_t filter1[2];
#endif //#if INPUTFILTER EQ 1
#if (INPUTFILTER == 2)
  int16_t filter1;
#endif //#if INPUTFILTER EQ 1
  int16_t filter2[2];
#endif //#if INPUTFILTER
#if defined(ENVMASK)
  int16_t envMaskCount;
  int16_t envTriggerMin;
  int16_t envTriggerMax;
  int16_t envPreviousLevel;
#endif //#if defined(ENVMASK)
} EXTPADWORK_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
//extern void midi_IF_send_usb_blocking(uint8_t *str, uint16_t cnt);
//extern void midi_IF_send_uart_blocking(uint8_t *str, uint16_t cnt);
void jobTimeStart(int index);
void jobTimeStop(int index);
void jobTimeInterval(int index);

extern void AdcAudioSet(uint16_t value, uint8_t ch); // for debug

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if INPUTFILTER
#if (INPUTFILTER == 1)
static EXTPADWORK_t extPadWork[EXTPAD_NUM_OF] = {{.flag = EXTPAD_VEL_WINDOW, .filter1 = {2048, 2048}},
                                                 {.flag = EXTPAD_VEL_WINDOW, .filter1 = {2048, 2048}},
                                                 {.flag = EXTPAD_VEL_WINDOW, .filter1 = {2048, 2048}},
                                                 {.flag = EXTPAD_VEL_WINDOW, .filter1 = {2048, 2048}}};
#endif //#if (INPUTFILTER == 1)
#if (INPUTFILTER == 2)
static EXTPADWORK_t extPadWork[EXTPAD_NUM_OF] = {{.flag = EXTPAD_VEL_WINDOW, .filter1 = 2048},
                                                 {.flag = EXTPAD_VEL_WINDOW, .filter1 = 2048},
                                                 {.flag = EXTPAD_VEL_WINDOW, .filter1 = 2048},
                                                 {.flag = EXTPAD_VEL_WINDOW, .filter1 = 2048}};
#endif //#if (INPUTFILTER == 2)
#else  //#if INPUTFILTER
static EXTPADWORK_t extPadWork[EXTPAD_NUM_OF] = {
    {.flag = EXTPAD_VEL_WINDOW}, {.flag = EXTPAD_VEL_WINDOW}, {.flag = EXTPAD_VEL_WINDOW}, {.flag = EXTPAD_VEL_WINDOW}};
#endif //#if INPUTFILTER

/*******************************************************************************
 * Code
 ******************************************************************************/

static uint16_t getOtherLevel(int id) {
  uint16_t ret = 0;
  int pairId = id ^ 1;

  for (int i = 0; i < EXTPAD_NUM_OF; i++) {
    if (i != id) {
      //現段階の他PADのTriggerPeak2Peak値を取得
      uint16_t trg = extPadWork[i].triggermax - extPadWork[i].triggermin;

      //pairのPADなら1/2, pair以外のPADなら1/4がxTalkの減少値になる
      trg >>= (i == pairId) ? 1 : 2;
      if (ret < trg) //全てのPADのうち最も値の大きいPADのTriggerPeak2Peak値を返す
      {
        ret = trg;
      }
    }
  }

  return ret;
}

/* --- extPad --- */
void extPadRev2(TRIGSCN_t *ptrigscn) {
  EXTPAD_t *pExtPad = &(ptrigscn->extPad);
  EXTPADWORK_t *pExtPadWork = &(extPadWork[pExtPad->id]);
  uint16_t trigger = adcGetValue(ptrigscn->adcCh1st);
  uint16_t sigmax = pExtPad->vel.smax; // / 2;	//source maxをsigmaxへ入れる
  uint16_t adcValue = trigger;         //for debug

  // adcのフラグをクリア
  adcClearFlag(ptrigscn->adcCh1st, ptrigscn->adcCh2nd);
  /**
	 * ADの生値をSoftware Filter 処理
	 */
#if INPUTFILTER
  {
#if (INPUTFILTER == 1)
#ifdef INPUTFILTER600
    int16_t fil1 = (trigger - (pExtPadWork->filter1[0] + pExtPadWork->filter1[1]) / 2) * 3 / 16;
    int16_t fil2 = fil1 + (pExtPadWork->filter2[0] * 7 / 4 - pExtPadWork->filter2[1] * 8 / 10);
#else  //#ifdef INPUTFILTER600
    int16_t fil1 = (trigger - (pExtPadWork->filter1[0] + pExtPadWork->filter1[1]) / 2) / 3;
    int16_t fil2 = fil1 + (pExtPadWork->filter2[0] * 3 / 2 - pExtPadWork->filter2[1] * 5 / 8);
#endif //#ifdef INPUTFILTER600
    pExtPadWork->filter1[1] = pExtPadWork->filter1[0];
    pExtPadWork->filter1[0] = trigger;
#endif //#if (INPUTFILTER == 1)
#if (INPUTFILTER == 2)
    int16_t filter = trigger - 2048;
    int16_t fil1 = (filter + pExtPadWork->filter1) / 8;
    int16_t fil2 = fil1 + (pExtPadWork->filter2[0] * 9 / 8 - pExtPadWork->filter2[1] * 3 / 8);
    pExtPadWork->filter1 = filter;
#endif //#if INPUTFILTER
    pExtPadWork->filter2[1] = pExtPadWork->filter2[0];
    pExtPadWork->filter2[0] = fil2;
    fil2 += 2048;
    trigger = fil2 < 0 ? 0 : fil2 > 4095 ? 4095 : fil2;
  }
#endif //#if (INPUTFILTER == 2)
  /*
	 * AD生値をスケールアップする(->決め打ちの定数でノーマライズ)
	 * 中心点は2048
	 * 2048 - (extPad.vel->smax)の値が4095になるようノーマライズ
	 * ノーマライズ後の値は変数「trigger」に入る
	*/
  MINMAXCONV_t cnvIn = {2048 - sigmax, 2048 + sigmax, 0,
                        4095}; //{2048-sigmax, 2048+sigmax, 0, 4095}; analog入力の正規化
  MINMAXCONV_t cnvOut = {0, 4095, pExtPad->vel.tmin, pExtPad->vel.tmax};
  trigger = minmaxconv(trigger, &cnvIn);
  uint16_t onLevel = pExtPad->vel.smin; // pExtPad->onLvl;  * EXTPAD_46_VEL_MAX50 / pExtPad->vel.smax;

  /**
	 * Trigger value progression  ---^v^v^v^v^---
	 */
  if (pExtPadWork->flag && (pExtPadWork->flag < pExtPad->velWnd)) {
    /**
		 * in velocity window
		 */
    //Trigger値を保存する配列の更新
    for (int i = 0; i < pExtPad->onCnt - 1; i++) {
      pExtPadWork->triggervalue[i] = pExtPadWork->triggervalue[i + 1];
    }
    pExtPadWork->triggervalue[pExtPad->onCnt - 1] = trigger;

    //Trigger値の最小、最大の更新 ->velocity windowの中にいるときだけTrigger値は更新される
    if (trigger < pExtPadWork->triggermin) {
      pExtPadWork->triggermin = trigger;
    } else if (trigger > pExtPadWork->triggermax) {
      pExtPadWork->triggermax = trigger;
    }
#if 1
    /**
		 * ☆velocity window 内におけるxTalk処理
		 * 　velocity window 半分に来た時点でほかのPADのPeak2Peak値を取得する
		 * Trigger値はPADがvelocity windowに入っていないと有効にならない
		 * 「現時点(velocity window半分)のPeak2Peak値が、ONになっている他PADのPeak2Peak値よりも小さい場合、発音をキャンセルする
		 * この時他PADのPaek2Peak値はPairなPADであるか否かによって-6dB,もしくは-12dBされた後比較される
		 * 
		 * Velocity windowに入る前にもxTalkによる処理があるので留意
		 */
    if (pExtPad->crsCan) //クロストーク処理
    {
      if (pExtPadWork->flag == (pExtPad->velWnd / 2)) {
        //Trigger値のPeak2Peakを取得
        uint16_t triggerLevel = pExtPadWork->triggermax - pExtPadWork->triggermin;
        //他PADのTrigger値のPeak2Peakを取得
        uint16_t xtalkcan = getOtherLevel(pExtPad->id); // -6dB(1/2, pair) or -12dB(1/4, notpair)

        if (xtalkcan) {
          /**
					 * xTalkの算出によって計算されたPeak2Peak値を現在のPeak2Peak値から削減する
					 */
          xtalkcan >>= pExtPad->crsCan; // -> -12dB(1/4,pair) or -18dB(1/8,notpair)
          if (triggerLevel > xtalkcan) {
            triggerLevel -= xtalkcan;
          } else {
            triggerLevel = 0;
          }
          /**
					 * 他PADのPeak2Peak値分減らした後、onLevelに満たなかったらキャンセル!
					 */
          if (triggerLevel <= onLevel) { // ===================================================== cancel !
            pExtPadWork->flag = 0;
            if (trigger_debug_flag) {
              //dprintf(SDIR_USBMIDI, "\n trigger cancel %d !", pExtPad->id);
            }
          }
        }
      }
    }
#endif
  } else {
    /**
		 * NOT in Velocity Window (=off or release)
		 */
    pExtPadWork->triggermin = trigger;
    pExtPadWork->triggermax = trigger;

    //Trigger値を保存する配列からmin, maxを求める
    for (int i = 0; i < pExtPad->onCnt - 1; i++) {
      int value = pExtPadWork->triggervalue[i + 1];

      pExtPadWork->triggervalue[i] = value;
      if (value < pExtPadWork->triggermin) {
        pExtPadWork->triggermin = value;
      } else if (value > pExtPadWork->triggermax) {
        pExtPadWork->triggermax = value;
      }
    }
    pExtPadWork->triggervalue[pExtPad->onCnt - 1] = trigger;
  }

  /**
	 * Envelope Mask initiation and progression ---^v^v^v^v^---
	 */
  if (!pExtPadWork->envMaskCount) {
    /**
		 * envelope masking initiation (envMaskCount == 0のとき必ず再始動)
		 */
    pExtPadWork->envPreviousLevel = pExtPadWork->envTriggerMax - pExtPadWork->envTriggerMin +
                                    onLevel; //次のonになる閾値 前のmaskのトリガー値を保存しとく
    pExtPadWork->envTriggerMax = pExtPadWork->envTriggerMin = trigger;
    pExtPadWork->envMaskCount =
        pExtPad->velWnd + pExtPad->mskTim * 3 - 1; // x3 scan cycle (=env mask window size), 175sample
  } else {
    /**
		 * envelope trigger min, max progression
		 * env maskごとに最大値と最低値を保存しておく Trigger値とは別に保存してるので注意!
		 */
    if (pExtPadWork->envTriggerMax < trigger) {
      pExtPadWork->envTriggerMax = trigger;
    }
    if (pExtPadWork->envTriggerMin > trigger) {
      pExtPadWork->envTriggerMin = trigger;
    }
    /**
		 * envMaskCountをデクリメント(常に行われる)
		 * envlope maskの処理は周期的に行われることになる
		 */
    pExtPadWork->envMaskCount--;
  }

  /**
	 * Trigger Detection ---^v^v^v^v^---
	 */

  // !!! ここで変数「trigger」がPeak2Peakに代わっていることに注意 !!!!!!
  // !!! もともとはAD値をスケールアップしたセンサ値               !!!!!!
  trigger = pExtPadWork->triggermax - pExtPadWork->triggermin;

  // maskCountのデクリメント いろいろなフラグにかかわってくるので先頭で行う
  if (pExtPadWork->maskCount) pExtPadWork->maskCount--;

  if (pExtPadWork->flag == 0) {
    /**
		 * in OFF
		 */
    uint16_t triggerLevel = trigger;

    /**
		 * Velocity windowに入る前のxTalk処理
		 * やっていることはVelocity Window半分時点でのxTalk処理と変わらない
		 */
    if (pExtPad->crsCan) {
      uint16_t xtalkcan = getOtherLevel(pExtPad->id); // -6dB(1/2) or -12dB(1/4)

      if (xtalkcan) {
        xtalkcan >>= pExtPad->crsCan; // -> -12dB(1/4) or -18dB(1/8)

        if (triggerLevel > xtalkcan) {
          triggerLevel -= xtalkcan;
        } else {
          triggerLevel = 0;
        }
      }
    }

    /**
		 * envelope mask処理
		 */
    if (triggerLevel >
        pExtPadWork
            ->envPreviousLevel) //envelopeMaskを現在のPeak2Peak値が乗り越えた envPreviousLevelの中にonLevelも含まれていることに注意！
    {
      if (pExtPadWork->maskCount == 0) //再発音禁止期間じゃないことを確認
      {
        //===============================================Velocity windowに突入
        pExtPadWork->flag++; //pExtPadWork->flagはここで初めて1にインクリ 以降は↓のvelWndの中で増えていく
        /**
				 * envelope Mask の初期化
				 * OFF -> VelocityWindow時にenvelope Maskは再始動する
				 */
        pExtPadWork->envMaskCount =
            pExtPad->mskTim * 3 -
            1; // x2 scan cycle ここだとvelWND分の長さが入っていない!? offset的なものを調節しているのかな
        pExtPadWork->envTriggerMax = pExtPadWork->triggermax; //現在のenvTrigger値をtrigger値の最大と最小で上書き
        pExtPadWork->envTriggerMin = pExtPadWork->triggermin;
      }
    }
  } else if (pExtPadWork->flag < pExtPad->velWnd) //pExtPadWork->flag != 0...かつ velWndに到達していない
  {
    /**
		 *Velocity Windowの処理
		 */
    pExtPadWork->flag++; //flagのインクリメント flagはvelocitywindowの中では勝手にインクリメントされる
    if (pExtPadWork->flag == pExtPad->velWnd) {
      /**
			 * Velocity Windowの端までとどいたら....
			 *  ======================================= trigger on !=================================================
			 */
      uint16_t velocity = trigger;
      uint16_t sigmin = pExtPad->vel.smin; // * 4095 / pExtPad->vel.smax;

      velocity = velocity < sigmin ? 0 : minmaxconvtbl(velocity, &cnvOut, pExtPad->id + 10);
      if (ptrigscn->enable) {
        uint8_t txbuf[6] = {0x90 + TRIGGER_CHANNEL, EXTPAD_NOTE_BASE, 0,  // note on
                            0x90 + TRIGGER_CHANNEL, EXTPAD_NOTE_BASE, 0}; // note off

        if (velocity) {
          txbuf[1] += pExtPad->id;
          txbuf[2] = velocity;
          txbuf[4] += pExtPad->id;
          //midi_IF_send_uart_blocking(txbuf, 6);
        }
      }
      if (trigger_debug_flag) {
        //jobTimeStop(3);
        dprintf(SDIR_USBMIDI, "\n %c ext pad on %1d, %3d(%4d)(%4d)", ptrigscn->enable ? 'S' : '-', pExtPad->id,
                velocity, trigger, adcValue);
      }
      //entryVcb(0, ((float)velocity / 127.0f ));
      //entryVoice(pExtPad->id, 0, ((float)velocity / 127.0f ));
      entryVoice(pExtPad->id, 0, (float)velocity / 127.0f);
      pExtPadWork->maskCount = pExtPad->mskTim * 3; // scan cycle
                                                    /**
			 * Trigger On時の処理 ここまで =============================================================================
			 */
    }
  } else {
    /**
		 * In Release
		 */
    if (pExtPadWork->maskCount == 0) {
      if (trigger_debug_flag) {
        //dprintf(SDIR_USBMIDI, "\n %c ext pad off %d", ptrigscn->enable ? 'S' : '-', pExtPad->id);
      }
      //発音後、一定期間はflagが0にならない -> triggerはretrigger maskが終了するまでは変化しないはず
      pExtPadWork->flag = 0; //mask期間終了 再発音
    }
  }

  return;
}
