/*
 * this is written by elcen.ir
 * this software is for running SR-04 with stm32f103c8 without delay by using timer
 * PA8 is connected to ECHO pin and it is TIM1 ch1 & ch2 in input capture mode
 * PA10 is connected to TRIG pin and it is TIM1 Ch3 in PWM mode
 *
 * Attention :
 * pay attention that this file contains the IRQ_Handler function for timer 1 update interrupt
 * do not use TIM1 and PA8 and PA10 in other part of your code
 *
 * launch :
 * include sr04.h in main.c
 * call sr04_init() in main function
 * distance is going to store in a global variable named dis
 *
 */


#include "sr04.h"

float dis;


void sr04_callback(void){
	dis =  (float)( TIM1->CCR2 - TIM1->CCR1 ) * 0.034f ;
}


void sr04_init(void){

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	GPIOA->CRH &= ~GPIO_CRH_MODE8;
	GPIOA->CRH &= ~GPIO_CRH_CNF8;
	GPIOA->CRH |= GPIO_CRH_CNF8_0;

	GPIOA->CRH &= ~(GPIO_CRH_MODE10|GPIO_CRH_CNF10);
	GPIOA->CRH |= (GPIO_CRH_MODE10_1 | GPIO_CRH_CNF10_1);

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	TIM1->CR1 &= ~TIM_CR1_CKD;
	TIM1->CR1 &= ~TIM_CR1_CMS;
	TIM1->CR1 &= ~TIM_CR1_DIR;

	TIM1->ARR = (uint16_t)49999;
	TIM1->PSC = (uint16_t)143;

	TIM1->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S);
	TIM1->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1);

	TIM1->CCER &= ~TIM_CCER_CC1P;
	TIM1->CCER |= TIM_CCER_CC2P;

	TIM1->CCR3 = (uint16_t)6;

	TIM1->CCMR2 |= TIM_CCMR2_CC3S;
	TIM1->CCMR2 |= TIM_CCMR2_OC3PE;
	TIM1->CCMR2 &= ~TIM_CCMR2_CC3S;

	TIM1->CCMR2 &= ~TIM_CCMR2_OC3M;
	TIM1->CCMR2 |= ( TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 );

	TIM1->CCER &= ~TIM_CCER_CC3P;

	TIM1->CCER |= ( TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E );

	TIM1->BDTR |= TIM_BDTR_MOE;

	TIM1->EGR = TIM_EGR_UG;

	TIM1->SR = ~TIM_SR_CC2IF;
	TIM1->DIER |= TIM_DIER_CC2IE;

	uint32_t PG= NVIC_GetPriorityGrouping();
	NVIC_SetPriority(TIM1_CC_IRQn,NVIC_EncodePriority(PG, 2, 0));

	NVIC_EnableIRQ(TIM1_CC_IRQn);

	TIM1->CR1 |= TIM_CR1_CEN;

}



void TIM1_CC_IRQHandler(void){

	if( (TIM1->DIER & TIM_DIER_CC2IE) && (TIM1->SR & TIM_SR_CC2IF)){
			TIM1->SR = ~TIM_SR_CC2IF;
			sr04_callback();}

}



