#pragma once

#include "../Main/Main.h"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_dx9.h"

#define GUI_KEY_DOWN( KeyNum ) ( GetAsyncKeyState(KeyNum) & 0x1 )
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

class CGui
{
public:

	CGui();
	~CGui();

	void	GUI_Init( IDirect3DDevice9* pDevice );
	void	GUI_Begin_Render();
	void	GUI_End_Render();

	void	GUI_Draw_Elements();

	void	MenuColor();
	void    BlueSheme();
	void	MainFont();
	void	IconsFont();
};

extern bool bIsGuiVisible;
extern LRESULT ImGui_ImplDX9_WndProcHandler( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam );

namespace ImGui
{
	IMGUI_API bool ComboBoxArray( const char* label , int* currIndex , std::vector<std::string>& values );
	IMGUI_API bool TabLabels( const char **tabLabels , int tabSize , int &tabIndex , int *tabOrder );
}