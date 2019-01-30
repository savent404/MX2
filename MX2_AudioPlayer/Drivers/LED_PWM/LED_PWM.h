#pragma once

#include "LED.h"
#include "tim.h"

/** Some private functions used in PWM LED */
static LED_Trigger_Method_t LED_Trigger_Method(LED_Message_t trigger_bcd);
static void LED_RGB_Output(uint16_t r, uint16_t g, uint16_t b, uint16_t l);
static void LED_RGB_Limited(uint16_t r, uint16_t g, uint16_t b, uint16_t l);
static void LED_RGB_Output_Light(uint16_t *colors, float light);
static LED_Message_t LED_RGB_Breath(uint32_t step, uint32_t step_ms, uint32_t period_ms);
static LED_Message_t LED_RGB_Toggle(uint32_t step, uint32_t step_ms);
static LED_Message_t LED_RGB_Pulse(uint32_t step_ms);
static LED_Message_t LED_RGB_SoftRise(uint32_t step, uint32_t step_ms, uint32_t total_ms);
static LED_Message_t LED_RGB_SoftRise_Single(uint8_t channel, uint32_t delay_ms, uint32_t step, uint32_t step_ms, uint32_t total_ms);
static LED_Message_t LED_RGB_SoftDown(uint32_t step, uint32_t step_ms, uint32_t total_ms);
static LED_Message_t LED_RGB_Charging(uint32_t step, uint32_t step_ms, uint32_t period_ms);
static LED_Message_t LED_RGB_Charged(uint32_t step, uint32_t step_ms, uint32_t period_ms);

#ifndef CHx_VAL
#define CHx_VAL(x) (TIM1->CCR##x)
#endif

bool LED_PWM_Init(void * arg);
void LED_PWM_Handle(PARA_DYNAMIC_t* ptr);
bool LED_PWM_Update(PARA_DYNAMIC_t* ptr);
