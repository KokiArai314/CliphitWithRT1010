/*
 * fsl_pit.c
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#include "fsl_pit.h"

/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.pit"
#endif


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Gets the instance from the base address to be used to gate or ungate the module clock
 *
 * @param base PIT peripheral base address
 *
 * @return The PIT instance
 */
static uint32_t PIT_GetInstance(PIT_Type *base);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief Pointers to PIT bases for each instance. */
static PIT_Type *const s_pitBases[] = PIT_BASE_PTRS;

#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
/*! @brief Pointers to PIT clocks for each instance. */
static const clock_ip_name_t s_pitClocks[] = PIT_CLOCKS;
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint32_t PIT_GetInstance(PIT_Type *base)
{
    uint32_t instance;

    /* Find the instance index from base address mappings. */
    for (instance = 0; instance < ARRAY_SIZE(s_pitBases); instance++)
    {
        if (s_pitBases[instance] == base)
        {
            break;
        }
    }

    assert(instance < ARRAY_SIZE(s_pitBases));

    return instance;
}

void PIT_Init(PIT_Type *base, const pit_config_t *config)
{
    assert(config);

#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    /* Ungate the PIT clock*/
    CLOCK_EnableClock(s_pitClocks[PIT_GetInstance(base)]);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */

#if defined(FSL_FEATURE_PIT_HAS_MDIS) && FSL_FEATURE_PIT_HAS_MDIS
    /* Enable PIT timers */
    base->MCR &= ~PIT_MCR_MDIS_MASK;
#endif
    /* Config timer operation when in debug mode */
    if (config->enableRunInDebug)
    {
        base->MCR &= ~PIT_MCR_FRZ_MASK;
    }
    else
    {
        base->MCR |= PIT_MCR_FRZ_MASK;
    }
}

void PIT_Deinit(PIT_Type *base)
{
#if defined(FSL_FEATURE_PIT_HAS_MDIS) && FSL_FEATURE_PIT_HAS_MDIS
    /* Disable PIT timers */
    base->MCR |= PIT_MCR_MDIS_MASK;
#endif

#if !(defined(FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL) && FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL)
    /* Gate the PIT clock*/
    CLOCK_DisableClock(s_pitClocks[PIT_GetInstance(base)]);
#endif /* FSL_SDK_DISABLE_DRIVER_CLOCK_CONTROL */
}

#if defined(FSL_FEATURE_PIT_HAS_LIFETIME_TIMER) && FSL_FEATURE_PIT_HAS_LIFETIME_TIMER

uint64_t PIT_GetLifetimeTimerCount(PIT_Type *base)
{
    uint32_t valueH = 0U;
    uint32_t valueL = 0U;

    /* LTMR64H should be read before LTMR64L */
    valueH = base->LTMR64H;
    valueL = base->LTMR64L;

    return (((uint64_t)valueH << 32U) + (uint64_t)(valueL));
}

#endif /* FSL_FEATURE_PIT_HAS_LIFETIME_TIMER */

