#ifndef APIHOOK_HPP_INCLUDED
#define APIHOOK_HPP_INCLUDED

LPVOID HookAPICall(HMODULE targetModule, LPCSTR targetLibName, LPCSTR targetProcName, FARPROC hookProc);
LPVOID HookAPIExport(HMODULE targetModule, LPCSTR targetProcName, FARPROC hookProc);

#endif
