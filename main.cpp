//
// Created by huangyingfan on 5/22/18.
//
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "sift.h"

using namespace std;

int main(int argc, char ** argv)
{
    char *file1 = "../1.bmp";
    char *file2 = "../2.bmp";
    // Read two input images 读入图片
    ImageObj_uch image1, image2;

    if(read_bmp(file1, image1.data, image1.w, image1.h)!=0)
    {
        printf("Failed to open input image!\n");
        return -1;
    }

    if(read_bmp(file2, image2.data, image2.w, image2.h)!=0)
    {
        printf("Failed to open input image!\n");
        return -1;
    }
    printf("Image 1 loaded. Image size: %d x %d\n", image1.w, image1.h);
    printf("Image 2 loaded. Image size: %d x %d\n", image2.w, image2.h);

    // Double the original image as the first octive.构造 层
    double_original_image(true);
    // Detect keypoints检测关键点
    Vector *kpt_list1 = (Vector *)malloc(sizeof(Vector));
    Vector *kpt_list2 = (Vector *)malloc(sizeof(Vector));
    vector_setup(kpt_list1,1024,sizeof(SiftKeypoint));
    vector_setup(kpt_list2,1024,sizeof(SiftKeypoint));

    printf("\nSIFT detection on image 1 ...\n");
    sift(image1, kpt_list1, true);
    printf("# keypoints in image1: %d\n", (*kpt_list1).size);

    printf("\nSIFT detection on image 2 ...\n");
    sift(image2, kpt_list2, true);
    printf("# keypoints in image2: %d\n", (*kpt_list2).size);
    // above code have no bug


    // Save keypoint list, and draw keypoints on images.保存关键点为链表并画圈在图片上
    char filename[255];
    sprintf(filename, "s_A_keypoints.bmp");
    draw_keypoints_to_bmp_file(filename, image1, kpt_list1);
    export_kpt_list_to_file("s_A_keypoints.key", kpt_list1, true);

    sprintf(filename, "s_B_keypoints.bmp");
    draw_keypoints_to_bmp_file(filename, image2, kpt_list2);
    export_kpt_list_to_file("s_B_keypoints.key", kpt_list2, true);

    // Match keypoints. 匹配两张图片的画出的特征点
    Vector match_list =  match_keypoints(kpt_list1, kpt_list2);

    // Draw result image.画匹配结果图
    sprintf(filename, "s_A_B_matching.bmp");
    draw_match_lines_to_bmp_file(filename, image1, image2, match_list);
    printf("# of matched keypoints: %d\n", (match_list).size);
	system("pause");
	return 0;
}