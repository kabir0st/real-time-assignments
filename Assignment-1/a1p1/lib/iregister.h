//  Created by Mohammadreza Mousavi [mohmou] on 9/5/14.
//  Updated by Masoumeh Taromirad on 11/08/16.
//  Updated by Wagner Morais and Johannes van Esch on 28/08/18.
//  Copyright (c) 2014 by Mohammadreza Mousavi [mohmou]. All rights reserved.

#ifndef lab0_iregister_h
#define lab0_iregister_h

/**
 *  iRegister
 *  An iRegister is a structure which represents an 32-bit register and
 *  is equipped with standard operations to modify and display them.
 */
typedef struct{
    int content;
} iRegister;

/**
 *  Bellow you find the declarations for the functions to modify and display the
 *  memory content of a iRegister data structure. Before each declaration, a brief
 *  description about what the function shall do is given.
 *  Later in this file, the documentation for the resetBit function is given.
 *  Students should follow that format.
 */

/** @brief Resets all the bits of the iRegister to 0
 *
 *  @param r Pointer to iRegister
 *
 *  @return void
 *
 *  Pre-condition: iRegister != NULL
 *
 *  Post-condition: After resetAll(r), all bits of the iRegister are set to 0.
 *                  The content field becomes 0.
 *
 *  Properties:
 *  After resetAll(r), getBit(i, r) = 0 for all 0 <= i < 32
 *
 *  Test-cases:
 *  1. Allocate memory to an iRegister r
 *  2. Call resetAll(&r)
 *  3. Verify by printf("%s", reg2str(r)) - should display all zeros
 */
void resetAll(iRegister *);

/** @brief Sets the i'th bit of the iRegister to 1
 *
 *  @param i The bit position to set
 *
 *  @param r Pointer to iRegister
 *
 *  @return void
 *
 *  Pre-condition: 0 <= i < 32 and iRegister != NULL
 *
 *  Post-condition: After setBit(i, r), the i'th bit of iRegister is 1
 *
 *  Properties:
 *  After setBit(i, r), getBit(i, r) = 1
 *
 *  Test-cases:
 *  1. Allocate memory to an iRegister r
 *  2. Call resetAll(&r) to clear all bits
 *  3. Call setBit(5, &r) to set bit 5
 *  4. Verify by printf("%s", reg2str(r)) - bit 5 should be 1
 */
void setBit(int, iRegister *);


/** @brief Sets all the bits of the iRegister to 1
 *
 *  @param r Pointer to iRegister
 *
 *  @return void
 *
 *  Pre-condition: iRegister != NULL
 *
 *  Post-condition: After setAll(r), all bits of the iRegister are set to 1.
 *                  The content field becomes -1 (all bits set in two's complement).
 *
 *  Properties:
 *  After setAll(r), getBit(i, r) = 1 for all 0 <= i < 32
 *
 *  Test-cases:
 *  1. Allocate memory to an iRegister r
 *  2. Call setAll(&r)
 *  3. Verify by printf("%s", reg2str(r)) - should display all ones
 */
void setAll(iRegister *);


/** @brief Returns the i'th bit of the iRegister
 *
 *  @param i The bit position to retrieve
 *
 *  @param r Pointer to iRegister
 *
 *  @return int The value of the i'th bit (0 or 1)
 *
 *  Pre-condition: 0 <= i < 32 and iRegister != NULL
 *
 *  Properties:
 *  getBit(i, r) returns either 0 or 1
 *
 *  Test-cases:
 *  1. Allocate memory to an iRegister r
 *  2. Call setBit(3, &r) to set bit 3
 *  3. Verify getBit(3, &r) returns 1
 */
int getBit(int, iRegister *);


/** @brief Sets the first or the second nibble of iRegister to the given value
 *
 *  @param nibble value for the nibble
 *
 *  @param pos position to write the nibble
 *
 *  @param r Pointer to iRegister
 *
 *  @return void
 *
 *  Pre-condition: 0 <= nibble <= 15, pos == 1 or pos == 2, and iRegister != NULL
 *
 *  Post-condition: The specified nibble is set to the given value, other bits remain unchanged.
 *                  For pos=1: bits 0-3 are modified
 *                  For pos=2: bits 4-7 are modified
 *
 *  Properties:
 *  After assignNibble(nibble, pos, r), getNibble(pos, r) == nibble
 *  Bits outside the specified nibble remain unchanged
 *
 *  Test-cases:
 *  1. Allocate memory to an iRegister r, set to 0
 *  2. Call assignNibble(5, 1, &r) to set lower nibble to 5
 *  3. Verify getNibble(1, &r) returns 5
 *  4. Call assignNibble(10, 2, &r) to set upper nibble to 10
 *  5. Verify getNibble(2, &r) returns 10 and getNibble(1, &r) still returns 5
 */
void assignNibble(int, int, iRegister *);


/** @brief Gets the first or the second nibble of iRegister
 *
 *  @param pos position to read the nibble
 *
 *  @param r Pointer to iRegister
 *
 *  @return int value of the nibble
 *
 *  Pre-condition: pos == 1 or pos == 2, and iRegister != NULL
 *
 *  Post-condition: The iRegister remains unchanged. Returns the value of the nibble.
 *                  For pos=1: returns value of bits 0-3
 *                  For pos=2: returns value of bits 4-7
 *
 *  Properties:
 *  getNibble(pos, r) returns a value between 0 and 15
 *
 *  Test-cases:
 *  1. Allocate memory to an iRegister r
 *  2. Call assignNibble(7, 1, &r) to set lower nibble to 7
 *  3. Verify getNibble(1, &r) returns 7
 *  4. Call assignNibble(12, 2, &r) to set upper nibble to 12
 *  5. Verify getNibble(2, &r) returns 12
 */
int getNibble(int, iRegister *);


/** @brief Returns a pointer to an array of 32 characters representing the iRegister in binary
 *
 *  @param r Pointer to iRegister
 *
 *  @return char* A pointer to a static string containing 32 characters ('0' or '1')
 *                representing each bit of the iRegister. The string is null-terminated.
 *
 *  Pre-condition: iRegister != NULL
 *
 *  Post-condition: Returns a string representation of the binary of iRegister
 *
 *  Properties:
 *  The returned string has exactly 32 characters plus null terminator
 *
 *  Test-cases:
 *  1. Create iRegister r with content = 5
 *  2. Call reg2str(r)
 *  3. Verify result is "00000000000000000000000000000101"
 *  4. Create iRegister r with content = -1
 *  5. Verify result is "11111111111111111111111111111111"
 */
char *reg2str(iRegister);


/** @brief Shifts all the bits of the iRegister to the right by n places (logical shift)
 *
 *  @param n The number of positions to shift right (0-31)
 *
 *  @param r Pointer to iRegister
 *
 *  @return void
 *
 *  Pre-condition: 0 <= n <= 31 and iRegister != NULL
 *
 *  Post-condition: All bits are shifted right by n positions. Leftmost n bits become 0.
 *                  This is a logical shift that always fills with zeros from the left.
 *
 *  Properties:
 *  After shiftRight(n, r), the rightmost n bits are lost
 *  The leftmost n bits become 0
 *  For positive numbers: equivalent to integer division by 2^n
 *
 *  Test-cases:
 *  1. Create iRegister r with content = 8 (binary: 1000)
 *  2. Call shiftRight(1, &r)
 *  3. Verify content becomes 4 (binary: 0100)
 *  4. Create iRegister r with content = -8
 *  5. Call shiftRight(1, &r)
 *  6. Verify result is a large positive number (logical shift fills with 0s)
 */
void shiftRight(int, iRegister *);


/** @brief Shifts all the bits of the iRegister to the left by n places (appends 0 from the right) - performs LOGICAL shift (always fills with 0s)
 *
 *  @param n The number of positions to shift left (0-31)
 *
 *  @param r Pointer to iRegister
 *
 *  @return void
 *
 *  Pre-condition: 0 <= n <= 31 and iRegister != NULL
 *
 *  Post-condition: All bits are shifted left by n positions. Rightmost n bits become 0.
 *                  This is a logical shift that always fills with zeros from the right.
 *                  The leftmost n bits are lost (shifted out).
 *
 *  Properties:
 *  After shiftLeft(n, r), the leftmost n bits are lost
 *  The rightmost n bits become 0
 *  For positive numbers without overflow: equivalent to multiplication by 2^n
 *  For negative numbers without overflow: equivalent to division by 2^n
 *
 *  Test-cases:
 *  1. Create iRegister r with content = 4 (binary: 0100)
 *  2. Call shiftLeft(1, &r)
 *  3. Verify content becomes 8 (binary: 1000)
 *  4. Create iRegister r with content = 1073741824 (large positive)
 *  5. Call shiftLeft(1, &r)
 *  6. Verify overflow behavior (leftmost bit is lost)
 */
void shiftLeft(int, iRegister *);


/** @brief Resets the i'th bit of the iRegister (to 0)
 *
 *  @param i Is i'th bit of the iRegister to be reset
 *
 *  @param r Pointer to iRegister
 *
 *  @return void
 *
 *  Pre-condition: 0 <= i < 32 and iRegister != Null
 *
 *  Post-condition: after resetBit(i, r) the i'th bit of iRegister is 0, all other
 *  bits remain unchanged
 *  properties:
 *  after resetBit(i, r),  getBit(i, r) = 0
 *  if getBit(i, r) == 0 then getBit(j, r) returns the same value for all 0 <= j < 32 and j <> i before and after resetBit(i, r)
 *
 *  test-cases:
 *  1,2,3. Allocate memory to an iRegister r
 *  first do resetAll(&r),
 *  then set the i'th bit of &x by setBit(i, &r) for i = 0, 15 and 23 and then
 *  display the result after each and every call by
 *    printf("%s",reg2str(r))
 */
void resetBit(int, iRegister *);

#endif
