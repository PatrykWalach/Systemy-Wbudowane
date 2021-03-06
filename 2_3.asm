ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
_INIT:
    CLR C
    MOV 30h, #50
    MOV 31h, #49
    MOV A, 30h
    SUBB A, 31h
    JNC _ELSE
_IF:
    ;30h < 31h
    CLR P1.7
    CLR P1.5
    JMP _FI
_ELSE:
    JZ _DEFAULT
    ;30h > 31h
    CLR P1.7
    JMP _FI
_DEFAULT:
    ;30h == 31h
    CLR P1.5
_FI:
_LOOP:
    LJMP _LOOP
END

 