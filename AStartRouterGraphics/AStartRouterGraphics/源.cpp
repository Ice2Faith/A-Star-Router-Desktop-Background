#include<graphics.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<Windows.h>
#include<conio.h>
#include<vector>
#include <algorithm>
#include"VideoWallpaperHelper.h"

using namespace std;

// 定义地图方块类型
// 空块
#define BLK_EMPTY 0
// 墙体
#define BLK_BLOCK -1

// 是否显示块权重文字
bool isShowText = false;

// 寻路步骤
typedef struct _step{
	int x;
	int y;
	struct _step * pre;
	double dis;
} Step;

// 地图
typedef struct _map{
	char * map;
	int wid;
	int hei;
	// 开始结束点
	int beginX;
	int beginY;
	int endX;
	int endY;
} Map;

// 距离计算
double distance(int x1, int y1, int x2, int y2){
	return sqrt(pow(x2 - x1, 2.0)+ pow(y2 - y1, 2.0));
}

//创建地图
Map createMap(int wid, int hei){
	char * map = (char*)malloc(sizeof(char)*(wid*hei));
	Map ret;
	ret.wid = wid;
	ret.hei = hei;
	ret.map = map;
	return ret;
}

// 释放地图空间
void releaseMap(Map& map){
	if (map.map != NULL){
		free(map.map);
		map.map = NULL;
	}
	map.wid = 0;
	map.hei = 0;
}

// 线性地图的二维转换
char* mapAt(Map& map, int x, int y)
{
	return &(map.map[y*map.wid + x]);
}

// 初始化地图
void initMap(Map& map)
{
	// 墙体概率
	int rate = rand() % 30+10;
	// 生成地图
	for (int y = 0; y < map.hei; y++){
		for (int x = 0; x < map.wid; x++){
			*mapAt(map, x, y) = BLK_EMPTY;
			if (rand() % 100 < rate){
				*mapAt(map, x, y) = BLK_BLOCK;
			}
		}
	}
	// 生成开始结束点
	int wd = max(map.wid, map.hei);
	bool startOk = false;
	bool endOk = false;
	while (!startOk || !endOk){
		int x = rand() % map.wid;
		int y = rand() % map.hei;
		if (*mapAt(map, x, y) == BLK_EMPTY){
			if (!startOk){
				map.beginX = x;
				map.beginY = y;
				startOk = true;
				continue;
			}
			if (!endOk){
				map.endX = x;
				map.endY = y;
				if (distance(map.beginX, map.beginY, map.endX, map.endY) > wd / 3.5){
					endOk = true;
				}
			}
		}
	}
}

// 绘制开始结束点
void drawSEP(Map& map, HDC hdc, int hdcWid, int hdcHei){
	int blkWid = hdcWid / map.wid;
	int blkHei = hdcHei / map.hei;
	
	if (map.beginX>=0){
		HGDIOBJ oldBrush = NULL;
		HBRUSH brush = CreateSolidBrush(RGB(0, 255, 0));
		oldBrush = SelectObject(hdc, brush);
		Rectangle(hdc, map.beginX*blkWid, map.beginY*blkHei, (map.beginX + 1)*blkWid, (map.beginY + 1)*blkHei);
		HGDIOBJ obj = SelectObject(hdc, oldBrush);
		DeleteObject(obj);
		DeleteObject(brush);
	}
	if (map.endX>=0){
		HGDIOBJ oldBrush = NULL;
		HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
		oldBrush = SelectObject(hdc, brush);
		Rectangle(hdc, map.endX*blkWid, map.endY*blkHei, (map.endX + 1)*blkWid, (map.endY + 1)*blkHei);
		HGDIOBJ obj = SelectObject(hdc, oldBrush);
		DeleteObject(obj);
		DeleteObject(brush);
	}
}

COLORREF blockColor = 0x0;
// 绘制地图
void drawMap(Map& map, HDC hdc, int hdcWid, int hdcHei)
{
	int blkWid = hdcWid / map.wid;
	int blkHei = hdcHei / map.hei;
	for (int y = 0; y < map.hei; y++){
		for (int x = 0; x < map.wid; x++){
			char ch = *mapAt(map, x, y);
			HGDIOBJ oldBrush = NULL;
			HBRUSH brush = NULL;
			
			if (ch == BLK_EMPTY){
				brush = CreateSolidBrush(RGB(255, 255, 255));
				oldBrush = SelectObject(hdc, brush);
			}
			else if (ch == BLK_BLOCK){
				brush = CreateSolidBrush(blockColor);
				oldBrush = SelectObject(hdc, brush);
			}
			else if (ch > BLK_EMPTY){
				char pch = ch - 1;
				brush = CreateSolidBrush(RGB((220 - min(150, pch * 20)), (255 - min(200, pch * 10)), 0));
				oldBrush = SelectObject(hdc, brush);
			}
			
			
			Rectangle(hdc, x*blkWid, y*blkHei, (x + 1)*blkWid, (y + 1)*blkHei);
			HGDIOBJ brushObj = SelectObject(hdc, oldBrush);
			DeleteObject(brushObj);
			DeleteObject(brush);

			// 绘制地图文字
			if (isShowText && ch>0){
				COLORREF textColor = 0xeeddaa;
				if (ch < 8){
					textColor = 0x443366;
				}
				SetTextColor(hdc,textColor);
				SetBkMode(hdc, TRANSPARENT);

				TCHAR buf[20];
				_stprintf(buf, TEXT("%d"), (int)ch);
				TextOut(hdc, x*blkWid, y*blkHei, buf,_tcslen(buf));
			}
			
		}
	}
	drawSEP(map, hdc, hdcWid, hdcHei);
}

// 排序比较器
bool comStepDistance(Step * step1, Step * step2){
	return step1->dis < step2->dis;
}

// 绘制路径
void drawPath(Step * cur,COLORREF color,HDC hdc,double blkWid,double blkHei,double rate=1.0){
	Step* rp = cur;
	Step* last = NULL;
	double drate = 1.0 - rate;
	while (rp != NULL){
		HGDIOBJ oldBrush = NULL;
		HGDIOBJ oldPen = NULL;

		HBRUSH brush = CreateSolidBrush(color);
		oldBrush = SelectObject(hdc, brush);

		HPEN pen = CreatePen(PS_SOLID, 1, 0xffffff);
		oldPen = SelectObject(hdc, pen);

		Rectangle(hdc, (rp->x+drate/2)*blkWid, (rp->y+drate/2)*blkHei, (rp->x + rate+drate/2)*blkWid, (rp->y + rate+drate/2)*blkHei);
		HGDIOBJ brushObj = SelectObject(hdc, oldBrush); 
		DeleteObject(brushObj);
		DeleteObject(brush);

		HGDIOBJ penObj = SelectObject(hdc, oldPen);
		DeleteObject(penObj);
		DeleteObject(pen);

		if (last != NULL){
			HPEN pen = CreatePen(PS_SOLID, 1, 0x0);
			HGDIOBJ oldPen = SelectObject(hdc, pen);
			MoveToEx(hdc, (rp->x+0.5)*blkWid, (rp->y+0.5)*blkHei, NULL);
			LineTo(hdc, (last->x+0.5)*blkWid, (last->y+0.5)*blkHei);
			HGDIOBJ delPen = SelectObject(hdc, oldPen);
			DeleteObject(delPen);
			DeleteObject(pen);
		}
		       
		last = rp;
		rp = rp->pre;
	}
}



// 是否按键按下
bool isKeyDownEx(int keyCode){
	return GetAsyncKeyState(keyCode) & 0x08000;
}

// 程序内检测按键按下，必须同时按下CAPSLock键
bool isKeyDown(int keyCode){
	return isKeyDownEx(VK_CAPITAL) && isKeyDownEx(keyCode);
}

int boostCount = 0;
// 是否加速模式
bool isBoostMode(){
	return isKeyDown(VK_RETURN);
}

// 在地图中手动设置方块或移除方块
void exchangeBlocks(Map& map, int hdcWid, int hdcHei){
	POINT p;
	GetCursorPos(&p);

	int px = p.x / (hdcWid / map.wid);
	int py = p.y / (hdcHei / map.hei);

	if (px < 0 || py < 0 || px >= map.wid || py >= map.hei){
		return;
	}

	if (px == map.beginX && py == map.beginY){
		return;
	}

	if (px == map.endX && py == map.endY){
		return;
	}

	if (*mapAt(map, px, py)>0){
		return;
	}

	if (isKeyDown('E') || isKeyDown('e')){
		*mapAt(map, px, py) = BLK_EMPTY;
	}
	if (isKeyDown('R') || isKeyDown('r')){
		*mapAt(map, px, py) = BLK_BLOCK;
	}

}

int main(int argc, char * argv[]){
	// 初始化随机数
	srand((unsigned int)time(NULL));

	int blkWid = 20;
	int blkHei = 20;

	int winWid = 1080;
	int winHei = 720;

	
	HWND hwnd = initgraph(winWid, winHei);

	// 获取桌面背景窗口为父窗口
	HWND wallHwnd = GetDesktopWallpaperWorkerHwnd();
	// 第一次设置父窗口，在EasyX中，多次initgraph返回的hwnd不变
	// 因此，第一次设为为父窗口后，通过获取尺寸函数，就能获得父窗口模式下的正确窗口尺寸
	if (wallHwnd != NULL){
		SetParent(hwnd, wallHwnd);
	}
	
	
	// 获取全屏尺寸
	winWid = GetSystemMetrics(SM_CXSCREEN)+2;
	winHei = GetSystemMetrics(SM_CYSCREEN)+2;
	hwnd=initgraph(winWid, winHei);
	
	if (wallHwnd != NULL){
		SetParent(hwnd, wallHwnd);
	}
	
	//消除边框
	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) - WS_CAPTION);
	ShowWindow(hwnd, SW_MAXIMIZE);

	// 固定到屏幕左上角对其
	SetWindowPos(hwnd, NULL, -1, -1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	HDC hdc = GetImageHDC(NULL);

	// 开启全局缓冲绘图
	BeginBatchDraw();

	// 初始化堆栈
	int cols = winWid / blkWid;
	int rows = winHei / blkHei;
	// 创建图
	Map map = createMap(cols, rows);
	// 初始化随机图
	initMap(map);

	// 绘制初始图像
	drawMap(map, hdc, winWid, winHei);

	// 刷新一次缓冲
	FlushBatchDraw();

	// 绘制程序指引
	settextstyle(24, 16, NULL);
	setbkmode(TRANSPARENT);
	settextcolor(0x0055ff);
	setfillcolor(0xffffff);
	fillrectangle(winWid / 8, winHei / 2-50, winWid * 7/8, winHei / 2 + 7 * 24+50);
	outtextxy(winWid / 5, winHei / 2 + 0 * 24, TEXT("欢迎来到寻路算法演示程序"));
	outtextxy(winWid / 5, winHei / 2 + 1 * 24, TEXT("请按任意键继续"));
	outtextxy(winWid / 5, winHei / 2 + 3 * 24, TEXT("Caps+Enter/Return/回车键:加速寻路过程"));
	outtextxy(winWid / 5, winHei / 2 + 4 * 24, TEXT("Caps+Space/空格键：退出寻路过程"));
	outtextxy(winWid / 5, winHei / 2 + 5 * 24, TEXT("Caps+T/t/T键：显示隐藏文字"));
	outtextxy(winWid / 5, winHei / 2 + 6 * 24, TEXT("Caps+E/e/E键：鼠标位置移除方块"));
	outtextxy(winWid / 5, winHei / 2 + 7 * 24, TEXT("Caps+R/r/R键：鼠标位置添加方块"));

	FlushBatchDraw();
	Sleep(4000);

	settextstyle(12, 8, NULL);
	settextcolor(0x0);

	
	while (true)
	{
		blockColor = RGB((rand() % 100), (rand() % 100), (rand() % 100));

		int stat = IDCANCEL;
		// 初始化随机图
		initMap(map);

		// 绘制初始图像
		drawMap(map, hdc, winWid, winHei);

		FlushBatchDraw();
		Sleep(1000);

		// 是否使用A*寻路模式
		bool aStarMode = true;
		
		
		// 下一步偏移定义
		int offset[][2] = {
			{ -1, 0 },
			{ 1, 0 },
			{ 0, -1 },
			{ 0, 1 },
			 { 0, 0 }, // 上下左右四向
			{ -1, -1 },
			{ -1, 1 },
			{ 1, -1 },
			{ 1, 1 },
			{ -999, -999 } // 对角八向
		};
		
		// 大概率使用四向寻路
		if (rand() % 100 < 80){
			offset[4][0] = -999;
		}

		// 一半概率显示文字
		if (rand() % 100 < 50){
			isShowText = !isShowText;
		}

		// 找到多少条路径开始下一张地图
		int breakCount = rand() % 10 + 1;

		// 寻路列表
		vector<Step*> lst;
		// 寻路进度，当前寻路索引
		int idx = 0;

		// 添加开始点到寻路中，开始寻路
		Step* step=new Step();
		step->x = map.beginX;
		step->y = map.beginY;
		step->dis = distance(map.beginX, map.beginY, map.endX, map.endY);
		step->pre = NULL;
		lst.push_back(step);

		// 计算最小距离
		double minDis = distance(map.beginX, map.beginY, map.endX, map.endY);

		// 成功的寻路路径终止节点
		vector<Step*> succ;

		// 直到没有新的点可以走为止
		while (idx<lst.size())
		{
			// 获取当前步骤
			Step* cur = lst[idx];

			// 已经走过+1
			*mapAt(map, cur->x, cur->y) = min(*mapAt(map, cur->x, cur->y)+1,128);

			// 如果是终点坐标，则找到一条路径
			if (cur->x == map.endX && cur->y == map.endY){
				succ.push_back(cur);

				// 绘制成功路径
				drawPath(cur, RGB(0, 200, 255), hdc, blkWid, blkHei);
				drawSEP(map, hdc, winWid, winHei);

				// 显示成功次数
				settextstyle(24, 16, NULL);
				setbkmode(OPAQUE);
				settextcolor(0x0055ff);
				setfillcolor(0xffffff);
				TCHAR buf[20];
				_stprintf(buf, TEXT("****第  %d  条通路****"), (int)succ.size());
				
				outtextxy(winWid - _tcslen(buf)*24, 0, buf);

				settextstyle(12, 8, NULL);
				settextcolor(0x0);
				setbkmode(TRANSPARENT);

				FlushBatchDraw();
				//Sleep(3000);
				// 非阻塞下监听按键
				int time = 3000;
				while (time>0){
					if (isKeyDown(' ') || isKeyDown(VK_SPACE)){
						goto pass_end_tag;
					}
					Sleep(10);
					time -= 10;
				}
				
				// 大于次数后，退出进入下一个地图
				if (succ.size() >= breakCount){
					break;
				}
				idx++;
				continue;
			}
			
			// 下一步已被做过的个数
			int nextRoutedCount = 0;
			// 下一步被走过的权重和
			int sumRoutedWeights = 0;
			// 下一步集合
			vector<Step*> nexts;
			// 枚举下一步可走的区域
			int i = 0;
			while (offset[i][0] != -999){
				// 基于上一步的偏移量进行偏移，得到下一步的坐标
				Step* nst=new Step();
				nst->x = cur->x + offset[i][0];
				nst->y = cur->y + offset[i][1];
				// 不再地图内跳过
				if (nst->x<0 || nst->x>=map.wid || nst->y<0 || nst->y>=map.hei){
					i++;
					continue;
				}
				// 如果是墙体，跳过
				char ch = *mapAt(map, nst->x, nst->y);
				if (ch == BLK_BLOCK ){
					i++;
					continue;
				}
				// 计算此条路径的已走过的长度
				int hisCount = 0;
				// 是否下一步需要走这个点
				bool notSteped = true;
				// 是A*寻路模式，需要遍历本条路径走过的点，防止循环
				if (aStarMode){
					Step* stest = cur;
					while (stest != NULL){
						if (stest->x == nst->x && stest->y == nst->y){
							notSteped = false;
							break;
						}
						hisCount++;
						stest = stest->pre;
					}
				}
				else{ // 平凡寻路，只要是没走过的即可
					notSteped = ch == BLK_EMPTY;
				}
				// 将没走过的点，加入寻路列表
				if (notSteped){
					// 如果是A*寻路，则需要计算下一步的距离，距离重点的距离，也可以说是权重
					if (aStarMode){
						// 成功条数相关性
						double perDis = *mapAt(map, nst->x, nst->y)*1.0 / (succ.size() + 1);
						perDis = pow(perDis, 2.0);
						nst->dis =  distance(nst->x, nst->y, map.endX, map.endY) + perDis;
						// 已走过长度相关，上一步距离相关
						//if (hisCount > minDis){
							nst->dis =nst->dis*0.8+ hisCount*0.1+cur->dis*0.1;
						//}
					}
					else{ // 平凡寻路，无距离概念
						nst->dis = 0;
					}
					
					// 记录这一步的上一步，并添加到队列
					nst->pre = cur;
					lst.push_back(nst);

					// 计算已被走过的次数
					nexts.push_back(nst);
					char nchv = *mapAt(map, nst->x, nst->y);
					if (nchv > 0){
						nextRoutedCount++;
						sumRoutedWeights += nchv;
					}
					
					// 已被走过+1
					*mapAt(map, nst->x, nst->y) = min(nchv+1,128);
					

					if (boostCount % (boostCount/10+1) == 0){
						// 绘制路由路径

						drawMap(map, hdc, winWid, winHei);

						drawPath(nst, RGB(255, 0, 255), hdc, blkWid, blkHei);


						drawSEP(map, hdc, winWid, winHei);

						FlushBatchDraw();
					}
					//MessageBox(hwnd, TEXT("Next"), TEXT("下一步确认"), MB_OK);
					if (isBoostMode()){
						//Sleep(1);
						boostCount++;
					}
					else{
						boostCount = 0;
						Sleep(max(1, min(200 - max(cols, rows),35)));
					}

					
				}
				
;				i++;
			}

			// 如果还没有成功的路径，说明，可能存在直接指向方向有障碍，需要绕行，则增大直接指向的距离权重，使得进行绕行
			if (succ.size() == 0){
				// 如果下一步已经都被走过，则说明绕行概率很高，根据走过的次数平均值，进行增大距离权重，使得绕行
				if (nextRoutedCount == nexts.size()){
					for (vector<Step*>::iterator it = nexts.begin(); it != nexts.end(); it++){
						(*it)->dis *= 1.0 + min(0.1*(sumRoutedWeights / nextRoutedCount), 9.0);
					}
				}
			}

			// 排序找出最短距离优先，对当前步骤之后的所有步骤排序，找出最小距离，作为下一步
			if (aStarMode){
				sort(lst.begin() + idx + 1, lst.end(), comStepDistance);
			}

			
			// 绘制图像
			drawMap(map, hdc, winWid, winHei);

			FlushBatchDraw();
			idx++;
			if (isBoostMode()){
				//Sleep(1);
			}
			else{
				Sleep(60);
			}

			if (isKeyDown(' ') || isKeyDown(VK_SPACE)){
				goto pass_end_tag;
			}
			if (isKeyDown('T') || isKeyDown('t')){
				isShowText = !isShowText;
			}
			if (isKeyDown('E') || isKeyDown('e') || isKeyDown('R') || isKeyDown('r')){
				exchangeBlocks(map,winWid,winHei);
			}
		}

		drawMap(map, hdc, winWid, winHei);
		FlushBatchDraw();
		// 绘制成功路径
		for (int i = 0; i < succ.size(); i++){
			double rate = i*1.0 / succ.size();
			Step* cur = succ[i];
			
			drawPath(succ[i], RGB(0, 200, ((1.0 - rate) * 255)), hdc, blkWid, blkHei,(1.0-rate));

			drawSEP(map, hdc, winWid, winHei);

			FlushBatchDraw();
			//MessageBox(hwnd, TEXT("Next"), TEXT("下一步确认"), MB_OK);
			//Sleep(3000);
			// 非阻塞监听按键功能
			int time = 3000;
			while (time>0){
				if (isKeyDown(' ') || isKeyDown(VK_SPACE)){
					goto pass_end_tag;
				}
				Sleep(10);
				time -= 10;
			}
		}

		
	
		// 轮继续确认
		Sleep(2000);
	pass_end_tag: 

		// 释放步骤申请的内存
		for (vector<Step*>::iterator it = lst.begin(); it != lst.end(); it++){
			delete *it;
		}
	}

	// 释放空间
	releaseMap(map);

	EndBatchDraw();
	closegraph();
	return 0;
}