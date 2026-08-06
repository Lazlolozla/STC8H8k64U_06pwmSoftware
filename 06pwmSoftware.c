/* 07pwmLedDesvanecimiento.c - PWM software 1kHz con desvanecimiento (STC8H @ 24MHz) */
#include "stc8h.h"

volatile uint8_t brillo_actual = 0;
static volatile uint8_t pwm_ciclo = 0;
static volatile uint8_t contador_10ms = 0;
static volatile uint8_t subiendo = 1;
volatile uint8_t flag_actualizar_brillo = 0;

/**
 * @brief ISR Timer0 - Vector 0x000B (interrupción #1)
 * Ejecutada cada 4µs. Genera PWM por comparación y marca flag cada ~10ms.
 */
void isr_timer0(void) __interrupt(1) {
    /* Comparación PWM: menor = ON, mayor o igual = OFF */
    if (pwm_ciclo < brillo_actual) {
        P2 &= ~(1 << 0);
    } else {
        P2 |= (1 << 0);
    }
    pwm_ciclo++;  /* Overflow uint8_t: 255→0 automático */

    /* Base de tiempo: cada 256 ticks (≈1ms) incrementar contador */
    if (pwm_ciclo == 0) {
        contador_10ms++;
        if (contador_10ms >= 10) {  /* 10ms entre actualizaciones de brillo */
            contador_10ms = 0;
            flag_actualizar_brillo = 1;
        }
    }
}

void main(void) {
    /* P2.0 salida cuasi-bidireccional */
    P2M1 &= ~(1 << 0);
    P2M0 &= ~(1 << 0);
    P2 |= (1 << 0);

    /* Timer0 modo 16-bit auto-reload, 12T forzado */
    AUXR &= ~(1 << 7);
    TMOD &= ~0x0F;

    /* 4µs @ 2MHz = 8 ticks → recarga = 65536-8 = 0xFFF8 */
    TH0 = 0xFF;
    TL0 = 0xF8;

    ET0 = 1;
    EA = 1;
    TR0 = 1;

    /* Superloop: solo responde a flag */
    while (1) {
        if (flag_actualizar_brillo == 1) {
            flag_actualizar_brillo = 0;

            if (subiendo == 1) {
                if (brillo_actual < 255) brillo_actual++;
                else subiendo = 0;
            } else {
                if (brillo_actual > 0) brillo_actual--;
                else subiendo = 1;
            }
        }
    }
}
