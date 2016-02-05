.586
.model flat,stdcall
.code

@SwitchUST@8 proc SYSCALL
	push ebp
	push ebx
	push esi
	push edi

	mov dword ptr [ecx], esp
	mov esp, dword ptr [edx]

	pop edi
	pop esi
	pop ebx
	pop ebp
	ret
@SwitchUST@8 endp

end