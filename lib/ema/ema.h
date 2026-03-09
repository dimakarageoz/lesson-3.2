typedef struct EmaFilterCoefficient {
    int averageValue;
    float coefficient;
} EmaFilterCoefficient;

EmaFilterCoefficient* createEmaFilterCoefficient(float coefficient);

int exponentialMovingAverage(
    int nextValue,
    EmaFilterCoefficient *filterSettings
);