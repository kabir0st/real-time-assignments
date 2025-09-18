#include <stdlib.h>
#include <stdio.h>
#include "lib/iregister.h"

int main(){

    iRegister r;
    int initial_value;
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

    printf("Initial value: %d\n", initial_value);
    // Test left shift function
    r.content = initial_value;


    assignNibble(7, 3, &r);  // Set lowest nibble to 15

    int nibble0 = getNibble(3, &r);
    printf("Nibble: %d\n", nibble0);
    convert_to_binary(nibble0);
    return 0;
}
