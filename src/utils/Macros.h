#include <cassert>
#include <Windows.h>
#include <comdef.h>

#ifdef _DEBUG
#define DX_CALL(line) {		               \
HRESULT hardwareResult = line;             \
if (FAILED(hardwareResult)) {			   \
OutputDebugStringA("-------HardwareResult error in "); \
OutputDebugStringA( __FILE__);	           \
OutputDebugStringA("\n-------Line: ");	   \
char line_number[32];					   \
sprintf_s(line_number, "%u", __LINE__);    \
OutputDebugStringA(line_number);	       \
OutputDebugStringA("\n-------");	       \
OutputDebugStringA(#line);	               \
OutputDebugStringA("\n-------");	       \
char hrString[32];					   \
sprintf_s(hrString, "%x", hardwareResult);    \
OutputDebugStringA(hrString);	       \
OutputDebugStringA("\n-------");	       \
_com_error err(hardwareResult);		       \
LPCTSTR errMsg = err.ErrorMessage();       \
OutputDebugStringA(errMsg);	               \
OutputDebugStringA("\n");                  \
__debugbreak();                            \
}                                          \
}                                          
#else
#define DX_CALL(line) line
#endif

#ifdef _DEBUG
#define DEBUG_LINE(line) line
#else
#define DEBUG_OP(line) 
#endif