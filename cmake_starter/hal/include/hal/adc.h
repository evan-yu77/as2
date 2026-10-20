#ifndef ADC_H
#define ADC_H

void ADC_init(void);
int ADC_getRawReading(void);
void ADC_cleanup(void);
double ADC_getVoltageReading(void);

#endif