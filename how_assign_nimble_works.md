
 **setting value 9 at position 3 in a register with initial content 85**.

## Initial Values
- **value = 9** (decimal)
- **pos = 3** (nibble position)
- **r->content = 85** (decimal)

Let me show the binary representations:

## Step-by-Step Process

### 1. Initial State
```
r->content = 85 (decimal)
Binary (32-bit): 00000000 00000000 00000000 01010101
                 ^^^^^^^^ ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
Nibbles:           7        6        5        4    3    2    1    0
Bit positions:   31-28   27-24   23-20   19-16  15-12 11-8  7-4  3-0
```

### 2. Calculate Shift Amount
```c
int shift = 4 * pos;  // 4 * 3 = 12
```
Position 3 means we target bits 15-12 (nibble 3).

### 3. Create Mask
```c
int mask = 0xF << shift;  // 0xF << 12
```

**0xF** in binary: `1111` (4 bits)
**0xF << 12** shifts these 4 bits left by 12 positions:

```
Original 0xF:     00000000 00000000 00000000 00001111
After << 12:      00000000 00000000 11110000 00000000
                                    ^^^^
                              Target nibble (bits 15-12)
```

So **mask = 0x0000F000** (decimal: 61440)

### 4. Clear Target Nibble
```c
r->content &= ~mask;
```

**~mask** (bitwise NOT of mask):
```
mask:             00000000 00000000 11110000 00000000
~mask:            11111111 11111111 00001111 11111111
```

**r->content & ~mask**:
```
r->content:       00000000 00000000 00000000 01010101  (85)
~mask:            11111111 11111111 00001111 11111111
Result:           00000000 00000000 00000000 01010101  (85)
                                    ^^^^
                              Cleared nibble 3
```

In this case, nibble 3 was already 0000, so clearing it doesn't change the value.

### 5. Set New Nibble Value
```c
r->content |= (value << shift);  // value=9, shift=12
```

**value << shift** (9 << 12):
```
value = 9:        00000000 00000000 00000000 00001001
9 << 12:          00000000 00000000 10010000 00000000
                                    ^^^^
                              Value 9 in position 3
```

**Final OR operation**:
```
r->content:       00000000 00000000 00000000 01010101  (85)
(9 << 12):        00000000 00000000 10010000 00000000  (36864)
Result:           00000000 00000000 10010000 01010101  (36949)
```

### 6. Final Result
The register content becomes **36949** (decimal):
```
Binary: 00000000 00000000 10010000 01010101
                          ^^^^
                    Nibble 3 = 1001 (9 in binary)
```

### 7. Post-condition Verification
```c
if (((r->content >> shift) & 0xF) != value)
```

This extracts nibble 3 and verifies it equals 9:
- `r->content >> 12` shifts right by 12: `00000000 00000000 00000000 10010000`
- `& 0xF` masks to get only the lower 4 bits: `00000000 00000000 00000000 00001001` = 9 ✓

## Summary
The function successfully:
1. **Cleared** the target nibble (position 3, bits 15-12) using a mask
2. **Set** the new value (9) in that position using bitwise OR
3. **Verified** the operation was successful

The register content changed from **85** to **36949**, with nibble 3 now containing the value **9**.
