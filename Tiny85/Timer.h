/*
 * Timer.h
 *
 * Created: 2017-01-19 15:53:59
 *  Author: tkoene
 */ 


#ifndef TIMER_H_
#define TIMER_H_

void timer_Config();
void timer_NewPeriod(uint8_t period);
void timer_Start();
void timer_Stop();

#endif /* TIMER_H_ */