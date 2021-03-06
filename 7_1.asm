_I DATA 030h

ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
_INIT:
    LCALL _LCD_INIT

    MOV _I, #0
_FOR:
    MOV A, _I
    CJNE A, #6, _DO
    JMP _BREAK
_DO:
    MOV DPTR, #_STRING
    MOVC A, @A+DPTR
    MOV R7, A
    LCALL _LCD_DATA_FROM_R7
    INC _I
    JMP _FOR
_BREAK:

    MOV R7, #11000000b
    LCALL _LCD_CMD_FROM_R7

    MOV _I, #6
_FOR3:
    MOV A, _I
    CJNE A, #12, _DO3
    JMP _BREAK3
_DO3:
    MOV DPTR, #_STRING
    MOVC A, @A+DPTR
    MOV R7, A
    LCALL _LCD_DATA_FROM_R7
    INC _I
    JMP _FOR3
_BREAK3:
_LOOP:
    LJMP _LOOP
_LCD_WAIT_WHILE_BUSY:
    MOV DPTR, #0FF82h
    MOVX A, @DPTR
    ANL A, #80h
    CJNE A, #0, _LCD_WAIT_WHILE_BUSY
    RET
_LCD_CMD_FROM_R7:
    LCALL _LCD_WAIT_WHILE_BUSY
    MOV DPTR, #0FF80h
    MOV A, R7
    MOVX @DPTR, A
    RET
_LCD_DATA_FROM_R7:
    LCALL _LCD_WAIT_WHILE_BUSY
    MOV DPTR, #0FF81h
    MOV A, R7
    MOVX @DPTR, A
    RET
_LCD_INIT:

    MOV R7, #111000b
    LCALL _LCD_CMD_FROM_R7

    MOV R7, #1111b
    LCALL _LCD_CMD_FROM_R7

    MOV R7, #110b
    LCALL _LCD_CMD_FROM_R7

    MOV R7, #1b
    LCALL _LCD_CMD_FROM_R7

    RET
_STRING:
    ;DB 20h, 44h, 53h, 4Dh, 2Dh, 35h, 31h, 20h
    ;DB 46h, 4Fh, 52h, 45h, 56h, 45h, 52h, 00h
    DB 50h, 41h, 54h, 52h, 59h, 4Bh
    DB 57h, 41h, 4Ch, 41h, 43h, 48h
END
