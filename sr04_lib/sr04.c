/*
 * this software is for running SR-04 with stm32f103c8 without delay by using timer
 * PA8 is connected to ECHO pin and it is TIM1 ch1 & ch2 in input capture mode
 * PA10 is connected to TRIG pin and it is TIM1 Ch3 in PWM mode
 *
 * Attention :
 * this file contains the IRQ_Handler function for timer 1 update interrupt
 * do not use TIM1 and PA8 and PA10 in other part of your code
 * you can add these files to your project with HAL .
 * just do'nt do any configuration for TIM1.
 *
 * launch :
 * 1.include sr04.h in main.c
 * 2.call sr04_init() in main function
 * 3.distance is going to store in a global variable named dis
 *
 *
 * Licenses : CC BY-NC
 * elcen.ir
 *
 */


#include "sr04.h"

//this variable stores the measured distance by SR-04
float sr04_dis;


/*
 * this is the initialization function , write it in main function
 * after this function is called , you can read the distance from "dis" variable
 */

void sr04_init(void){

	//GPIOA clock enable
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

	// PA8 input no pull
	GPIOA->CRH &= ~GPIO_CRH_MODE8;
	GPIOA->CRH &= ~GPIO_CRH_CNF8;
	GPIOA->CRH |= GPIO_CRH_CNF8_0;

	//PA10 output PP
	GPIOA->CRH &= ~(GPIO_CRH_MODE10|GPIO_CRH_CNF10);
	GPIOA->CRH |= (GPIO_CRH_MODE10_1 | GPIO_CRH_CNF10_1);

	//TIM1 Clock enble
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	TIM1->CR1 &= ~TIM_CR1_CKD;
	TIM1->CR1 &= ~TIM_CR1_CMS;
	TIM1->CR1 &= ~TIM_CR1_DIR;

	//Counted period = 100ms
	TIM1->ARR = (uint16_t)49999;
	TIM1->PSC = (uint16_t)143;

	//TIM1 ch1 & ch2 input capture
	TIM1->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S);
	TIM1->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_1);

	TIM1->CCER &= ~TIM_CCER_CC1P;
	TIM1->CCER |= TIM_CCER_CC2P;

	TIM1->CCR3 = (uint16_t)6;

	//TIM1 ch3 PWM
	TIM1->CCMR2 |= TIM_CCMR2_CC3S;
	TIM1->CCMR2 |= TIM_CCMR2_OC3PE;
	TIM1->CCMR2 &= ~TIM_CCMR2_CC3S;

	TIM1->CCMR2 &= ~TIM_CCMR2_OC3M;
	TIM1->CCMR2 |= ( TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 );

	TIM1->CCER &= ~TIM_CCER_CC3P;

	TIM1->CCER |= ( TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E );

	TIM1->BDTR |= TIM_BDTR_MOE;

	TIM1->EGR = TIM_EGR_UG;

	//enabling TIM1 ch2 interrupt
	TIM1->SR = ~TIM_SR_CC2IF;
	TIM1->DIER |= TIM_DIER_CC2IE;

	uint32_t PG= NVIC_GetPriorityGrouping();
	NVIC_SetPriority(TIM1_CC_IRQn,NVIC_EncodePriority(PG, 2, 0));

	NVIC_EnableIRQ(TIM1_CC_IRQn);

	//starting TIM1
	TIM1->CR1 |= TIM_CR1_CEN;

}


/*
 * at the end of each sampling , this function is called by IRQ_Handler function
 * this is the only thing CPU dose for each sampling
 */

void sr04_callback(void){
	sr04_dis =  (float)( TIM1->CCR2 - TIM1->CCR1 ) * 0.034f ;
}



/*
 * this is IRQ_Handler function for TIM1 update interrupt
 * these function address's are written in interrupt vector table in start up file.
 * in HAL library these functions are written in it.c file
 *
 * IRQHandler functions are first functions that are called after an interrupt is happened
 * they do these:
 *
 * 1.finding interrupt request
 * 2.clearing interrupt request flag
 * 3.calling callback function
 *
 */

void TIM1_CC_IRQHandler(void){

	if( (TIM1->DIER & TIM_DIER_CC2IE) && (TIM1->SR & TIM_SR_CC2IF)){
			TIM1->SR = ~TIM_SR_CC2IF;
			sr04_callback();}

}



