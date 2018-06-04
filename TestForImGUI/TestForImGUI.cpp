#include <windows.h>
#include "imgui.h"

int wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	ImGui::Begin("MyFirstImGUIWindow");
	ImGui::Text("HelloWorld!");
	ImGui::End();
	return 0;
}