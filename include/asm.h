#pragma once

/*
 * clang-cl currently has a bug with jmp [address] (see https://godbolt.org/z/aasYia)
 * it emits a relative jump of +[address] instead of an absolute jump to [address]
 *
 * to ensure consistent absolute jump behaviour on clang-cl, emit an absolute jump by running:
 * push X
 * ret
 *
 * which will in all cases jump to the absolute address at X
 *
 * this could slightly impact performance if the spot was critical, but it's not the case here
 *
 *
 * same for call: call address calls +address on clang-cl instead of an absolute call to address on msvc
 * circumvent by moving the address to eax, then calling eax
 */

#ifdef __clang__
#define JUMP(X) \
	__asm push X \
	__asm ret
#define CALL(X) \
	_asm mov eax, X \
	_asm call eax
#else
#define JUMP(X) \
	__asm jmp X
#define CALL(X) \
	__asm call X
#endif
