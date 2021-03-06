counter DATA 033h
ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
_INIT:
    LCALL _7SEG_INIT
_LOOP:
    LCALL _DELAY
    RR A
    MOV counter, A
    LCALL _7SEG_REFRESH
    LJMP _LOOP
_7SEG_REFRESH:
    SETB P1.6
    MOV A, counter
    MOV DPTR, #0FF38H
    MOVX @DPTR, A
    CLR P1.6
    RET
_7SEG_INIT:
    MOV A, #40h
    MOV DPTR, #0FF30h
    MOVX @DPTR, A
    MOV counter, #10h
    LCALL _7SEG_REFRESH
    RET
_DELAY:
    MOV R6, #250d
LABEL1:
    MOV R7, #250d
LABEL2:
    DJNZ R7,LABEL2
    DJNZ R6,LABEL1
    RET
_7SEG_SET_F1_ON:
    MOV A, counter
    ORL A, #01h
    MOV counter, A
    RET
_7SEG_SET_F1_OFF:
    MOV A, counter
    ANL A, #0FEh
    MOV counter, A
    RET
_7SEG_SET_F2_ON:
    MOV A, counter
    ORL A, #02h
    MOV counter, A
    RET
_7SEG_SET_F2_OFF:
    MOV A, counter
    ANL A, #0FDh
    MOV counter, A
    RET
_7SEG_SET_F3_ON:
    MOV A, counter
    ORL A, #04h
    MOV counter, A
    RET
_7SEG_SET_F3_OFF:
    MOV A, counter
    ANL A, #0FBh
    MOV counter, A
    RET
_7SEG_SET_F4_ON:
    MOV A, counter
    ORL A, #08h
    MOV counter, A
    RET
_7SEG_SET_F4_OFF:
    MOV A, counter
    ANL A, #0F7h
    MOV counter, A
    RET
_7SEG_SET_OK_ON:
    MOV A, counter
    ORL A, #10h
    MOV counter, A
    RET
_7SEG_SET_OK_OFF:
    MOV A, counter
    ANL A, #0EFh
    MOV counter, A
    RET
_7SEG_SET_ER_ON:
    MOV A, counter
    ORL A, #20h
    MOV counter, A
    RET
_7SEG_SET_ER_OFF:
    MOV A, counter
    ANL A, #0DFh
    MOV counter, A
    RET
END
