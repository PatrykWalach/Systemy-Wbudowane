_RECV_FLG BIT 00h
_SEND_FLG BIT 01h
_RECV_BUF EQU 030h
_RECV_BUF_INDEKS DATA 030h
DS 8

ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
ORG 0023h
_INT_SERIAL:
    JB TI, _INT_SERIAL_TI
_INT_SERIAL_RI:
    MOV A, #_RECV_BUF
    INC _RECV_BUF_INDEKS
    ADD A, _RECV_BUF_INDEKS
    MOV R1, A
    MOV A, SBUF
    MOV SBUF, A 
    MOV @R1, A
    CLR RI
    SETB _RECV_FLG 
    RETI
_INT_SERIAL_TI:
    CLR TI
    ;SETB _SEND_FLG
    RETI
_INIT:
    SETB ES
    SETB EA
    CLR _RECV_FLG
    CLR _SEND_FLG
    MOV _RECV_BUF, #0d
    MOV _RECV_BUF_INDEKS, #0d
    MOV SCON, #01010000b
    ANL TMOD, #00101111b
    ORL TMOD, #00100000b
    MOV TL1, #0FDh
    MOV TH1, #0FDh
    ANL PCON, #01111111b
    CLR TF1
    SETB TR1
_LOOP:
    JNB _RECV_FLG, _LOOP10
    MOV A, _RECV_BUF_INDEKS 
    CLR CY
    SUBB A, #8d
    JC _LOOP5
    LCALL _CALCULATE
    ;MOV R7, #0d
    ;SETB _SEND_FLG
_LOOP5:
    CLR _RECV_FLG
_LOOP10:
    JNB _SEND_FLG, _LOOP
    CLR _SEND_FLG
    MOV A, R7
    INC R7
    MOVC A, @A+DPTR
    JZ _LOOP
    MOV SBUF, A
    LJMP _LOOP
_CALCULATE:
    CPL P1.7

    MOV A, #_RECV_BUF
    ADD A, #3d
    MOV R1, A
    MOV A, @R1
    SUBB A, #'0'

    MOV R6, A

    MOV A, #_RECV_BUF
    ADD A, #7d
    MOV R1, A
    MOV A, @R1
    SUBB A, #'0'

    ADD A, R6
    ADD A, #'0'
    MOV SBUF, A
    MOV _RECV_BUF_INDEKS, #0d
    RET
_MESSAGE:
    DB 'WITAJ', 013d, 010d, 0d
END
