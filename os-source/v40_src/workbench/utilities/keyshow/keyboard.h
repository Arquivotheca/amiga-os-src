
#define KEYHEIGHT 10
#define KSP 2

#define K1 24
#define K2 30
#define K3 37
#define K4 45
#define K5 50
#define K6 232
#define K7 56
#define K8 32
#define K9 K3+KSP+K1
#define KB 69
#define KC 32

#define ADDROW KEYHEIGHT+2
#define INDENT 20

#define R1X INDENT
#define R1Y 25
#define R2X INDENT
#define R2Y R1Y+ADDROW
#define R3X INDENT
#define R3Y R2Y+ADDROW
#define R4X INDENT
#define R4Y R3Y+ADDROW
#define R5X INDENT
#define R5Y R4Y+ADDROW
#define R6X INDENT+K1-10
#define R6Y R5Y+ADDROW
#define PADX 20*(K1+2)
#define PADX2 16*(K1+2)+12
#define K00X R2X
#define K00Y R2Y

#define K01X KSP+K00X+K2
#define K01Y R2Y

#define K02X KSP+K01X+K1
#define K02Y R2Y

#define K03X KSP+K02X+K1
#define K03Y R2Y

#define K04X KSP+K03X+K1
#define K04Y R2Y

#define K05X KSP+K04X+K1
#define K05Y R2Y

#define K06X KSP+K05X+K1
#define K06Y R2Y

#define K07X KSP+K06X+K1
#define K07Y R2Y

#define K08X KSP+K07X+K1
#define K08Y R2Y

#define K09X KSP+K08X+K1
#define K09Y R2Y

#define K0AX KSP+K09X+K1
#define K0AY R2Y

#define K0BX KSP+K0AX+K1
#define K0BY R2Y

#define K0CX KSP+K0BX+K1
#define K0CY R2Y

#define K0DX KSP+K0CX+K1
#define K0DY R2Y

#define K0EX NULL
#define K0EY NULL

#define K0FX PADX
#define K0FY R6Y

#define K10X KSP+K42X+K4
#define K10Y R3Y

#define K11X KSP+K10X+K1
#define K11Y R3Y

#define K12X KSP+K11X+K1
#define K12Y R3Y

#define K13X KSP+K12X+K1
#define K13Y R3Y

#define K14X KSP+K13X+K1
#define K14Y R3Y

#define K15X KSP+K14X+K1
#define K15Y R3Y

#define K16X KSP+K15X+K1
#define K16Y R3Y

#define K17X KSP+K16X+K1
#define K17Y R3Y

#define K18X KSP+K17X+K1
#define K18Y R3Y

#define K19X KSP+K18X+K1
#define K19Y R3Y

#define K1AX KSP+K19X+K1
#define K1AY R3Y

#define K1BX KSP+K1AX+K1
#define K1BY R3Y

#define K1CX NULL
#define K1CY NULL

#define K1DX PADX
#define K1DY R5Y

#define K1EX KSP+K1DX+K1
#define K1EY R5Y

#define K1FX KSP+K1EX+K1
#define K1FY R5Y

#define K20X KSP+K62X+K1
#define K20Y R4Y

#define K21X KSP+K20X+K1
#define K21Y R4Y

#define K22X KSP+K21X+K1
#define K22Y R4Y

#define K23X KSP+K22X+K1
#define K23Y R4Y

#define K24X KSP+K23X+K1
#define K24Y R4Y

#define K25X KSP+K24X+K1
#define K25Y R4Y

#define K26X KSP+K25X+K1
#define K26Y R4Y

#define K27X KSP+K26X+K1
#define K27Y R4Y

#define K28X KSP+K27X+K1
#define K28Y R4Y

#define K29X KSP+K28X+K1
#define K29Y R4Y

#define K2AX KSP+K29X+K1
#define K2AY R4Y

#define K2BX KSP+K2AX+K1
#define K2BY R4Y

#define K2CX NULL
#define K2CY NULL

#define K2DX PADX
#define K2DY R4Y

#define K2EX KSP+K2DX+K1
#define K2EY R4Y

#define K2FX KSP+K2EX+K1
#define K2FY R4Y

#define K30X KSP+K60X+K3
#define K30Y R5Y

#define K31X KSP+K30X+K1
#define K31Y R5Y

#define K32X KSP+K31X+K1
#define K32Y R5Y

#define K33X KSP+K32X+K1
#define K33Y R5Y

#define K34X KSP+K33X+K1
#define K34Y R5Y

#define K35X KSP+K34X+K1
#define K35Y R5Y

#define K36X KSP+K35X+K1
#define K36Y R5Y

#define K37X KSP+K36X+K1
#define K37Y R5Y

#define K38X KSP+K37X+K1
#define K38Y R5Y

#define K39X KSP+K38X+K1
#define K39Y R5Y

#define K3AX KSP+K39X+K1
#define K3AY R5Y

#define K3BX NULL
#define K3BY NULL

#define K3CX KSP+K0FX+K5
#define K3CY R6Y

#define K3DX PADX
#define K3DY R3Y

#define K3EX KSP+K3DX+K1
#define K3EY R3Y

#define K3FX KSP+K3EX+K1
#define K3FY R3Y

#define K40X KSP+K66X+K2
#define K40Y R6Y

#define K41X KSP+K0DX+K1
#define K41Y R2Y

#define K42X R3X
#define K42Y R3Y

#define K43X KSP+K1FX+K1
#define K43Y R5Y

#define K44X KSP+K1BX+K1
#define K44Y R3Y

#define K45X R1X
#define K45Y R1Y

#define K46X PADX2
#define K46Y R2Y

#define K47X NULL
#define K47Y NULL

#define K48X NULL
#define K48Y NULL

#define K49X NULL
#define K49Y NULL

#define K4AX KSP+K3FX+K1
#define K4AY R3Y

#define K4BX NULL
#define K4BY NULL

#define K4CX PADX2+KSP+K1
#define K4CY R4Y

#define K4DX KSP+K4FX+K1
#define K4DY R5Y

#define K4EX KSP+K4DX+K1
#define K4EY R5Y

#define K4FX PADX2
#define K4FY R5Y

#define K50X K01X
#define K50Y R1Y

#define K51X KSP+K50X+K2
#define K51Y R1Y

#define K52X KSP+K51X+K2
#define K52Y R1Y

#define K53X KSP+K52X+K2
#define K53Y R1Y

#define K54X KSP+K53X+K2
#define K54Y R1Y

#define K55X K08X-(2*KSP)
#define K55Y R1Y

#define K56X KSP+K55X+K2
#define K56Y R1Y

#define K57X KSP+K56X+K2
#define K57Y R1Y

#define K58X KSP+K57X+K2
#define K58Y R1Y

#define K59X KSP+K58X+K2
#define K59Y R1Y

#define K5AX PADX
#define K5AY R2Y

#define K5BX KSP+K5AX+K1
#define K5BY R2Y

#define K5CX KSP+K5BX+K1
#define K5CY R2Y

#define K5DX KSP+K5CX+K1
#define K5DY R2Y

#define K5EX KSP+K2FX+K1
#define K5EY R4Y

#define K5FX KSP+K46X+K3
#define K5FY R2Y

#define K60X R5X
#define K60Y R5Y

#define K61X KSP+K3AX+K1
#define K61Y R5Y

#define K62X KSP+K63X+K1
#define K62Y R4Y

#define K63X R4X
#define K63Y R4Y

#define K64X R6X
#define K64Y R6Y

#define K65X KSP+K67X+K2
#define K65Y R6Y

#define K66X KSP+K64X+KC
#define K66Y R6Y

#define K67X KSP+K40X+K6
#define K67Y R6Y


