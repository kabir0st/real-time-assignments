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

    printf("Initial Value In Binary Format: %s = %d\n", reg2str(r), r.content);
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
    printf("BE reseting: %s = %d\n", reg2str(r), r.content);
    resetBit(i, &r);
    printf("AF reseting: %s = %d\n", reg2str(r), r.content);
    printf("--------------------------------\n");


    // Implementing resetAll
    r.content = initial_value;
    printf("Operation Reseting all bits: ");
    printf("r.content: %s = %d\n", reg2str(r), r.content);
    resetAll(&r);
    printf("AF reseting: %s = %d\n", reg2str(r), r.content);
    printf("--------------------------------\n");

    // Implementing setBit
    r.content = initial_value;
    valid_input = 0;
    printf("Enter bit position to set (0-31): ");
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
    printf("BE setting: %s = %d\n", reg2str(r), r.content);
    setBit(i, &r);
    printf("AF setting: %s = %d\n", reg2str(r), r.content);
    printf("--------------------------------\n");

    // Implementing setAll
    r.content = 0; // Start with all bits cleared
    printf("Operation Setting all bits: ");
    printf("r.content: %s = %d\n", reg2str(r), r.content);
    setAll(&r);
    printf("AF setting all: %s = %d\n", reg2str(r), r.content);
    printf("--------------------------------\n");

    // Implementing getBit
    r.content = initial_value;
    printf("Testing getBit function: ");
    printf("r.content: %s = %d\n", reg2str(r), r.content);
    printf("Enter bit position to read (0-31): ");
    valid_input = 0;
    while (!valid_input) {
        if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
            if (sscanf(input_buffer, "%d", &i) == 1) {
                if (i >= 0 && i <= 31) {
                    printf("Reading bit at position %d: %d\n", i, getBit(i, &r));
                    valid_input = 1;
                } else {
                    printf("Invalid bit position! Please enter a number between 0 and 31: ");
                }
            } else {
                printf("Invalid input! Please enter a valid integer between 0 and 31: ");
            }
        }
    }
    printf("--------------------------------\n");

    // Implementing nibble operations
    r.content = initial_value;
    printf("Testing nibble operations: ");
    printf("r.content: %s = %d\n", reg2str(r), r.content);
    printf("Lower nibble (position 1): %d\n", getNibble(1, &r));
    printf("Upper nibble (position 2): %d\n", getNibble(2, &r));

    printf("Setting lower nibble to 15 (1111 binary)\n");
    assignNibble(15, 1, &r);
    printf("After setting lower nibble: %s = %d\n", reg2str(r), r.content);

    printf("Setting upper nibble to 10 (1010 binary)\n");
    assignNibble(10, 2, &r);
    printf("After setting upper nibble: %s = %d\n", reg2str(r), r.content);
    printf("--------------------------------\n");

    // Implementing shift operations
    r.content = initial_value;
    printf("Testing shift operations: ");
    printf("Initial r.content: %s = %d\n", reg2str(r), r.content);

    printf("Shifting left by 2 positions:\n");
    shiftLeft(2, &r);
    printf("After left shift: %s = %d\n", reg2str(r), r.content);

    printf("Shifting right by 1 position:\n");
    shiftRight(1, &r);
    printf("After right shift: %s = %d\n", reg2str(r), r.content);
    printf("--------------------------------\n");

    return 0;
}
