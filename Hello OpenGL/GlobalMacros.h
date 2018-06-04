#pragma once
#ifdef _DEBUG
#define SHOW_DEBUG_DIALOG(msg) MessageBoxA(nullptr,msg,"Hello OpenGL",MB_OK)
#else 
#define SHOW_DEBUG_DIALOG(msg)
#endif 