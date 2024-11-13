/*
	fader.h
	copy from x19850
*/

#ifndef FADER_H_
#define FADER_H_

#include "../definitions.h"

typedef struct fader_t_ {
	float		fGoal; //最終的に目指す値
	float		fRate; //1sampleごとにどれだけ値が動くか
	float		fCur;  //現在のfaderの値
} Fader_t;

static inline void xfaderSetup(Fader_t *ps){
	ps->fGoal = ps->fCur = 1.0f;
	ps->fRate = 0.0f;
}

static inline void xfaderStop(Fader_t *ps){
	ps->fGoal = 0.0f;
	ps->fRate = ps->fCur / (SAMPLING_RATE / 1000);	// 1msec fade
	if (ps->fCur > ps->fRate)
	{	// 1st substruct
		ps->fCur -= ps->fRate;
	}
}

/*
	クロスフェーダー実行部
*/
static inline int xfaderExecute(Fader_t *ps)
{
	int ret = 0;

	if (ps->fRate != 0.0f)
	{	// now running !
		if (ps->fGoal == ps->fCur)
		{	// equal! = Goal!
			ret = 1;
			ps->fRate = 0.0f;
		}
		else
		{
			if (ps->fGoal > ps->fCur)	//値が上がっていくFader
			{	// open !
				float fTemp = ps->fGoal - ps->fCur;

				if (fTemp > ps->fRate)
				{	// next !
					fTemp = ps->fCur;
					ps->fCur += ps->fRate;
					if (fTemp == ps->fCur)
					{
						ret = 1;
					}
				}
				else
				{	// end !
					ret = 1;
				}
			}
			else if (ps->fGoal < ps->fCur) //値が下がっていくFader
			{	// close !
				float fTemp = ps->fCur - ps->fGoal;

				if (fTemp > ps->fRate)
				{	// next !
					fTemp = ps->fCur;
					ps->fCur -= ps->fRate;
					if (fTemp == ps->fCur)
					{
						ret = 1;
					}
				}
				else
				{	// end !
					ret = 1;
				}
			}
			if (ret)
			{	// end
				ps->fCur = ps->fGoal;
				ps->fRate = 0.0f;
			}
		}
	}

	return ret;
}

#ifdef 	__cplusplus

#if 0
#include "MidiDebugMonitor/MidiDebugMonitor.h"
using namespace MidiDebugMonitor;
using namespace korg::midi;
#endif

class CFader
{
public:
	CFader() :
		m_sFader({0.0f,0.0f,0.0f})
	{
	}
	virtual ~CFader()
	{
	}

	void process()
	{
		xfaderExecute(&m_sFader);
	}

	void setGoal(float goal, int sample)
	{
		m_sFader.fRate = 0.0f;
		m_sFader.fGoal = goal;
		if (sample > 0)
		{
			float fTemp = m_sFader.fGoal - m_sFader.fCur;
			fTemp = fTemp < 0.0f ? - fTemp : fTemp;
			m_sFader.fRate = fTemp / (float)sample;
		}
		else
		{
			m_sFader.fCur = m_sFader.fGoal;
		}
	}
	void setGoal(float goal, float rate)
	{
		m_sFader.fRate = 0.0f;
		m_sFader.fGoal = goal;
		if (rate > 0.0f)
		{
			m_sFader.fRate = rate;
		}
		else
		{
			m_sFader.fCur = m_sFader.fGoal;
		}
	}

	float getCur()
	{
		return m_sFader.fCur;
	}

#if 0
	void debugView()
	{
		dprintf(SDIR_UARTMIDI, (char *)"\n %10.8f,%10.8f,%10.8f", m_sFader.fGoal, m_sFader.fRate, m_sFader.fCur);
	}
#endif

private:
	Fader_t m_sFader;
};

#endif	/* __cplusplus */

#endif /* FADER_H_ */
