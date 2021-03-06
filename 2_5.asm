ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
_INIT:
    CLR C
    MOV 30h, #6
    MOV 31h, #50
_DO:
    INC 30h
    CPL P1.7
_WHILE:
    MOV A, 30h
    CJNE A, 31h, _DO
_FI:
_LOOP:
    LJMP _LOOP
END

 