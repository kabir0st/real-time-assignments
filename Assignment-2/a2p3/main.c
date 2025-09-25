/*
    Part of the Real-Time Embedded Systems course at Halmstad University
    Copyright (c) 2017, Sebastian Kunze <sebastian.kunze@hh.se>
    All rights reserved.
	Wagner de Morais (Wagner.deMorais@hh.se)
*/

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib/expstruct.h"

#define LINE 32

#define ITERATION_BATCH_SIZE 5
const long long WAIT_TILL = 3;


void delay_ms(long milliseconds) {
    struct timespec req, rem;

    if (milliseconds <= 0) return;

    req.tv_sec = milliseconds / 1000;
    req.tv_nsec = (milliseconds % 1000) * 1000000L;

    // Loop until the full delay is completed, handling interruptions
    while (nanosleep(&req, &rem) == -1) {
        req = rem;
    }
}


void check_led (int *state, long long *timer) {
    if (*timer > WAIT_TILL) {
        if (*state) {
            printf("LED ON, Turning Off\n");
            *state = 0;
            *timer = 0;
            return;
        }
        printf("LED OFF, Turning On\n");
        *state = 1;
        *timer = 0;
        return;
    }
    return;
};

void initialize_exp_state(ExpProgramState *state) {
    state->x = 1;  // Start computing from e^1
    // Start from term n=1 since we want to compute e^x
    state->n = 1;
    // Start with term for n=0 which is 1
    state->last_calculated_term = 1.0;
    state->n_sum = 0.0;
    state->completed = 0;
    state->n_exp_int = 0;
    state->n_exp_fraction = 0;
}

void update_state_after_success_complete(ExpProgramState *state) {
    if (state-> x >=20) {
        return;
    }
    (state->x)++;  // Start computing from e^1
    // Start from term n=1 since we want to compute e^x
    state->n = 1;
    // Start with term for n=0 which is 1
    state->last_calculated_term = 1.0;
    state->n_sum = 0.0;
    state->completed = 0;
    state->n_exp_int = 0;
    state->n_exp_fraction = 0;
}



int main()
{
    ExpProgramState *state = malloc(sizeof(ExpProgramState));
    if (!state) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }

    long long timer = 0;
    int led_state = 0;

    initialize_exp_state(state);
    while(1) {
        // artificial delay cause otherwise the led toggles too fast
        // check if the led needs to be toggled
        check_led(&led_state, &timer);
        for (int i = 0; i < ITERATION_BATCH_SIZE && !state->completed; i++) {
            iexp(state);
            if (state->completed) {
                printf("e^%d = %d.%02d (computed with %d terms)\n", state->x, state->n_exp_int, state->n_exp_fraction, state->n);
                update_state_after_success_complete(state);
                break;
            }
        }
        delay_ms(500);
        timer++;
    }
    free(state);
    return 0;
};
