/*----------------------------------
updated by wonderly321 on 4/11/19.
-----------------------------------*/

#include "image_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include "Windows.h"

//read bmp image and convert to gray scale
int read_bmp(const char * filename, unsigned char * & data, int & w, int & h)
{
    unsigned char head[54];
    FILE *f = fopen(filename,"rb");
	if (NULL == f)
	{
		printf("can't open %s,please check!", filename);
		system("pause");
		return -1;//return -1 :can't open file.

	}
	// BMP header is 54 bytes
	fread(head, 1, 54, f);
    w = head[18] + ( ((int)head[19]) << 8) + ( ((int)head[20]) << 16) + ( ((int)head[21]) << 24);
    h = head[22] + ( ((int)head[23]) << 8) + ( ((int)head[24]) << 16) + ( ((int)head[25]) << 24);

    unsigned char *temp = (unsigned char*)malloc(w*h);
    fseek(f,54,SEEK_SET);

    int count = w * h - 1;
    unsigned char b,g,r;
    while (!feof(f))
    {
        // read scan line
        for (int i = 0; i < w; i++)
        {
            // because the bitmap is stored inverted, the color
            // byte Blue is always the first
            if (!feof(f))
            {
                fread((unsigned char*)&b, sizeof(unsigned char), 1, f);
            }
            // Then we read the Green byte.
            if (!feof(f))
            {
                fread((unsigned char*)&g, sizeof(unsigned char), 1, f);
            }
            // Finally, the Red byte.
            if (!feof(f))
            {
                fread((unsigned char*)&r, sizeof(unsigned char), 1, f);
            }
            unsigned char scale = (unsigned char)(0.299*r + 0.587*g + 0.114*b);
            temp[count] = scale;
            --count;     // counter operation for pixel index.e
        }
        if(count == -1)
            break;
    }
    data = (unsigned char*)malloc(w*h);
// We do another inversion of the array.
    count = 0;
    unsigned  char *t2;
    for (int y = 0; y < h; y++)
    {
        for (int x = w - 1; x > -1; x--)
        {
            t2 = temp + y * w + x;
            data[count] = *t2;
            count++;
        }
    }
    delete [] temp;      // when done, the old array is discarded.
    fclose(f);          // close file
    return 0;
}

void write_pgm(const char * filename, unsigned char* data, int w, int h)
{
    FILE* out_file;
    assert(w > 0);
    assert(h > 0);

    out_file = fopen(filename, "wb");
    if (! out_file){
        fprintf(stderr, "Fail to open file: %s\n", filename);
        exit(1);
    }

    fprintf(out_file, "P5\n");
    fprintf(out_file, "%d %d\n255\n", w, h);
    fwrite(data, sizeof(unsigned char), w * h, out_file);
    fclose(out_file);
}// write_pgm ()


void write_float_pgm(const char* filename, float* data, int w, int h, int mode)
{
    int i, j;
    unsigned char* charImg;
    int tmpInt;
    charImg = (unsigned char*)malloc(w*h*sizeof(unsigned char));
    for(i = 0; i < h; i++)
    {
        for(j = 0; j < w; j++)
        {
            if(mode == 1){ // clop
                if( data[i*w+j] >= 255.0){
                    charImg[i*w+j] = 255;
                } else if(data[i*w+j] <= 0.0){
                    charImg[i*w+j]=0;
                } else{
                    charImg[i*w+j]=(int)data[i*w+j];
                }
            }else if(mode == 2){ // abs, x10, clop
                tmpInt =
                        (int)(fabs(data[i*w+j])*10.0);
                if( fabs(data[i*w+j]) >= 255){
                    charImg[i*w+j] = 255;
                } else if(tmpInt <= 0){
                    charImg[i*w+j] = 0;
                } else{
                    charImg[i*w+j] = tmpInt;
                }
            }else{
                return;
            }
        }
    }
    write_pgm(filename, charImg, w, h);
    free(charImg);
}// write_float_pgm()

void setPixelRed(BMP_IMG* img, int r, int c) {
    if( (r >= 0) && (r < img->h) && (c >= 0) && (c < img->w)){
        img->img_r[r*img->w + c] = 0;
        img->img_g[r*img->w + c] = 0;
        img->img_b[r*img->w + c] = 255;
    }
}// setPixelRed()


//http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void rasterCircle(BMP_IMG* imgBMP, int r, int c, int radius)
{
    int f = 1 - radius;
    int ddF_x = 1;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;

    int x0 = r;
    int y0 = c;

    setPixelRed(imgBMP, x0, y0 + radius);
    setPixelRed(imgBMP, x0, y0 - radius);
    setPixelRed(imgBMP, x0 + radius, y0);
    setPixelRed(imgBMP, x0 - radius, y0);

    while(x < y)
    {
        // ddF_x == 2 * x + 1;
        // ddF_y == -2 * y;
        // f == x*x + y*y - radius*radius + 2*x - y + 1;
        if(f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        setPixelRed(imgBMP, x0 + x, y0 + y);
        setPixelRed(imgBMP, x0 - x, y0 + y);
        setPixelRed(imgBMP, x0 + x, y0 - y);
        setPixelRed(imgBMP, x0 - x, y0 - y);
        setPixelRed(imgBMP, x0 + y, y0 + x);
        setPixelRed(imgBMP, x0 - y, y0 + x);
        setPixelRed(imgBMP, x0 + y, y0 - x);
        setPixelRed(imgBMP, x0 - y, y0 - x);
    }
}


void draw_red_orientation(BMP_IMG* imgBMP, int x, int y, float ori, int cR)
{
    int xe = (int) (x + cos(ori)*cR), ye = (int) (y + sin(ori)*cR);
    // Bresenham's line algorithm
    int dx =  abs(xe-x), sx = x<xe ? 1 : -1;
    int dy = -abs(ye-y), sy = y<ye ? 1 : -1;
    int err = dx+dy, e2; /* error value e_xy */

    for(;;)
    {  /* loop */
        //setPixelRed(imgBMP, x, y);
        setPixelRed(imgBMP, y, x);
        if (x==xe && y==ye) break;
        e2 = 2*err;
        if (e2 >= dy) { err += dy; x += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y += sy; } /* e_xy+e_y < 0 */
    }
}// draw_red_orientation()

void write_ppm(const char* filename, unsigned char* data, int w, int h)
{
    FILE* fp;
    if ((fp = fopen(filename, "wb")) == NULL) {
        printf("Cannot write to file %s\n", filename);
        return;
    }
    //printf("Saving %s...", aFilename);
    /* Write header */
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", w, h);
    fprintf(fp, "255\n");

    fwrite(data, sizeof(unsigned char), w*h*3, fp);
    fclose(fp);
    //printf("Done.\n");
}


void write_rgb2ppm(const char * filename, unsigned char* r, unsigned char* g, unsigned char* b, int w, int h)
{
    FILE * out_file;
    int i;

    unsigned char* obuf = (unsigned char*)malloc(3*w*h*sizeof(unsigned char));

    for(i = 0; i < w*h; i ++){
        obuf[3*i + 0] = r[i];
        obuf[3*i + 1] = g[i];
        obuf[3*i + 2] = b[i];
    }
    out_file = fopen(filename, "wb");
    fprintf(out_file, "P6\n");
    fprintf(out_file, "%d %d\n255\n", w, h);
    fwrite(obuf,sizeof(unsigned char), 3*w*h, out_file);
    fclose(out_file);
    free(obuf);
}// write_ppm()


void write_bmp(const char* filename, unsigned char* data, int w, int h)
{
	
	//颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0  
	int colorTablesize = 0;
	int biBitCount = 24;//彩色图
	//待存储图像数据每行字节数为4的倍数  
	int lineByte = (w* biBitCount / 8 + 3) / 4 * 4;
	//以二进制写的方式打开文件  
	FILE *fp = fopen(filename, "wb");
	if (fp == 0)
	{
		printf("Cannot write to file %s\n", filename);
		system("pause");
		return;
	}
	//申请位图文件头结构变量，填写文件头信息  
	BITMAPFILEHEADER fileHead;
	fileHead.bfType = 0x4D42;//bmp类型  
	//bfSize是图像文件4个组成部分之和  
	fileHead.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)
		+colorTablesize + lineByte*h;
	fileHead.bfReserved1 = 0;
	fileHead.bfReserved2 = 0;
	//bfOffBits是图像文件前3个部分所需空间之和  
	fileHead.bfOffBits = 54 + colorTablesize;
	//写文件头进文件  
	fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);
	//申请位图信息头结构变量，填写信息头信息  
	BITMAPINFOHEADER head;
	head.biBitCount = biBitCount;
	head.biClrImportant = 0;
	head.biClrUsed = 0;
	head.biCompression = 0;
	head.biHeight = h;
	head.biPlanes = 1;
	head.biSize = 40;
	head.biSizeImage = lineByte*h;
	head.biWidth = w;
	head.biXPelsPerMeter = 0;
	head.biYPelsPerMeter = 0;
	//写位图信息头进内存  
	fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp);
	//写位图数据进文件  
	fwrite(data, h*lineByte, 1, fp);
	//关闭文件  
	fclose(fp);
	return;
}


void write_rgb2bmp(const char * filename, unsigned char* r, unsigned char* g, unsigned char* b, int w, int h)
{
	//如果位图数据指针为0，则没有数据传入，函数返回  
	if (!r || !g || !b)
		return;
	unsigned char* data = (unsigned char*)malloc(3 * w*h*sizeof(unsigned char));
	//int totalLength = w*h;
	//由于bmp特殊存储方式，需要上下翻转图像
	for (int i = 0; i < h / 2; ++i)
	for (int j = 0; j < w; ++j)
	{
		unsigned char temp = 0;
		temp = r[i*w + j];
		r[i*w + j] = r[(h - 1 - i)*w + j];
		r[(h - 1 - i)*w + j] = temp;
		temp = g[i*w + j];
		g[i*w + j] = g[(h - 1 - i)*w + j];
		g[(h - 1 - i)*w + j] = temp;
		temp = b[i*w + j];
		b[i*w + j] = b[(h - 1 - i)*w + j];
		b[(h - 1 - i)*w + j] = temp;
	}
	//存入新数组
	for (int i = 0; i < w*h; i++){
		data[3 * i + 0] = r[i];
		data[3 * i + 1] = g[i];
		data[3 * i + 2] = b[i];
	}


	//颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0  
	int colorTablesize = 0;
	int biBitCount = 24;//彩色图
	//待存储图像数据每行字节数为4的倍数  
	int lineByte = (w* biBitCount / 8 + 3) / 4 * 4;
	//以二进制写的方式打开文件  
	FILE *fp = fopen(filename, "wb");
	if (fp == 0)
	{
		printf("can't write file:%s", filename);
		system("pause");
		return;
	}
	//申请位图文件头结构变量，填写文件头信息  
	BITMAPFILEHEADER fileHead;
	fileHead.bfType = 0x4D42;//bmp类型  
	//bfSize是图像文件4个组成部分之和  
	fileHead.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)
		+colorTablesize + lineByte*h;
	fileHead.bfReserved1 = 0;
	fileHead.bfReserved2 = 0;
	//bfOffBits是图像文件前3个部分所需空间之和  
	fileHead.bfOffBits = 54 + colorTablesize;
	//写文件头进文件  
	fwrite(&fileHead, sizeof(BITMAPFILEHEADER), 1, fp);
	//申请位图信息头结构变量，填写信息头信息  
	BITMAPINFOHEADER head;
	head.biBitCount = biBitCount;
	head.biClrImportant = 0;
	head.biClrUsed = 0;
	head.biCompression = 0;
	head.biHeight = h;
	head.biPlanes = 1;
	head.biSize = 40;
	head.biSizeImage = lineByte*h;
	head.biWidth = w;
	head.biXPelsPerMeter = 0;
	head.biYPelsPerMeter = 0;
	//写位图信息头进内存  
	fwrite(&head, sizeof(BITMAPINFOHEADER), 1, fp);
	//写位图数据进文件  
	fwrite(data, h*lineByte, 1, fp);
	//关闭文件  
	fclose(fp);
	return;
}