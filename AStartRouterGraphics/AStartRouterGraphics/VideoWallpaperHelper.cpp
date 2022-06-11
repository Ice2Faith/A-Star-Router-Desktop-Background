#include"VideoWallpaperHelper.h"
BOOL CALLBACK __EnumWindowProcFindDesktopWindow(HWND hTop, LPARAM lparam)
{
	HWND * hWorkerW = (HWND *)lparam;
	// 查找 SHELLDLL_DefView 窗体句柄
	// 存在多个WorkerW窗体，只有底下存在SHELLDLL_DefView的才是
	HWND hTemp = ::FindWindowEx(
		hTop, NULL, TEXT("SHELLDLL_DefView"), NULL);
	if (hTemp == NULL) {
		return TRUE;
	}

	// XP 直接查找SysListView32窗体
	// g_hWorker = ::FindWindowEx(hWndShl, NULL, _T("SysListView32"),_T("FolderView"));

	// win7或更高版本
	// 查找 WorkerW 窗口句柄(以桌面窗口为父窗口)
	*hWorkerW = ::FindWindowEx(NULL, hTop, TEXT("WorkerW"), NULL);
	return (*hWorkerW) == NULL;
}
HWND GetDesktopWallpaperWorkerHwnd(){
	// 背景窗体句柄
	HWND hProgman = NULL;
	HWND hWorkerW = NULL;
	// 遍历桌面所有顶层窗口去查找桌面背景窗口句柄
	if (hProgman == NULL) {
		// 先找到Progman 窗口
		hProgman = ::FindWindowEx(NULL, NULL, TEXT("Progman"), NULL);
		if (hProgman == NULL) {
			//找不到Progman窗体
			return NULL;
		}
		// 发送消息 0x052C 到Program Manager窗口，创建一个我们要的目标窗口
		// win10 1903 下未能成功（无法分离）
		DWORD_PTR lpdwResult = 0;
		SendMessageTimeout(hProgman, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, &lpdwResult);

		// 查找到 WorkerW 窗口，设置显示
		EnumWindows(__EnumWindowProcFindDesktopWindow, (LPARAM)&hWorkerW);
		//::ShowWindowAsync(hWorkerW, 0);
	}
	if (hProgman == NULL) {
		//找不到桌面壁纸层窗体
		return NULL;
	}
	return hWorkerW;
}
