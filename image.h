/*----------------------------------
updated by wonderly321 on 4/11/19.
-----------------------------------*/

#ifndef SIFT_IMAGE_H
#define SIFT_IMAGE_H

typedef struct{
    int w;
    int h;
    unsigned char *data;
}ImageObj_uch; //unsigned char image

typedef struct{
    int w;
    int h;
    float *data;
}ImageObj_float;

void init_uch_imageobj(ImageObj_uch *image_obj ,int _w, int _h);


void init_float_imageobj(ImageObj_float *image_obj ,int _w, int _h);

ImageObj_uch upsample_2x(ImageObj_uch image_obj);

ImageObj_float floatimg_downsample_2x(ImageObj_float image_obj);

ImageObj_uch  uchimg_downsample_2x(ImageObj_uch image_obj);


ImageObj_float unc_to_float(ImageObj_uch image_obj);

#endif //SIFT_IMAGE_H
