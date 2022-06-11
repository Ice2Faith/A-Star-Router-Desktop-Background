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

// �����ͼ��������
// �տ�
#define BLK_EMPTY 0
// ǽ��
#define BLK_BLOCK -1

// �Ƿ���ʾ��Ȩ������
bool isShowText = false;

// Ѱ·����
typedef struct _step{
	int x;
	int y;
	struct _step * pre;
	double dis;
} Step;

// ��ͼ
typedef struct _map{
	char * map;
	int wid;
	int hei;
	// ��ʼ������
	int beginX;
	int beginY;
	int endX;
	int endY;
} Map;

// �������
double distance(int x1, int y1, int x2, int y2){
	return sqrt(pow(x2 - x1, 2.0)+ pow(y2 - y1, 2.0));
}

//������ͼ
Map createMap(int wid, int hei){
	char * map = (char*)malloc(sizeof(char)*(wid*hei));
	Map ret;
	ret.wid = wid;
	ret.hei = hei;
	ret.map = map;
	return ret;
}

// �ͷŵ�ͼ�ռ�
void releaseMap(Map& map){
	if (map.map != NULL){
		free(map.map);
		map.map = NULL;
	}
	map.wid = 0;
	map.hei = 0;
}

// ���Ե�ͼ�Ķ�άת��
char* mapAt(Map& map, int x, int y)
{
	return &(map.map[y*map.wid + x]);
}

// ��ʼ����ͼ
void initMap(Map& map)
{
	// ǽ�����
	int rate = rand() % 30+10;
	// ���ɵ�ͼ
	for (int y = 0; y < map.hei; y++){
		for (int x = 0; x < map.wid; x++){
			*mapAt(map, x, y) = BLK_EMPTY;
			if (rand() % 100 < rate){
				*mapAt(map, x, y) = BLK_BLOCK;
			}
		}
	}
	// ���ɿ�ʼ������
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

// ���ƿ�ʼ������
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
// ���Ƶ�ͼ
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

			// ���Ƶ�ͼ����
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

// ����Ƚ���
bool comStepDistance(Step * step1, Step * step2){
	return step1->dis < step2->dis;
}

// ����·��
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



// �Ƿ񰴼�����
bool isKeyDownEx(int keyCode){
	return GetAsyncKeyState(keyCode) & 0x08000;
}

// �����ڼ�ⰴ�����£�����ͬʱ����CAPSLock��
bool isKeyDown(int keyCode){
	return isKeyDownEx(VK_CAPITAL) && isKeyDownEx(keyCode);
}

int boostCount = 0;
// �Ƿ����ģʽ
bool isBoostMode(){
	return isKeyDown(VK_RETURN);
}

// �ڵ�ͼ���ֶ����÷�����Ƴ�����
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
	// ��ʼ�������
	srand((unsigned int)time(NULL));

	int blkWid = 20;
	int blkHei = 20;

	int winWid = 1080;
	int winHei = 720;

	
	HWND hwnd = initgraph(winWid, winHei);

	// ��ȡ���汳������Ϊ������
	HWND wallHwnd = GetDesktopWallpaperWorkerHwnd();
	// ��һ�����ø����ڣ���EasyX�У����initgraph���ص�hwnd����
	// ��ˣ���һ����ΪΪ�����ں�ͨ����ȡ�ߴ纯�������ܻ�ø�����ģʽ�µ���ȷ���ڳߴ�
	if (wallHwnd != NULL){
		SetParent(hwnd, wallHwnd);
	}
	
	
	// ��ȡȫ���ߴ�
	winWid = GetSystemMetrics(SM_CXSCREEN)+2;
	winHei = GetSystemMetrics(SM_CYSCREEN)+2;
	hwnd=initgraph(winWid, winHei);
	
	if (wallHwnd != NULL){
		SetParent(hwnd, wallHwnd);
	}
	
	//�����߿�
	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) - WS_CAPTION);
	ShowWindow(hwnd, SW_MAXIMIZE);

	// �̶�����Ļ���ϽǶ���
	SetWindowPos(hwnd, NULL, -1, -1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	HDC hdc = GetImageHDC(NULL);

	// ����ȫ�ֻ����ͼ
	BeginBatchDraw();

	// ��ʼ����ջ
	int cols = winWid / blkWid;
	int rows = winHei / blkHei;
	// ����ͼ
	Map map = createMap(cols, rows);
	// ��ʼ�����ͼ
	initMap(map);

	// ���Ƴ�ʼͼ��
	drawMap(map, hdc, winWid, winHei);

	// ˢ��һ�λ���
	FlushBatchDraw();

	// ���Ƴ���ָ��
	settextstyle(24, 16, NULL);
	setbkmode(TRANSPARENT);
	settextcolor(0x0055ff);
	setfillcolor(0xffffff);
	fillrectangle(winWid / 8, winHei / 2-50, winWid * 7/8, winHei / 2 + 7 * 24+50);
	outtextxy(winWid / 5, winHei / 2 + 0 * 24, TEXT("��ӭ����Ѱ·�㷨��ʾ����"));
	outtextxy(winWid / 5, winHei / 2 + 1 * 24, TEXT("�밴���������"));
	outtextxy(winWid / 5, winHei / 2 + 3 * 24, TEXT("Caps+Enter/Return/�س���:����Ѱ·����"));
	outtextxy(winWid / 5, winHei / 2 + 4 * 24, TEXT("Caps+Space/�ո�����˳�Ѱ·����"));
	outtextxy(winWid / 5, winHei / 2 + 5 * 24, TEXT("Caps+T/t/T������ʾ��������"));
	outtextxy(winWid / 5, winHei / 2 + 6 * 24, TEXT("Caps+E/e/E�������λ���Ƴ�����"));
	outtextxy(winWid / 5, winHei / 2 + 7 * 24, TEXT("Caps+R/r/R�������λ����ӷ���"));

	FlushBatchDraw();
	Sleep(4000);

	settextstyle(12, 8, NULL);
	settextcolor(0x0);

	
	while (true)
	{
		blockColor = RGB((rand() % 100), (rand() % 100), (rand() % 100));

		int stat = IDCANCEL;
		// ��ʼ�����ͼ
		initMap(map);

		// ���Ƴ�ʼͼ��
		drawMap(map, hdc, winWid, winHei);

		FlushBatchDraw();
		Sleep(1000);

		// �Ƿ�ʹ��A*Ѱ·ģʽ
		bool aStarMode = true;
		
		
		// ��һ��ƫ�ƶ���
		int offset[][2] = {
			{ -1, 0 },
			{ 1, 0 },
			{ 0, -1 },
			{ 0, 1 },
			 { 0, 0 }, // ������������
			{ -1, -1 },
			{ -1, 1 },
			{ 1, -1 },
			{ 1, 1 },
			{ -999, -999 } // �Խǰ���
		};
		
		// �����ʹ������Ѱ·
		if (rand() % 100 < 80){
			offset[4][0] = -999;
		}

		// һ�������ʾ����
		if (rand() % 100 < 50){
			isShowText = !isShowText;
		}

		// �ҵ�������·����ʼ��һ�ŵ�ͼ
		int breakCount = rand() % 10 + 1;

		// Ѱ·�б�
		vector<Step*> lst;
		// Ѱ·���ȣ���ǰѰ·����
		int idx = 0;

		// ��ӿ�ʼ�㵽Ѱ·�У���ʼѰ·
		Step* step=new Step();
		step->x = map.beginX;
		step->y = map.beginY;
		step->dis = distance(map.beginX, map.beginY, map.endX, map.endY);
		step->pre = NULL;
		lst.push_back(step);

		// ������С����
		double minDis = distance(map.beginX, map.beginY, map.endX, map.endY);

		// �ɹ���Ѱ··����ֹ�ڵ�
		vector<Step*> succ;

		// ֱ��û���µĵ������Ϊֹ
		while (idx<lst.size())
		{
			// ��ȡ��ǰ����
			Step* cur = lst[idx];

			// �Ѿ��߹�+1
			*mapAt(map, cur->x, cur->y) = min(*mapAt(map, cur->x, cur->y)+1,128);

			// ������յ����꣬���ҵ�һ��·��
			if (cur->x == map.endX && cur->y == map.endY){
				succ.push_back(cur);

				// ���Ƴɹ�·��
				drawPath(cur, RGB(0, 200, 255), hdc, blkWid, blkHei);
				drawSEP(map, hdc, winWid, winHei);

				// ��ʾ�ɹ�����
				settextstyle(24, 16, NULL);
				setbkmode(OPAQUE);
				settextcolor(0x0055ff);
				setfillcolor(0xffffff);
				TCHAR buf[20];
				_stprintf(buf, TEXT("****��  %d  ��ͨ·****"), (int)succ.size());
				
				outtextxy(winWid - _tcslen(buf)*24, 0, buf);

				settextstyle(12, 8, NULL);
				settextcolor(0x0);
				setbkmode(TRANSPARENT);

				FlushBatchDraw();
				//Sleep(3000);
				// �������¼�������
				int time = 3000;
				while (time>0){
					if (isKeyDown(' ') || isKeyDown(VK_SPACE)){
						goto pass_end_tag;
					}
					Sleep(10);
					time -= 10;
				}
				
				// ���ڴ������˳�������һ����ͼ
				if (succ.size() >= breakCount){
					break;
				}
				idx++;
				continue;
			}
			
			// ��һ���ѱ������ĸ���
			int nextRoutedCount = 0;
			// ��һ�����߹���Ȩ�غ�
			int sumRoutedWeights = 0;
			// ��һ������
			vector<Step*> nexts;
			// ö����һ�����ߵ�����
			int i = 0;
			while (offset[i][0] != -999){
				// ������һ����ƫ��������ƫ�ƣ��õ���һ��������
				Step* nst=new Step();
				nst->x = cur->x + offset[i][0];
				nst->y = cur->y + offset[i][1];
				// ���ٵ�ͼ������
				if (nst->x<0 || nst->x>=map.wid || nst->y<0 || nst->y>=map.hei){
					i++;
					continue;
				}
				// �����ǽ�壬����
				char ch = *mapAt(map, nst->x, nst->y);
				if (ch == BLK_BLOCK ){
					i++;
					continue;
				}
				// �������·�������߹��ĳ���
				int hisCount = 0;
				// �Ƿ���һ����Ҫ�������
				bool notSteped = true;
				// ��A*Ѱ·ģʽ����Ҫ��������·���߹��ĵ㣬��ֹѭ��
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
				else{ // ƽ��Ѱ·��ֻҪ��û�߹��ļ���
					notSteped = ch == BLK_EMPTY;
				}
				// ��û�߹��ĵ㣬����Ѱ·�б�
				if (notSteped){
					// �����A*Ѱ·������Ҫ������һ���ľ��룬�����ص�ľ��룬Ҳ����˵��Ȩ��
					if (aStarMode){
						// �ɹ����������
						double perDis = *mapAt(map, nst->x, nst->y)*1.0 / (succ.size() + 1);
						perDis = pow(perDis, 2.0);
						nst->dis =  distance(nst->x, nst->y, map.endX, map.endY) + perDis;
						// ���߹�������أ���һ���������
						//if (hisCount > minDis){
							nst->dis =nst->dis*0.8+ hisCount*0.1+cur->dis*0.1;
						//}
					}
					else{ // ƽ��Ѱ·���޾������
						nst->dis = 0;
					}
					
					// ��¼��һ������һ��������ӵ�����
					nst->pre = cur;
					lst.push_back(nst);

					// �����ѱ��߹��Ĵ���
					nexts.push_back(nst);
					char nchv = *mapAt(map, nst->x, nst->y);
					if (nchv > 0){
						nextRoutedCount++;
						sumRoutedWeights += nchv;
					}
					
					// �ѱ��߹�+1
					*mapAt(map, nst->x, nst->y) = min(nchv+1,128);
					

					if (boostCount % (boostCount/10+1) == 0){
						// ����·��·��

						drawMap(map, hdc, winWid, winHei);

						drawPath(nst, RGB(255, 0, 255), hdc, blkWid, blkHei);


						drawSEP(map, hdc, winWid, winHei);

						FlushBatchDraw();
					}
					//MessageBox(hwnd, TEXT("Next"), TEXT("��һ��ȷ��"), MB_OK);
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

			// �����û�гɹ���·����˵�������ܴ���ֱ��ָ�������ϰ�����Ҫ���У�������ֱ��ָ��ľ���Ȩ�أ�ʹ�ý�������
			if (succ.size() == 0){
				// �����һ���Ѿ������߹�����˵�����и��ʺܸߣ������߹��Ĵ���ƽ��ֵ�������������Ȩ�أ�ʹ������
				if (nextRoutedCount == nexts.size()){
					for (vector<Step*>::iterator it = nexts.begin(); it != nexts.end(); it++){
						(*it)->dis *= 1.0 + min(0.1*(sumRoutedWeights / nextRoutedCount), 9.0);
					}
				}
			}

			// �����ҳ���̾������ȣ��Ե�ǰ����֮������в��������ҳ���С���룬��Ϊ��һ��
			if (aStarMode){
				sort(lst.begin() + idx + 1, lst.end(), comStepDistance);
			}

			
			// ����ͼ��
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
		// ���Ƴɹ�·��
		for (int i = 0; i < succ.size(); i++){
			double rate = i*1.0 / succ.size();
			Step* cur = succ[i];
			
			drawPath(succ[i], RGB(0, 200, ((1.0 - rate) * 255)), hdc, blkWid, blkHei,(1.0-rate));

			drawSEP(map, hdc, winWid, winHei);

			FlushBatchDraw();
			//MessageBox(hwnd, TEXT("Next"), TEXT("��һ��ȷ��"), MB_OK);
			//Sleep(3000);
			// ������������������
			int time = 3000;
			while (time>0){
				if (isKeyDown(' ') || isKeyDown(VK_SPACE)){
					goto pass_end_tag;
				}
				Sleep(10);
				time -= 10;
			}
		}

		
	
		// �ּ���ȷ��
		Sleep(2000);
	pass_end_tag: 

		// �ͷŲ���������ڴ�
		for (vector<Step*>::iterator it = lst.begin(); it != lst.end(); it++){
			delete *it;
		}
	}

	// �ͷſռ�
	releaseMap(map);

	EndBatchDraw();
	closegraph();
	return 0;
}