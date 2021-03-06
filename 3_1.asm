ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
_INIT:
    MOV 30h, #50
    MOV 31h, #50
    CLR C
    MOV A, 30h
    SUBB A, 31h

    JNZ _ELSE
    ACALL TEST_LED_ON
    JMP _END
_ELSE:
    ACALL BUZZER_ON
_END:
_LOOP:
    LJMP _LOOP
TEST_LED_ON:
    CLR P1.7
    RET
BUZZER_ON:
    CLR P1.5
    RET
END