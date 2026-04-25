//////////////////////////////////////////////////////////////////////////////////
//
//  最近修改   : 2022-11-27
//  功能描述   : 0.96寸OLED 接口演示例程(STM32F103C8系列IIC)
//              说明:
//              ----------------------------------------------------------------
//              GND   电源地
//              VCC   接3.3v电源
//              SCL   接PA5（SCL）
//              SDA   接PA7（SDA）
//              ----------------------------------------------------------------
//  by:凝望夜空的心
//////////////////////////////////////////////////////////////////////////////////

#include "oled.h"
#include "gui.h"
#include "oledfont.h"
#include "bmp.h"
#include "delay.h"
#include "mymath.h"
#include "string.h"

//GUI
u8 Map[8][128] = { 0 };
//存放格式如下.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127

//旋转参数，默认旋转中心为显示区域坐标，对应贴图上的坐标(0,0)点(左上角)，角度为顺时针旋转
struct SpanSet{
	POINT spanCenter;	//字旋转中心
	POINT spanZero;		//句旋转中心
	float ang;				//旋转角
	float argCos;			//旋转角余弦值
	float argSin;			//旋转角正弦值
}	spanSet;

//设置旋转角度
void GUI_SetSpanAng(float ang)
{
	spanSet.ang = ang;
	spanSet.argSin = MYMATH_Sin(ang);
	spanSet.argCos = MYMATH_Cos(ang);
}
//设置字旋转中心
void GUI_SetSpan(float x, float y)
{
	spanSet.spanCenter.x = x;
	spanSet.spanCenter.y = y;
}
//设置句旋转中心
void GUI_SetSpanZero(float x, float y)
{
	spanSet.spanZero.x = x;
	spanSet.spanZero.y = y;
}
//设置旋转中心，(原偏移坐标，设置的坐标)返回设置后的偏移坐标(贴图左上角坐标)
POINT GUI_SetSpanCenter(float xBef, float yBef, float xSet, float ySet)
{
	POINT point;
	point.x = xSet - spanSet.argCos * (xSet - xBef) + spanSet.argSin * (ySet - yBef);
	point.y = ySet - spanSet.argSin * (xSet - xBef) - spanSet.argCos * (ySet - yBef);
	return point;
}
/***************************************************
//GUI_WriteMap:向Map中写入数据
//bit:写入的位
//xpose,ypose:x,y坐标
***************************************************/
void GUI_WriteMap(u8 bit, int xpose, int ypose)
{
	u8 page = 0;
	u8 y = 0;
	u8 bit_ = 0x01;
	if(xpose >=0 && xpose < X_WIDTH && ypose >= 0 && ypose < Y_WIDTH)
	{
		page = ypose / Y_PAGE;
		for(y = ypose % Y_PAGE; y > 0; y--)	bit_ <<= 1;
		Map[page][xpose] &= (~bit_);
		if(bit > 0)	Map[page][xpose] |= bit_;
	}
}
/***************************************************
//Map载入显存
***************************************************/
void GUI_ShowMap(void)
{
	OLED_ShowPic(Map[0]);
}
/***************************************************
//清除Map数据
***************************************************/
void GUI_ClearMap(u8 data)
{
	u8 i = 0, n = 0;
	for(i = 0; i < 8; i++)
	{
		for(n = 0; n < 128; n++)
		{
			Map[i][n] = data;
		}
	}
}
//二进制数反转
u8 reversedata(u8 data)
{
	u8 i = 0;
	u8 tem = 0;
	u8 ans = 0;
	for(i=0; i<8;i++)
	{
		tem = data & 0x01;
		ans <<= 1;
		ans += tem;
		data >>= 1;
	}
	return ans;
}

/**********************************************/
/****************** 显示函数 ******************/
/**********************************************/

/***************************************************
//GUI_Reverse: 将选定的区域取反
//xpose,ypose :	左上角坐标
//width,height:	区域宽高
****************************************************/
void GUI_Reverse(int xpose, int ypose, u8 width, u8 height)
{
	u8 xend = 0, yend = 0, ypagestart = 0, ypageend = 0;
	u8 i = 0, n = 0;
	u8 y1 = 0, y2 = 0;	//第一个、最后一个字节需要取反的位数
	u8 len = 0;					//记录取反的总位数

	if(xpose < 0)	xpose = 0;
	if(ypose < 0)	ypose = 0;
	xend = xpose + width;
	yend = ypose + height;
	if(xend >= X_WIDTH)	xend = X_WIDTH;
	if(yend >= Y_WIDTH)	yend = Y_WIDTH;
	ypagestart = ypose / 8;
	ypageend = yend / 8;
	y1 = ypose - 8 * ypagestart;
	y2 = yend - 8 * ypageend;

	for(i = xpose; i < xend; i++)
	{
		len = 0;
		/*对开始一个字节存在的部分位取反*/
		for(n = (y1 > 0 ? y1 : 8); n < 8 && len < height; n++)
		{
			len++;
			Map[ypagestart][i] ^= (0x01 << n);
		}
		/*对中间存在的整个字节取反*/
		for(n = ypagestart + (y1 > 0 ? 1 : 0); n < ypageend && n < Y_PAGE && len < height; n++)
		{
			len += 8;
			Map[n][i] = ~Map[n][i];
		}
		/*对最后一个字节存在的部分位取反*/
		for(n = (y2 > 0 ? (y2 - 1) : 0); n < 8 && len < height; n--)
		{
			len++;
			Map[ypageend][i] ^= (0x01 << n);
		}
	}
}
/***************************************************
//GUI_ShowChar: 在指定位置显示一个字符,包括部分字符
//x,y :左上角坐标
//ch  :待显示的字符
//Size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
****************************************************/
void GUI_ShowChar(int x, int y, u8 ch, u8 Size, u8 mode)
{
	u8 i = 0, n = 0;
	u8 o = 0, p = 0;
	float xpose = 0, ypose = 0;
	u8 temp = 0;
	ch -= ' ';  //得到偏移后的值

	for(i = 0; i < 8; i++)
	{
		if(Size == SIZE16)
		{
			if(mode&L2R)	o = 8 - i;
			else o = i;
		}
		else
		{
			if(i >= 6)		break;
			if(mode&L2R)	o = 6 - i;
			else o = i;
		}

		for(n = 0; n < 16; n++)
		{
			if(Size == SIZE16)
			{
				if(mode&U2D)	p = 16 - n;
				else p = n;
				temp = F8X16[ch*16+i+n/8*8]&(0x01<<(n%8));
			}
			else
			{
				if(n >= 8)		break;
				if(mode&U2D)	p = 8 - n;
				else p = n;
				temp = F6x8[ch][i]&(0x01<<n);
			}

			if((mode&Transparent) && temp == 0) continue;	//透明贴图
			if(mode&Reverse)			temp = !temp;						//反色
			if((mode&Span) || (mode&SpanSingle))					//旋转
			{
				xpose = spanSet.spanCenter.x + spanSet.argCos * o - spanSet.argSin * p;
				ypose = spanSet.spanCenter.y + spanSet.argSin * o + spanSet.argCos * p;
				GUI_WriteMap(temp, (int)xpose, (int)ypose);
			}
			else
				GUI_WriteMap(temp, o+x, p+y);
		}
	}
}
/***************************************************
//GUI_ShowNum: 显示数字
//x,y :左上角坐标
//length :指定显示数字长度（从个位开始），为0自动确定长度
//num:数值(0~4294967295)
//Size:字体大小
//if0 :是否将不足位上补零
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void GUI_ShowNum(int x,int y,u32 num, u8 length, u8 size, u8 if0, u8 mode)
{
	int i = 0;
	u32 temp = num;
	u8 data = 0;

	if(size != SIZE16)	size = SIZE12;
//	if(mode&Span || mode&SpanSingle)	GUI_SetSpan(x,y);

	if(length == 0)
	{
		do{
			temp /= 10;
			length++;
		}while(temp > 0);
	}
	for(i = 0; i < length; i++)
	{
		data = num % 10 + '0';
		num /= 10;
		if(num == 0 && data == '0')
		{
			if(if0 == Add0)	data = '0';
			else					data = ' ';
		}
		if(mode&Span)	GUI_SetSpan(x+(size/2)*(length-i-1),y);
		else if(mode&SpanSingle)
		{
			GUI_SetSpan(spanSet.spanZero.x+(x+(size/2)*(length-i-1)-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x+(size/2)*(length-i-1)-spanSet.spanZero.x)*spanSet.argSin);
		}
		GUI_ShowChar(x+(size/2)*(length-i-1),y,data,size,mode);
	}
}

/***************************************************
//GUI_ShowInt: 显示整型数
//x,y :左上角坐标
//num :整型数值
//size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void GUI_ShowInt(int x, int y, int num, u8 size, u8 mode)
{
	int temp = 0;
	u8 length = 0;

	if(size != SIZE16)	size = SIZE12;

	if(mode&Span || mode&SpanSingle)
	{
		GUI_SetSpan(x,y);
		GUI_SetSpanZero(x,y);
	}

	if(num < 0)
	{
		GUI_ShowChar(x,y,'-',size,mode);
		x += size / 2;
		num = -num;
	}

	temp = num;

	do{
		temp /= 10;
		length++;
	}while(temp > 0);
	if(mode&Span)	GUI_SetSpan(x+size/2*length,y);
	else if(mode&SpanSingle)
	{
		GUI_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
									spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
	}
	GUI_ShowNum(x,y,num,length,size,Add0,mode);
}

/***************************************************
//GUI_ShowFloat: 显示浮点数，当len = 0时，可显示整数
//x,y :左上角坐标
//num :浮点型数值
//len :保留的小数位数
//size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void GUI_ShowFloat(int x, int y, float num, u8 len, u8 size, u8 mode)
{
	int temp = 0;
	int tempInt = 0;
	int tempFloat2Int = 0;
	float tempFloat = 0;
	u8 length = 0;

	if(size != SIZE16)	size = SIZE12;

	if(mode&Span || mode&SpanSingle)
	{
		GUI_SetSpan(x,y);
		GUI_SetSpanZero(x,y);
	}

	if(num < 0)
	{
		GUI_ShowChar(x,y,'-',size,mode);
		x += size / 2;
		num = -num;
	}

	tempInt = (int)num;
	tempFloat = num - tempInt;
	length = len;

	while(length > 0)
	{
		tempFloat *= 10;
		length--;
	}
	tempFloat2Int = (int)tempFloat;
	temp = tempInt;
	length = 0;
	do{
		temp /= 10;
		length++;
	}while(temp > 0);
	if(mode&Span)	GUI_SetSpan(x+size/2*length,y);
	else if(mode&SpanSingle)
	{
		GUI_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
									spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
	}
	GUI_ShowNum(x,y,tempInt,length,size,Add0,mode);
	if(len > 0)
	{
		if(mode&Span)	GUI_SetSpan(x+size/2*length,y);
		else if(mode&SpanSingle)
		{
			GUI_SetSpan(spanSet.spanZero.x+(x+size/2*length-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x+size/2*length-spanSet.spanZero.x)*spanSet.argSin);
		}
		GUI_ShowChar(x+size/2*length,y,'.',size,mode);
		if(mode&Span)	GUI_SetSpan(x+size/2*length+size/2,y);
		else if(mode&SpanSingle)
		{
			GUI_SetSpan(spanSet.spanZero.x+(x+size/2*length+size/2-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x+size/2*length+size/2-spanSet.spanZero.x)*spanSet.argSin);
		}
		GUI_ShowNum(x+size/2*length+size/2,y,tempFloat2Int,len,size,Add0,mode);
	}
}


/***************************************************
//GUI_ShowString: 显示一个字符号串
//x,y :左上角坐标
//chr :字符串
//size:字体大小
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void GUI_ShowString(int x, int y, char *chr, u8 size, u8 mode)
{
	u8 i = 0;
	if(size != SIZE16)	size = SIZE12;
	if(mode&SpanSingle)	GUI_SetSpanZero(x,y);

	while(chr[i] != '\0')
	{
		if(mode&Span)	GUI_SetSpan(x,y);
		else if(mode&SpanSingle)
		{
			GUI_SetSpan(spanSet.spanZero.x+(x-spanSet.spanZero.x)*spanSet.argCos,
										spanSet.spanZero.y+(x-spanSet.spanZero.x)*spanSet.argSin);
		}
		GUI_ShowChar(x,y,chr[i],size,mode);
		x += size / 2;
		i++;
	}
}
/***************************************************
//GUI_ShowChinese: 显示汉字
//x,y :左上角坐标
//num :待显示汉字字符编号，字体默认16号
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void GUI_ShowChinese(int x, int y, u8 num, u8 mode)
{
	int i = 0, n = 0;
	u8 o = 0, p = 0;
	float xpose = 0, ypose = 0;
	u8 temp = 0;

//	if(mode&Span || mode&SpanSingle)	GUI_SetSpan(x,y);

	for(i = 0; i < 16; i++)
	{
		if(mode&L2R)	o = 16 - i;
		else o = i;

		for(n = 0; n < 16; n++)
		{
			if(mode&U2D)	p = 16 - n;
			else p = n;

			// temp = Hzk[2*num+n/8][i]&(0x01<<(n%8));
			temp = CN16_Msk[num].Msk[i + (n / 8) * 16] & (0x01<<(n%8));
			if((mode&Transparent) && temp == 0) continue;
			if(mode&Reverse)			temp = !temp;
			if((mode&Span) || (mode&SpanSingle))					//旋转
			{
				xpose = spanSet.spanCenter.x + spanSet.argCos * o - spanSet.argSin * p;
				ypose = spanSet.spanCenter.y + spanSet.argSin * o + spanSet.argCos * p;
				GUI_WriteMap(temp, (int)xpose, (int)ypose);
			}
			else
				GUI_WriteMap(temp, o+x, p+y);
		}
	}
}

//GUI_ShowChineseStr: 显示一句话
void GUI_printf(int x, int y, char *s, u8 mode)
{
	unsigned char i, k, length;
	unsigned short Index = 0;
	length = strlen(s); // 取字符串总长
	// printf("length = %d\r\n",length);

	for (k = 0; k < length; k++)
	{
		// printf("k = %d\r\n",k);
		if (*(s + k) <= 127)
		{ // 小于128是ASCII符号
			// printf("ascii\r\n");
			GUI_ShowChar(x, y, *(s + k), SIZE16, mode);
			x += 8; // x坐标右移8
		}
		else if (*(s + k) > 127)
		{ // 大于127，为汉字，前后两个组成汉字内码
			// printf("汉字\r\n");
			Index = (*(s + k) << 8) | (*(s + k + 1)); // 取汉字的内码
			for (i = 0; i < sizeof(CN16_Msk) / sizeof(typFNT_GB16); i++)
			{ // 查数组
				if (Index == CN16_Msk[i].Index)
				{
					GUI_ShowChinese(x, y, i, mode);

					// 查询到这个字
					// OLED_Set_Pos(x, y);
					// for (t = 0; t < 16; t++)
					// 	OLED_WR_Byte(CN16_Msk[i].Msk[t], OLED_DATA); // 写入字模

					// OLED_Set_Pos(x, y + 1);
					// for (t = 16; t < 32; t++)
					// 	OLED_WR_Byte(CN16_Msk[i].Msk[t], OLED_DATA);

					x += 16;
					k += 1; // 汉字占2B,跳过一个
				}
			}
		}
	}
}

/**********************************************/
/****************** 绘图函数 ******************/
/**********************************************/
//绘制直线
void GUI_line(int xpose1, int ypose1, int xpose2, int ypose2, u8 width)
{
	int i = 0, n = 0;
	float k = 0;
	float y = 0;
	if(xpose1 < xpose2)
	{
		if(ypose1 < ypose2)//从左上向右下画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i < xpose2; i++)
				for(n = ypose1; n < ypose2; n++)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							GUI_WriteMap(1, i, n);
					}
		}
		else if(ypose1 > ypose2)//从左下向右上画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i < xpose2; i++)
				for(n = ypose1; n > ypose2; n--)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							GUI_WriteMap(1, i, n);
					}
		}
		else	//从左向右画线
		{
			for(i = xpose1; i < xpose2; i++)
				for(n = ypose1 - width / 2; n <= ypose1 + width / 2; n++)
					if(i <128 &&  n < 64)
						GUI_WriteMap(1, i, n);
		}
	}
	else if(xpose1 > xpose2)
	{
		if(ypose1 < ypose2)//从右下向左上画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i > xpose2; i--)
				for(n = ypose1; n < ypose2; n++)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							GUI_WriteMap(1, i, n);
					}
		}
		else if(ypose1 > ypose2)//从右上向左下画线
		{
			k = (float)(ypose1 - ypose2) / (float)(xpose1 - xpose2);
			for(i = xpose1; i > xpose2; i--)
				for(n = ypose1; n > ypose2; n--)
					if(i <128 &&  n < 64)
					{
						y = k * (i - xpose2) + ypose2;
						if(y < (float)n + (float)width / 2 && y > (float)n - (float)width / 2)
							GUI_WriteMap(1, i, n);
					}
		}
		else	//从右向左画线
		{
			for(i = xpose1; i > xpose2; i--)
				for(n = ypose1 - width / 2; n <= ypose1 + width / 2; n++)
					if(i <128 &&  n < 64)
						GUI_WriteMap(1, i, n);
		}
	}
	else
	{
		if(ypose1 > ypose2)	//从下向上画线
		{
			for(i = xpose1 - width / 2; i <= xpose1 + width / 2; i++)
				for(n = ypose1; n > ypose2; n--)
					if(i <128 &&  n < 64)
						GUI_WriteMap(1, i, n);
		}
		else if(ypose1 < ypose2)	//从上向下画线
		{
			for(i = xpose1 - width / 2; i <= xpose1 + width / 2; i++)
				for(n = ypose1; n < ypose2; n++)
					if(i <128 &&  n < 64)
						GUI_WriteMap(1, i, n);
		}
		else	//画点
			GUI_rectangle(xpose1 - width / 2, ypose1 - width / 2, width,width);
	}
}
//绘制直线
void GUI_DrawLine(int x, int y, float ang, u8 length, u8 width)
{
	u8 i = 0;
	float n = 0;
	float xpose = 0, ypose = 0;
	float cosarg = MYMATH_Cos(ang), sinarg = MYMATH_Sin(ang);
	for(i = 0; i < length; i++)
	{
		for(n = -width/2.0; n < width/2.0; n++)
		{
			xpose = x + cosarg * i - sinarg * n;
			ypose = y + sinarg * i + cosarg * n;
			GUI_WriteMap(1, (int)xpose, (int)ypose);
		}
	}
}
//绘制圆形
void GUI_circle(int xpose, int ypose, u8 r)
{
	int i = 0, n = 0;
	for(i = xpose - r; i <= xpose + r; i++)
	{
		for(n = ypose - r; n <= ypose + r; n++)
		{
			if((i - xpose) * (i - xpose) + (n - ypose) * (n - ypose) <= r * r + 1)
				GUI_WriteMap(1, i, n);
		}
	}
}
//绘制圆形不填充
void GUI_circleunfilled(int xpose, int ypose, u8 r, u8 width)
{
	int i = 0, n = 0;
	width /= 2;
	for(i = xpose - r - width; i <= xpose + r + width; i++)
	{
		for(n = ypose - r - width; n <= ypose + r + width; n++)
		{
			if((i - xpose) * (i - xpose) + (n - ypose) * (n - ypose) <= (r + width) * (r + width) + 1
					&& (i - xpose) * (i - xpose) + (n - ypose) * (n - ypose) >= (r - width) * (r - width) - 1)
				GUI_WriteMap(1, i, n);
		}
	}
}
//绘制矩形
void GUI_rectangle(int xpose, int ypose, u8 width, u8 height)
{
	int i = 0, n = 0;
	for(i = xpose; i <= xpose + width; i++)
	{
		for(n = ypose; n <= ypose + height; n++)
		{
			GUI_WriteMap(1, i, n);
		}
	}
}
//绘制矩形不填充
void GUI_rectangleunfilled(int xpose, int ypose, u8 width, u8 height, u8 linewidth)
{
	int i = 0, n = 0;
	linewidth /= 2;
	for(i = xpose - linewidth; i <= xpose + width + linewidth; i++)
	{
		for(n = ypose - linewidth; n <= ypose + height + linewidth; n++)
		{
			if((i < xpose + linewidth + 1 || i >= xpose + width - linewidth)
					|| (n < ypose + linewidth + 1 || n >= ypose + height - linewidth))
				GUI_WriteMap(1, i, n);
		}
	}
}
/*
//绘制平行四边形
//最左边(最上边(先左后上))的顶点为1,四个顶点标号依次为顺时针方向1,2,3,4
//void GUI_rectangle(u8 xpose1, u8 ypose1, u8 xpose2, u8 ypose2,u8 xpose3, u8 ypose3)
//{

//}
//绘制三角形
//void GUI_triangle(u8 xpose1, u8 ypose1, u8 xpose2, u8 ypose2,u8 xpose3, u8 ypose3)
//{

//}
*/
/***************************************************
//GUI_ShowBMP: 显示位图
//x0、y0、x1、y1:分别为左上角坐标右下角坐标
//BMP :图片
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void GUI_ShowBMP(int x0, int y0, int x1, int y1, const u8* BMP, u8 mode)
{
	u32  j = 0;
	u8 index = 0;
	u8 temp = 0;
	int i = 0, n = 0;
	int o = 0, p = 0;
	float xpose, ypose;

	for(i = x0; i < x1; i++)
	{
    for(n = y0; n < y1; n += 8)
	  {
			if(mode&L2R)	o = x1 + x0 - i;
			else o = i;
			for(index = 0; index < 8 && index < y1-n; index++)
			{
				if(mode&U2D)	p = y1 + y0 - n - index;
				else p = n+index;
				temp = (BMP[j]<<index)&0x80;
				if((mode&Transparent) && temp == 0) continue;	//透明贴图
				if(mode&Reverse)			temp = !temp;						//反色
				if((mode&Span) || (mode&SpanSingle))					//旋转
				{
					xpose = x0 + spanSet.argCos * (o - x0) - spanSet.argSin * (p - y0);
					ypose = y0 + spanSet.argSin * (o - x0) + spanSet.argCos * (p - y0);
					GUI_WriteMap(temp, (int)xpose, (int)ypose);
				}
				else GUI_WriteMap(temp,o,p);
			}
			j++;
	  }
	}
}
/***************************************************
//GUI_ShowPNG: 显示透明贴图
//x0、y0、x1、y1:分别为左上角坐标右下角坐标
//Mask:图片掩码（决定在原图中抠出的形状）
//PNG :图片
//mode:显示模式，透明、反色、旋转、翻转
***************************************************/
void GUI_ShowPNG(int x0, int y0, int x1, int y1, const u8* Mask, const u8* PNG, u8 mode)
{
	u32  j = 0;
	u8 index = 0;
	u8 temp = 0;
	int i = 0, n = 0;
	int o = 0, p = 0;
	float xpose, ypose;

	for(i = x0; i < x1; i++)
	{
    for(n = y0; n < y1; n += 8)
	  {
			if(mode&L2R)	o = x1 + x0 - i;
			else o = i;
			for(index = 0; index < 8 && index < y1-n; index++)
			{
				if(mode&U2D)	p = y1 + y0 - n - index;
				else p = n+index;
				temp = (PNG[j]<<index)&0x80;
				if(((Mask[j]<<index)&0x80) == 0)			continue;	//判断掩码值
				if((mode&Transparent) && temp == 0) continue;	//透明贴图
				if(mode&Reverse)			temp = !temp;						//反色
				if((mode&Span) || (mode&SpanSingle))					//旋转
				{
					xpose = x0 + spanSet.argCos * (o - x0) - spanSet.argSin * (p - y0);
					ypose = y0 + spanSet.argSin * (o - x0) + spanSet.argCos * (p - y0);
					GUI_WriteMap(temp, (int)xpose, (int)ypose);
				}
				else GUI_WriteMap(temp,o,p);
			}
			j++;
	  }
	}
}



//GUI_Center_Bmp: 指定bmp图片横向显示居中
//y、width、height、y1:分别为图片起始高度，图片宽度，图片高度，
//BMP :图片
//mode:显示模式，透明、反色、旋转、翻转
void GUI_Center_Bmp(int y, int width, int height, const u8* BMP, u8 mode)
{
	int x0 = (X_WIDTH - width) / 2;
	int x1 = (X_WIDTH - width) / 2 + width - 1;
	int y0 = y;
	int y1 = y + height - 1;

	GUI_ShowBMP(x0, y0, x1, y1, BMP, mode);
}


//GUI_Center_Bmp: 指定中英文横向显示居中（暂时只支持16*16，16*8）
//y:分别为图片起始高度，图片宽度，图片高度，
//str :文字和英文
//mode:显示模式，透明、反色、旋转、翻转
void GUI_Center_str(int y, const char* str, u8 mode)
{
	int x0 = 0;
	int y0 = y;

	x0 = (X_WIDTH - (strlen(str) * 8)) / 2;

	GUI_printf(x0, y0, (char *)str, mode);
}

