/*
 * rtt_debugger.h
 *
 *  Created on: 2024/10/04
 *      Author: koki_arai
 */

#ifndef RTT_DEBUGGER_H_
#define RTT_DEBUGGER_H_

#include "lib/SEGGER_RTT.h"


void rtt_debugger_init();

void rtt_debug_with_jscope(int channel,uint32_t val);



#endif /* RTT_DEBUGGER_H_ */
