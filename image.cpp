/*----------------------------------
  updated by wonderly321 on 4/11/19.
-----------------------------------*/


#include "image.h"

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>

#include "common.h"
#include "image_io.h"

void init_uch_imageobj(ImageObj_uch *image_obj ,int _w, int _h)
{
    image_obj->w = _w;
    image_obj->h = _h;
    // if (data) delete [] data;
    image_obj->data = (unsigned char*)malloc(_w * _h);
}

void init_float_imageobj(ImageObj_float *image_obj ,int _w, int _h)
{
    image_obj->w = _w;
    image_obj->h = _h;
    // if (data) delete [] data;
    image_obj->data  = (float*)malloc(_w * _h * sizeof(float));
}

ImageObj_uch upsample_2x(ImageObj_uch image_obj)
{
    float scale = 2.0f;

    int srcW = image_obj.w, srcH = image_obj.h;
    int dstW = srcW << 1, dstH = srcH << 1;
    ImageObj_uch  *out_image = (ImageObj_uch *)malloc(sizeof(ImageObj_uch));
    init_uch_imageobj( out_image,dstW, dstH);

    unsigned char *srcData = image_obj.data;
    unsigned char *dstData = out_image->data;

    for (int r = 0; r < dstH; r++)
    {
        for (int c = 0; c < dstW; c++)
        {
            float ori_r = r / scale;
            float ori_c = c / scale;
            int r1 = (int)ori_r;
            int c1 = (int)ori_c;
            float dr = ori_r - r1;
            float dc = ori_c - c1;

            int idx = r1 * srcW + c1;
            dstData[r * dstW + c] =
                    (unsigned char)((1 - dr) * (1 - dc) * srcData[idx] +
                                    dr * (1 - dc) *
                                    (r1 < srcH - 1 ? srcData[idx + srcW]
                                                   : srcData[idx]) +
                                    (1 - dr) * dc *
                                    (c1 < srcW - 1 ? srcData[idx + 1]
                                                   : srcData[idx]) +
                                    dr * dc *
                                    ((c1 < srcW - 1 && r1 < srcH - 1)
                                     ? srcData[idx + srcW + 1]
                                     : srcData[idx]));
        }
    }
    return *out_image;
}

ImageObj_float floatimg_downsample_2x(ImageObj_float image_obj) {
    int srcW = image_obj.w, srcH = image_obj.h;
    int dstW = srcW >> 1, dstH = srcH >> 1;
    ImageObj_float out_image;
    init_float_imageobj(&out_image,dstW, dstH);

    float *srcData = image_obj.data;
    float *dstData = out_image.data;

    for (int r = 0; r < dstH; r++)
    {
        for (int c = 0; c < dstW; c++)
        {
            int ori_r = r << 1;
            int ori_c = c << 1;
            dstData[r * dstW + c] = srcData[ori_r * srcW + ori_c];
        }
    }
    return out_image;
}

ImageObj_uch  uchimg_downsample_2x(ImageObj_uch image_obj) {
    int srcW = image_obj.w, srcH = image_obj.h;
    int dstW = srcW >> 1, dstH = srcH >> 1;
    ImageObj_uch *out_image = (ImageObj_uch *)malloc(sizeof(ImageObj_uch));
    init_uch_imageobj(out_image,dstW, dstH);

    unsigned char *srcData = image_obj.data;
    unsigned char *dstData = out_image->data;

    for (int r = 0; r < dstH; r++)
    {
        for (int c = 0; c < dstW; c++)
        {
            int ori_r = r << 1;
            int ori_c = c << 1;
            dstData[r * dstW + c] = srcData[ori_r * srcW + ori_c];
        }
    }
    return *out_image;
}

ImageObj_float unc_to_float(ImageObj_uch image_obj)
{
    ImageObj_float dstImage;
    init_float_imageobj(&dstImage,image_obj.w,image_obj.h);
    int w = image_obj.w;
    int h = image_obj.h;
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            dstImage.data[i * w + j] = (float)image_obj.data[i * w + j];
        }
    }
    return dstImage;
}

