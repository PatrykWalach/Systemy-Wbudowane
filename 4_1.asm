led DATA 029h
indicator6 DATA 030h
indicator5 DATA 031h
indicator4 DATA 032h
indicator3 DATA 033h
indicator2 DATA 034h
indicator1 DATA 035h
active DATA 036h

ORG 0000h
_RESET:
    LJMP _INIT
ORG 0100h
_INIT:
    LCALL _7SEG_INIT
_LOOP:
    LCALL _DELAY
    ;RR A
    ;MOV led, A
    LCALL _7SEG_REFRESH
    LJMP _LOOP
_7SEG_REFRESH:
    SETB P1.6

    MOV A, active
    RR A
    CJNE A, #80h, _ELSE
    RR A
_ELSE:
    MOV active, A
    MOV DPTR, #0FF30h
    MOVX @DPTR, A

    CJNE A, #01h, _ELSE1
    MOV A, indicator1
    JMP _END
_ELSE1:
    CJNE A, #02h, _ELSE2
    MOV A, indicator2
    JMP _END
_ELSE2:
    CJNE A, #04h, _ELSE3
    MOV A, indicator3
    JMP _END
_ELSE3:
    CJNE A, #08h, _ELSE4
    MOV A, indicator4
    JMP _END
_ELSE4:
    CJNE A, #10h, _ELSE5
    MOV A, indicator5
    JMP _END
_ELSE5:
    CJNE A, #20h, _ELSE6
    MOV A, indicator6
    JMP _END
_ELSE6:
    CJNE A, #40h, _END
    MOV A, led
    JMP _END2
_END:
    MOV DPTR, #_NUMBERS
    MOVC A, @A+DPTR
_END2:

    MOV DPTR, #0FF38H
    MOVX @DPTR, A

    CLR P1.6
    RET
_7SEG_INIT:
    MOV active, #40h
    MOV A, active
    MOV DPTR, #0FF30h
    MOVX @DPTR, A

    MOV led, #10h
    MOV indicator6, #5d
    MOV indicator5, #4d
    MOV indicator4, #3d
    MOV indicator3, #2d
    MOV indicator2, #1d
    MOV indicator1, #0d

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
_NUMBERS:
    DB 3Fh ;0
    DB 06h ;1
    DB 5Bh ;2
    DB 4Fh ;3
    DB 66h ;4
    DB 6Dh ;5
    DB 7Dh ;6
    DB 07h ;7
    DB 7Fh ;8
    DB 6Fh ;9
END
