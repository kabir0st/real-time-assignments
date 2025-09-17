#include <stdlib.h>
#include <stdio.h>
#include "lib/iregister.h"

int main(){

    iRegister r;
    int initial_value;
    int i;
    char input_buffer[100];
    int valid_input = 0;

    printf("--------------------------------\n");
    printf("Initial value in decimal (or press Enter for default 1010101 binary = 85 decimal): ");

    // Get user input as string to handle empty input and invalid characters
    while (!valid_input) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
            // Check if user just pressed Enter (empty input)
            if (input_buffer[0] == '\n') {
                initial_value = 85; // 1010101 in binary = 85 in decimal
                printf("Initial value set to 85 (1010101 in binary)\n");
                valid_input = 1;
            } else {
                // Try to parse the input as integer
                if (sscanf(input_buffer, "%d", &initial_value) == 1) {
                    printf("Initial value set to %d\n", initial_value);
                    valid_input = 1;
                } else {
                    printf("Invalid input! Please enter a valid integer or press Enter for default: ");
                }
            }
        }
    }

    r.content = initial_value;

    printf("Initial Value In Binary Format: %s = %d\n", convert_to_binary(&r), r.content);
    printf("--------------------------------\n");

    // Handle bit position input with validation
    valid_input = 0;
    printf("Enter bit position to reset (0-31): ");
    while (!valid_input) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
            if (sscanf(input_buffer, "%d", &i) == 1) {
                if (i >= 0 && i <= 31) {
                    printf("Bit position set to %d\n", i);
                    valid_input = 1;
                } else {
                    printf("Invalid bit position! Please enter a number between 0 and 31: ");
                }
            } else {
                printf("Invalid input! Please enter a valid integer between 0 and 31: ");
            }
        }
    }
    printf("Before reseting: %s = %d\n", convert_to_binary(&r), r.content);
    resetBit(i, &r);
    printf("After reseting: %s = %d\n", convert_to_binary(&r), r.content);
    printf("--------------------------------\n");

    r.content = 10;
    printf("Operation Reseting all bits: ");
    printf("r.content: %s = %d\n", convert_to_binary(&r), r.content);
    resetAll(&r);
    printf("After reseting: %s = %d\n", convert_to_binary(&r), r.content);
    printf("--------------------------------\n");

    return 0;
}
