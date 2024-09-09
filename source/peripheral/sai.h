/*
 * sai.h
 *
 *  Created on: 2024/09/09
 *      Author: koki_arai
 */

#ifndef SAI_H_
#define SAI_H_

#define BUFFER_SIZE (4*4*2*2)//(256)	// 128:?(16fs),256:ok,512:ok
#define BUFFER_NUM  (2)
#define CODEC_AUDIO_SAMPLE_RATE (kSAI_SampleRate44100Hz)
#define CODEC_AUDIO_DATA_CHANNEL (2U)
#define CODEC_AUDIO_BIT_WIDTH (kSAI_WordWidth24bits)

#endif /* SAI_H_ */
