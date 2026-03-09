#include <stdlib.h>

#include "ema.h"

EmaFilterCoefficient* createEmaFilterCoefficient(float coefficient) {
    EmaFilterCoefficient *inst = (EmaFilterCoefficient*) malloc(sizeof(EmaFilterCoefficient));

    inst->coefficient = coefficient;
    inst->averageValue = 0;
    
    return inst;
}

int exponentialMovingAverage(
    int nextValue,
    EmaFilterCoefficient *filterSettings    
) {
    int avr = filterSettings->averageValue;

    filterSettings->averageValue = avr + (nextValue - avr) * filterSettings->coefficient;

    return filterSettings->averageValue;
}