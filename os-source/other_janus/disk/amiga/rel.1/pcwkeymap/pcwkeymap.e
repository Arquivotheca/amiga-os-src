;
;   PCWKeymap.e - Spanish keyboard support
;
; $VER: PCWKeymap_e 1.0 06 Feb 1992
;
; This file contains entries of the following format:
;
;   0xaa   0xbbbb   0xcccc   0xdd   ; Comments
;
; Where aa bbbb cccc and dd are hex values with the following meainings:
;
;   aa   = Amiga Scan Code
;   bbbb = Amiga Keyboard State flags as defined below
;   cccc = PC Keyboard state flags as defined below
;   dd   = Scan code to send to PC.
;
;   NOSHIFT  0x0000
;   CONTROL  0x0001
;   CAPSLOCK 0x0002
;   SHIFT    0x0004
;   ALT      0x0010
;   NUMLOCK  0x0100
;
; The following are the only combinations of State flags allowed.
;   NOSHIFT
;   SHIFT
;   CAPSLOCK
;   CONTROL | ALT
;   CONTROL
;   NUMLOCK
;   SHIFT | CAPSLOCK
;   SHIFT | NUMLOCK
;   CAPSLOCK | NUMLOCK
;   SHIFT | CAPSLOCK | NUMLOCK
;
; Trying to edit the Amiga Scan Codes for the Control, Left Shift,
; Right Shift, Left Alt, Right Alt, Left Amiga and Right Amiga keys
; will have no affect.
;
; Example:
;
;   0x31   0x0000   0x0004   0x42
;
; The above entry reads, If a Non Shifted 0x31 Amiga Scan Code (Z key on
; U.S. keyboard) is recived change the pckeyboard state to shifted and
; send the Scan Code 0x42 to the PC.
;
;
; -----
;| ~   |
;| `   |
; -----
0x00 0x0000 0x0000 0x1A
0x00 0x0004 0x0030 0x7E
0x00 0x0002 0x0002 0x1A
0x00 0x0011 0x0011 0x1A
0x00 0x0001 0x0001 0x1A
0x00 0x0100 0x0100 0x1A
0x00 0x0006 0x0030 0x7E
0x00 0x0104 0x0030 0x7E
0x00 0x0102 0x0102 0x1A
0x00 0x0106 0x0030 0x7E
; ---
;| � |
;| 1 |
; ---
0x01 0x0004 0x0000 0x0D
0x01 0x0006 0x0002 0x0D
0x01 0x0104 0x0100 0x0D
0x01 0x0106 0x0102 0x0D
; ---
;| � |
;| 2 |
; ---
0x02 0x0004 0x0004 0x0D
0x02 0x0006 0x0006 0x0D
0x02 0x0104 0x0104 0x0D
0x02 0x0106 0x0106 0x0D
; ---
;| # |
;| 3 |
; ---
0x03 0x0004 0x0011 0x04
0x03 0x0006 0x0011 0x04
0x03 0x0104 0x0011 0x04
0x03 0x0106 0x0011 0x04
; ---
;| 4 |
;|---|
;| � |
; ---
0x04 0x0011 0x0030 0xF8
; ---
;| / |
;| 6 |
;|---|
;| ^ |
; ---
0x06 0x0004 0x0004 0x08
0x06 0x0006 0x0006 0x08
0x06 0x0104 0x0104 0x08
0x06 0x0106 0x0106 0x08
0x06 0x0011 0x0004 0x1A
; ---
;| & |
;| 7 |
; ---
0x07 0x0004 0x0004 0x07
0x07 0x0006 0x0006 0x07
0x07 0x0104 0x0104 0x07
0x07 0x0106 0x0106 0x07
; ---
;| * |
;| 8 |
; ---
0x08 0x0004 0x0004 0x1B
0x08 0x0006 0x0006 0x1B
0x08 0x0104 0x0104 0x1B
0x08 0x0106 0x0106 0x1B
; ---
;| ( |
;| 9 |
; ---
0x09 0x0004 0x0004 0x09
0x09 0x0006 0x0006 0x09
0x09 0x0104 0x0104 0x09
0x09 0x0106 0x0106 0x09
; ---
;| ) |
;| 0 |
; ---
0x0A 0x0004 0x0004 0x0A
0x0A 0x0006 0x0006 0x0A
0x0A 0x0104 0x0104 0x0A
0x0A 0x0106 0x0106 0x0A
; ---
;| _ |
;| - |
; ---
0x0B 0x0000 0x0000 0x35
0x0B 0x0004 0x0004 0x35
0x0B 0x0002 0x0002 0x35
0x0B 0x0011 0x0011 0x35
0x0B 0x0001 0x0001 0x35
0x0B 0x0100 0x0100 0x35
0x0B 0x0006 0x0006 0x35
0x0B 0x0104 0x0104 0x35
0x0B 0x0102 0x0102 0x35
0x0B 0x0106 0x0106 0x35
; ---
;| + |
;| = |
; ---
0x0C 0x0000 0x0004 0x0B
0x0C 0x0004 0x0000 0x1B
0x0C 0x0002 0x0006 0x0B
0x0C 0x0011 0x0000 0xFF
0x0C 0x0001 0x0000 0xFF
0x0C 0x0100 0x0104 0x0B
0x0C 0x0006 0x0002 0x1B
0x0C 0x0104 0x0100 0x1B
0x0C 0x0102 0x0106 0x0B
0x0C 0x0106 0x0102 0x1B
; ---
;| | |
;| \ |
; ---
0x0D 0x0000 0x0011 0x29
0x0D 0x0004 0x0011 0x02
0x0D 0x0002 0x0011 0x29
0x0D 0x0011 0x0000 0xFF
0x0D 0x0001 0x0000 0xFF
0x0D 0x0100 0x0011 0x29
0x0D 0x0006 0x0011 0x02
0x0D 0x0104 0x0011 0x02
0x0D 0x0102 0x0011 0x29
0x0D 0x0106 0x0011 0x02
; ---
;| � |
;| � |
; ---
0x1A 0x0000 0x0000 0x28
0x1A 0x0004 0x0004 0x28
0x1A 0x0002 0x0002 0x28
0x1A 0x0011 0x0011 0x28
0x1A 0x0001 0x0001 0x28
0x1A 0x0100 0x0100 0x28
0x1A 0x0006 0x0006 0x28
0x1A 0x0104 0x0104 0x28
0x1A 0x0102 0x0102 0x28
0x1A 0x0106 0x0106 0x28
; ---
;| ^ |
;| ` |
; ---
0x1B 0x0000 0x0000 0x1A
0x1B 0x0004 0x0004 0x1A
0x1B 0x0002 0x0002 0x1A
0x1B 0x0011 0x0011 0x1A
0x1B 0x0001 0x0001 0x1A
0x1B 0x0100 0x0100 0x1A
0x1B 0x0006 0x0006 0x1A
0x1B 0x0104 0x0104 0x1A
0x1B 0x0102 0x0102 0x1A
0x1B 0x0106 0x0106 0x1A
; ---
;| : |
;| ; |
; ---
0x2A 0x0000 0x0004 0x33
0x2A 0x0004 0x0004 0x34
0x2A 0x0002 0x0006 0x33
0x2A 0x0011 0x0000 0xFF
0x2A 0x0001 0x0000 0xFF
0x2A 0x0100 0x0104 0x33
0x2A 0x0006 0x0006 0x34
0x2A 0x0104 0x0104 0x34
0x2A 0x0102 0x0106 0x33
0x2A 0x0106 0x0106 0x34
; ---
;| ? |
;| , |
; ---
0x38 0x0004 0x0004 0x0C
0x38 0x0006 0x0006 0x0C
0x38 0x0104 0x0104 0x0C
0x38 0x0106 0x0106 0x0C
; ---
;| ! |
;| . |
; ---
0x39 0x0004 0x0004 0x02
0x39 0x0006 0x0006 0x02
0x39 0x0104 0x0104 0x02
0x39 0x0106 0x0106 0x02
; ---
;| " |
;| ' |
; ---
0x3A 0x0000 0x0000 0x0C
0x3A 0x0004 0x0004 0x03
0x3A 0x0002 0x0002 0x0C
0x3A 0x0011 0x0011 0x0C
0x3A 0x0001 0x0001 0x0C
0x3A 0x0100 0x0100 0x0C
0x3A 0x0006 0x0006 0x03
0x3A 0x0104 0x0104 0x03
0x3A 0x0102 0x0102 0x0C
0x3A 0x0106 0x0106 0x03
; ---
;| { |
;| [ |
; ---
0x5A 0x0000 0x0011 0x1A
0x5A 0x0004 0x0011 0x28
0x5A 0x0002 0x0011 0x1A
0x5A 0x0011 0x0000 0xFF
0x5A 0x0001 0x0000 0xFF
0x5A 0x0100 0x0011 0x1A
0x5A 0x0006 0x0011 0x28
0x5A 0x0104 0x0011 0x28
0x5A 0x0102 0x0011 0x1A
0x5A 0x0106 0x0011 0x28
; ---
;| } |
;| ] |
; ---
0x5B 0x0000 0x0011 0x1B
0x5B 0x0004 0x0011 0x2B
0x5B 0x0002 0x0011 0x1B
0x5B 0x0011 0x0000 0xFF
0x5B 0x0001 0x0000 0xFF
0x5B 0x0100 0x0011 0x1B
0x5B 0x0006 0x0011 0x2B
0x5B 0x0104 0x0011 0x2B
0x5B 0x0102 0x0011 0x1B
0x5B 0x0106 0x0011 0x2B
; ---
;| / |
;|   |
; ---
0x5C 0x0000 0x0004 0x08
0x5C 0x0004 0x0004 0x08
0x5C 0x0002 0x0006 0x08
0x5C 0x0011 0x0000 0xFF
0x5C 0x0001 0x0000 0xFF
0x5C 0x0100 0x0104 0x08
0x5C 0x0006 0x0006 0x08
0x5C 0x0104 0x0104 0x08
0x5C 0x0102 0x0106 0x08
0x5C 0x0106 0x0106 0x08
; ---
;| * |
;|   |
; ---
0x5D 0x0000 0x0000 0x37
0x5D 0x0004 0x0004 0x37
0x5D 0x0002 0x0002 0x37
0x5D 0x0011 0x0011 0x37
0x5D 0x0001 0x0001 0x37
0x5D 0x0100 0x0100 0x37
0x5D 0x0006 0x0006 0x37
0x5D 0x0104 0x0104 0x37
0x5D 0x0102 0x0102 0x37
0x5D 0x0106 0x0106 0x37