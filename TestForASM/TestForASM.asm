.386	
.MODEL	FLAT,STDCALL
INCLUDELIB "kernel32.lib"
INCLUDELIB "msvcrt.lib"
INCLUDELIB "MSVCRTD.lib"
.DATA
	string DB 'Hello World',0
	hellotitle DB 'My First ASM Program',0
	arrayindex DB 0,':',0
	array DD 0,1,2,3,4,5,6,7,8,9
.CODE	

ExitProcess PROTO,
	dwExitCode:DWORD	

MessageBoxA PROTO,pHwnd:DWORD,pStr:DWORD,pTitle:DWORD,paramType:DWORD	
;printf PROTO C,:PTR,:VARARG


_output PROC
	MOV DX,10
	MOV AL,'0'
	loopb:
		MOV EBX,OFFSET arrayindex
		MOV [EBX],AL
		INVOKE MessageBoxA,0,EBX,OFFSET string,0
		INC AL
		DEC DX
		MOV CX,DX
	LOOP loopb
	RET
_output ENDP	

_main PROC
	MOV CX,100
	INVOKE	 _output	
_main ENDP	
END	 _main