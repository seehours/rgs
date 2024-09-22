#pragma once 
#include <windows.h>

namespace RGS {

	class ImGuiWindow 
	{
	private:
		ImGuiWindow();
		~ImGuiWindow();

		void Init() ;
		void Terminate();


	public:
		void Begin();
		void End();
		static ImGuiWindow& Instance();

	private:
		HWND m_Handle;
		WNDCLASSEXW m_WindowClass;
	};
}