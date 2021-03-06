ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
_INIT:
    CLR C
    MOV 30h, #6
    MOV 31h, #50
    MOV A, 30h
_WHILE:
    CJNE A, 31h, _ELSE
    JMP _FI
_ELSE:
    INC 30h
    CPL P1.7
    MOV A, 30h
    JMP _WHILE
_FI:
_LOOP:
    LJMP _LOOP
END

 