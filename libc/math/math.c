#include <math.h>

float sqrtf(float x) {
    if (x < 0.0f) return -1.0f; // error/NaN
    if (x == 0.0f) return 0.0f;

    float guess = x / 2.0f;
    for (int i = 0; i < 10; i++)
        guess = 0.5f * (guess + x / guess);

    return guess;
}

float expf(float x) {
    float sum = 1.0f;
    float term = 1.0f;

    for (int i = 1; i < 20; i++) {
        term *= x / i;
        sum += term;
    }

    return sum;
}

float powf(float base, float exp) {
    if (base <= 0.0f) return -1.0f; // crude error handling
    return expf(exp * logf(base));
}

float logf(float x) {
    if (x <= 0.0f) return -1.0f; // basic error handling
    float y = (x - 1) / (x + 1);
    //float y2 = y * y;
    float result = 0.0f;

    for (int i = 1; i < 10; i += 2) {
        result += (1.0f / i) * (i % 2 == 1 ? 1 : -1) * powf(y, i);
    }

    return 2 * result;
}


float sinf(float x) {
    float term = x;
    float sum = x;
    float x2 = x * x;

    for (int i = 1; i <= 5; ++i) {
        term *= -x2 / ((2 * i) * (2 * i + 1));
        sum += term;
    }

    return sum;
}

float cosf(float x) {
    float term = 1.0f;
    float sum = 1.0f;
    float x2 = x * x;

    for (int i = 1; i <= 5; ++i) {
        term *= -x2 / ((2 * i - 1) * (2 * i));
        sum += term;
    }

    return sum;
}

int abs(int x){
    return (x<0)?-x:x;
}