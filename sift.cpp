/*----------------------------------
updated by wonderly321 on 4/11/19.
-----------------------------------*/

#include "sift.h"
#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <float.h>
#include <fstream>
#include "matrix.h"
#include "common.h"
#include "image_io.h"
#include "image.h"

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

void double_original_image(bool doubleFirstOctave)
{
	SIFT_IMG_DBL = doubleFirstOctave;
	return;
}

int build_octaves(
	ImageObj_uch image,
	Vector *octaves,
	int firstOctave,
	int nOctaves)
{
	int w = image.w;
	int h = image.h;
	if (firstOctave == -1)
	{
		w = image.w * 2;
		h = image.h * 2;
	}

	for (int i = 0; i < nOctaves; i++)
	{
		if (i == 0 && firstOctave == -1)
		{
			ImageObj_uch tmp_uch = upsample_2x(image);
			vector_push_back_imageuch(octaves, &tmp_uch);

		}
		else if ((i == 0 && firstOctave != -1) || (i == 1 && firstOctave == -1)) {
			vector_push_back_imageuch(octaves, &image);

		}
		else {
			ImageObj_uch tmp2 = uchimg_downsample_2x(*(ImageObj_uch *)vector_get_imageuch(octaves, i - 1));
			vector_push_back_imageuch(octaves, &tmp2);
		}
		w = w / 2;
		h = h / 2;
	}
	return 0;
}


// Improved Gaussian Blurring Function
int gaussian_blur(
	ImageObj_float & in_image,
	ImageObj_float & out_image,
	Vector coef1d)
{
	int w = in_image.w;
	int h = in_image.h;
	int gR = coef1d.size / 2;

	ImageObj_float img_t;
	init_float_imageobj(&img_t, h, w);
	row_filter_transpose(in_image.data, img_t.data, w, h, (float *)vector_get_float(&coef1d, 0), gR);
	row_filter_transpose(img_t.data, out_image.data, h, w, (float *)vector_get_float(&coef1d, 0), gR);
	return 0;
}

// Apply Gaussian row filter to image, then transpose the image.
//bug
int row_filter_transpose(
	float * src,
	float * dst,
	int w, int h,
	float * coef1d, int gR)
{
	float * row_buf = new float[w + gR * 2];
	float * row_start;
	int elemSize = sizeof(float);

	float * srcData = src;
	float * dstData = dst + w * h - 1;
	float partialSum = 0.0f;
	float * coef = coef1d;
	float * prow;

	float firstData, lastData;
	for (int r = 0; r < h; r++)
	{
		row_start = srcData + r * w;
		memcpy(row_buf + gR, row_start, elemSize * w);
		firstData = *(row_start);
		lastData = *(row_start + w - 1);
		for (int i = 0; i < gR; i++)
		{
			row_buf[i] = firstData;
			row_buf[i + w + gR] = lastData;
		}

		prow = row_buf;
		dstData = dstData - w * h + 1;
		for (int c = 0; c < w; c++)
		{
			partialSum = 0.0f;
			coef = coef1d;

			for (int i = -gR; i <= gR; i++)
			{
				partialSum += (*coef++) * (*prow++);
			}

			prow -= 2 * gR;
			*dstData = partialSum;
			dstData += h;
		}
	}
	//printf("test/n");
	delete[] row_buf;

	return 0;
}


// Build Gaussian pyramid using recursive method.
// The first layer is downsampled from last octave, layer=nLayers.
// All n-th layer is Gaussian blur from (n-1)-th layer.
int build_gaussian_pyramid(
	Vector *octaves,
	Vector *gpyr,
	int nOctaves,
	int nGpyrLayers)
{
	int nLayers = nGpyrLayers - 3;
	Vector gaussian_coefs;

	compute_gaussian_coefs(&gaussian_coefs, nGpyrLayers);

	int w, h;
	for (int i = 0; i < nOctaves; i++)
	{
		w = (*(ImageObj_uch *)vector_get_imageuch(octaves, i)).w;
		h = (*(ImageObj_uch *)vector_get_imageuch(octaves, i)).h;
		for (int j = 0; j < nGpyrLayers; j++)
		{
			if (i == 0 && j == 0)
			{
				init_float_imageobj(&(*(ImageObj_float *)vector_get_imagefloat(gpyr, 0)), w, h);
				ImageObj_float tmp = unc_to_float(*(ImageObj_uch*)vector_get_imageuch(octaves, 0));
				gaussian_blur(tmp, *(ImageObj_float *)vector_get_imagefloat(gpyr, 0), *(Vector *)vector_get_imagefloat(&gaussian_coefs, j));

			}
			else if (i > 0 && j == 0) {
				*(ImageObj_float *)vector_get_imagefloat(gpyr, i * nGpyrLayers) =
					floatimg_downsample_2x(*(ImageObj_float *)vector_get_imagefloat(gpyr, (i - 1) * nGpyrLayers + nLayers));
			}
			else {
				init_float_imageobj((ImageObj_float *)vector_get_imagefloat(gpyr, i * nGpyrLayers + j), w, h);


				ImageObj_float tmp1 = *(ImageObj_float *)vector_get_imagefloat(gpyr, i * nGpyrLayers + j - 1);
				ImageObj_float tmp2 = *(ImageObj_float *)vector_get_imagefloat(gpyr, i * nGpyrLayers + j);
				Vector tmp3 = *(Vector *)vector_get_float(&gaussian_coefs, j);
				gaussian_blur(tmp1, tmp2, tmp3);
			}
		}
	}
	// Release octaves memory.
	vector_clear(octaves);
	return 0;
}

// For build_gaussian_pyramid()
int compute_gaussian_coefs(
	Vector *gaussian_coefs,
	int nGpyrLayers)
{
	// Compute all sigmas for different layers
	int nLayers = nGpyrLayers - 3;
	float sigma, sigma_pre;
	float sigma0 = SIFT_SIGMA;
	float k = powf(2.0f, 1.0f / nLayers);

	Vector sig;
	vector_setup(&sig, nGpyrLayers, sizeof(float));

	sigma_pre = SIFT_IMG_DBL ? 2.0f * SIFT_INIT_SIGMA : SIFT_INIT_SIGMA;
	vector_init_float(&sig);
	float tmp_sig = sqrtf(sigma0 * sigma0 - sigma_pre * sigma_pre);
	vector_assign_float(&sig, 0, &tmp_sig);
	for (int i = 1; i < nGpyrLayers; i++)
	{
		sigma_pre = powf(k, (float)(i - 1)) * sigma0;
		sigma = sigma_pre * k;
		float tmp_sig2 = sqrtf(sigma * sigma - sigma_pre * sigma_pre);
		vector_assign_float(&sig, i, &tmp_sig2);
	}

	//Vector gaussian_coefs;
	vector_setup(gaussian_coefs, nGpyrLayers, sizeof(sig));
	vector_init_float2(gaussian_coefs);

	for (int i = 0; i < nGpyrLayers; i++)
	{
		// Compute Gaussian filter coefficients
		float factor = SIFT_GAUSSIAN_FILTER_RADIUS;
		int gR = (*(float *)vector_get_float(&sig, i) * factor > 1.0f) ? (int)ceilf(*(float *)vector_get_float(&sig, i) * factor) : 1;
		int gW = gR * 2 + 1;

		vector_resize((Vector *)vector_get_float(gaussian_coefs, i), gW);
		float accu = 0.0f;
		float tmp;
		for (int j = 0; j < gW; j++)
		{
			tmp = (float)((j - gR) / *(float *)vector_get_float(&sig, i));
			*(float *)vector_get_float((Vector *)vector_get_float(gaussian_coefs, i), j) = expf(tmp * tmp * -0.5f) * (1 + j / 1000.0f);
			accu += *(float *)vector_get_float((Vector *)vector_get_float(gaussian_coefs, i), j);
		}

		for (int j = 0; j < gW; j++)
		{
			float tmp_ij = *(float *)vector_get_float((Vector *)vector_get_float(gaussian_coefs, i), j) / accu;
			*(float *)vector_get_float((Vector *)vector_get_float(gaussian_coefs, i), j) = tmp_ij;

		} // End compute Gaussian filter coefs
	}
	return 0;
}

// Build difference of Gaussian pyramids.
int build_dog_pyr(
	Vector & gpyr,
	Vector & dogPyr,
	int nOctaves,
	int nDogLayers)
{
	int nGpyrLayers = nDogLayers + 1;

	int w, h;
	float * srcData1; // always data2-data1.
	float * srcData2;
	float * dstData;
	int index = 0;

	for (int i = 0; i < nOctaves; i++)
	{
		int row_start = i * nGpyrLayers;
		w = (*(ImageObj_float *)vector_get_imagefloat(&gpyr, row_start)).w;
		h = (*(ImageObj_float *)vector_get_imagefloat(&gpyr, row_start)).h;

		for (int j = 0; j < nDogLayers; j++)
		{
			init_float_imageobj(((ImageObj_float *)vector_get_imagefloat(&dogPyr, i * nDogLayers + j)), w, h);
			dstData = (*(ImageObj_float *)vector_get_imagefloat(&dogPyr, i * nDogLayers + j)).data;

			srcData1 = (*(ImageObj_float *)vector_get_imagefloat(&gpyr, row_start + j)).data;
			srcData2 = (*(ImageObj_float *)vector_get_imagefloat(&gpyr, row_start + j + 1)).data;

			index = 0;
			while (index++ < w * h)
				*(dstData++) = *(srcData2++) - *(srcData1++);
		}
	}
	return 0;
}

// Build gradient pyramids.
int build_grd_rot_pyr(
	Vector & gpyr,
	Vector & grdPyr,
	Vector & rotPyr,
	int nOctaves,
	int nLayers)
{
	int nGpyrLayers = nLayers + 3;
	int w, h;
	float dr, dc;
	float angle;


	for (int i = 0; i < nOctaves; i++)
	{
		// use gradient information from layers 1~n Layers.
		w = (*(ImageObj_float *)vector_get_imagefloat(&gpyr, i * nGpyrLayers)).w;
		h = (*(ImageObj_float *)vector_get_imagefloat(&gpyr, i * nGpyrLayers)).h;

		for (int j = 1; j <= nLayers; j++)
		{
			float * srcData;
			float * grdData;
			float * rotData;

			int layer_index = i * nGpyrLayers + j;
			init_float_imageobj((ImageObj_float *)vector_get_imagefloat(&grdPyr, layer_index), w, h);
			init_float_imageobj((ImageObj_float *)vector_get_imagefloat(&rotPyr, layer_index), w, h);

			srcData = (*(ImageObj_float *)vector_get_imagefloat(&gpyr, layer_index)).data;
			grdData = (*(ImageObj_float *)vector_get_imagefloat(&grdPyr, layer_index)).data;
			rotData = (*(ImageObj_float *)vector_get_imagefloat(&rotPyr, layer_index)).data;

			for (int r = 0; r < h; r++)
			{
				for (int c = 0; c < w; c++)
				{
					dr = get_pixel_f(srcData, w, h, r + 1, c) - get_pixel_f(srcData, w, h, r - 1, c);
					dc = get_pixel_f(srcData, w, h, r, c + 1) - get_pixel_f(srcData, w, h, r, c - 1);

#if (USE_FAST_FUNC == 1)
					grdData[r * w + c] = fast_sqrt_f(dr * dr + dc * dc);

					angle = fast_atan2_f(dr, dc); //atan2f(dr, dc + FLT_MIN);
#else
					grdData[r * w + c] = sqrtf(dr * dr + dc * dc);
					angle = atan2f(dr, dc + FLT_MIN);
					angle = angle < 0 ? angle + _2PI : angle;
#endif
					rotData[r * w + c] = angle;
				}
			}
		}
	}
	return 0;
}

// Compute orientation histogram for keypoint detection.
// using pre-computed gradient information.
float compute_orientation_hist_with_gradient(
	ImageObj_float & grdImage,
	ImageObj_float & rotImage,
	SiftKeypoint & kpt,
	float * & hist)
{
	int nBins = SIFT_ORI_HIST_BINS;
	int octave = kpt.octave;
	int layer = kpt.layer;

	float kptr = kpt.ri;
	float kptc = kpt.ci;
	float kpt_scale = kpt.layer_scale;

	int kptr_i = (int)(kptr + 0.5f);
	int kptc_i = (int)(kptc + 0.5f);
	float d_kptr = kptr - kptr_i;
	float d_kptc = kptc - kptc_i;

	float sigma = SIFT_ORI_SIG_FCTR * kpt_scale;
	int win_radius = (int)(SIFT_ORI_RADIUS * kpt_scale);
	int win_width = win_radius * 2 + 1;
	float exp_factor = -1.0f / (2.0f * sigma * sigma);

	float * grdData = grdImage.data;
	float * rotData = rotImage.data;
	int w = grdImage.w;
	int h = grdImage.h;

	int r, c;
	float magni, angle, weight;
	int bin;
	float fbin; // float point bin

	float *tmpHist = new float[nBins];
	memset(tmpHist, 0, nBins * sizeof(float));

	for (int i = -win_radius; i <= win_radius; i++) // rows
	{
		r = kptr_i + i;
		if (r <= 0 || r >= h - 1) // Cannot calculate dy
			continue;
		for (int j = -win_radius; j <= win_radius; j++) // columns
		{
			c = kptc_i + j;
			if (c <= 0 || c >= w - 1)
				continue;

			magni = grdData[r * w + c];
			angle = rotData[r * w + c];

			fbin = angle * nBins / _2PI;
			weight = expf(((i - d_kptr) * (i - d_kptr) + (j - d_kptc) * (j - d_kptc)) * exp_factor);

#define SIFT_ORI_BILINEAR
#ifdef SIFT_ORI_BILINEAR
			bin = (int)(fbin - 0.5f);
			float d_fbin = fbin - 0.5f - bin;

			float mw = weight * magni;
			float dmw = d_fbin * mw;
			tmpHist[(bin + nBins) % nBins] += mw - dmw;
			tmpHist[(bin + 1) % nBins] += dmw;
#else
			bin = (int)(fbin);
			tmpHist[bin] += magni * weight;
#endif
		}
	}

#define TMPHIST(idx) (idx < 0? tmpHist[0] : (idx >= nBins ? tmpHist[nBins - 1] : tmpHist[idx]))

#define USE_SMOOTH1	1
#if		USE_SMOOTH1

	// Smooth the histogram. Algorithm comes from OpenCV.
	hist[0] = (tmpHist[0] + tmpHist[2]) * 1.0f / 16.0f +
		(tmpHist[0] + tmpHist[1]) * 4.0f / 16.0f +
		tmpHist[0] * 6.0f / 16.0f;
	hist[1] = (tmpHist[0] + tmpHist[3]) * 1.0f / 16.0f +
		(tmpHist[0] + tmpHist[2]) * 4.0f / 16.0f +
		tmpHist[1] * 6.0f / 16.0f;
	hist[nBins - 2] = (tmpHist[nBins - 4] + tmpHist[nBins - 1]) * 1.0f / 16.0f +
		(tmpHist[nBins - 3] + tmpHist[nBins - 1]) * 4.0f / 16.0f +
		tmpHist[nBins - 2] * 6.0f / 16.0f;
	hist[nBins - 1] = (tmpHist[nBins - 3] + tmpHist[nBins - 1]) * 1.0f / 16.0f +
		(tmpHist[nBins - 2] + tmpHist[nBins - 1]) * 4.0f / 16.0f +
		tmpHist[nBins - 1] * 6.0f / 16.0f;

	for (int i = 2; i < nBins - 2; i++)
	{
		hist[i] = (tmpHist[i - 2] + tmpHist[i + 2]) * 1.0f / 16.0f +
			(tmpHist[i - 1] + tmpHist[i + 1]) * 4.0f / 16.0f +
			tmpHist[i] * 6.0f / 16.0f;
	}

#else
	// Yet another smooth function
	// Algorithm comes from the vl_feat implementation.
	for (int iter = 0; iter < 6; iter++)
	{
		float prev = TMPHIST(nBins - 1);
		float first = TMPHIST(0);
		int i;
		for (i = 0; i < nBins - 1; i++)
		{
			float newh = (prev + TMPHIST(i) + TMPHIST(i + 1)) / 3.0f;
			prev = hist[i];
			hist[i] = newh;
		}
		hist[i] = (prev + hist[i] + first) / 3.0f;
	}
#endif

	// Find the maximum item of the histogram
	float maxitem = hist[0];
	int max_i = 0;
	for (int i = 0; i < nBins; i++)
	{
		if (maxitem < hist[i])
		{
			maxitem = hist[i];
			max_i = i;
		}
	}

	kpt.ori = max_i * _2PI / nBins;

	delete[] tmpHist;
	return maxitem;
}


// Keypoint detection.
int detect_keypoints(
	Vector  dogPyr,
	Vector  grdPyr,
	Vector  rotPyr,
	int nOctaves,
	int nDogLayers,
	Vector *kpt_list)
{
	float * currData = NULL;
	float * lowData = NULL;
	float * highData = NULL;

	int w, h;
	int layer_index;
	int index;
	float val;

	int nBins = SIFT_ORI_HIST_BINS;
	float * hist = new float[nBins];
	int nGpyrLayers = nDogLayers + 1;

	//  |D(x)|<0.03 will be rejected.
	float contr_thr = 0.8f * SIFT_CONTR_THR;
	for (int i = 0; i < nOctaves; i++)
	{
		w = ((ImageObj_float *)vector_get_imagefloat(&dogPyr, i * nDogLayers))->w;
		h = ((ImageObj_float *)vector_get_imagefloat(&dogPyr, i * nDogLayers))->h;

		for (int j = 1; j < nDogLayers - 1; j++)
		{
			layer_index = i * nDogLayers + j;

			highData = ((ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_index + 1))->data;
			currData = ((ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_index))->data;
			lowData = ((ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_index - 1))->data;

			for (int r = SIFT_IMG_BORDER; r < h - SIFT_IMG_BORDER; r++)
			{
				for (int c = SIFT_IMG_BORDER; c < w - SIFT_IMG_BORDER; c++)
				{
					SiftKeypoint *kpt = (SiftKeypoint *)malloc(sizeof(SiftKeypoint));
					index = r * w + c;
					val = currData[index];

					bool bExtrema =
						(val >= contr_thr &&
							val > highData[index - w - 1] && val > highData[index - w] && val > highData[index - w + 1] &&
							val > highData[index - 1] && val > highData[index] && val > highData[index + 1] &&
							val > highData[index + w - 1] && val > highData[index + w] && val > highData[index + w + 1] &&
							val > currData[index - w - 1] && val > currData[index - w] && val > currData[index - w + 1] &&
							val > currData[index - 1] && val > currData[index + 1] &&
							val > currData[index + w - 1] && val > currData[index + w] && val > currData[index + w + 1] &&
							val > lowData[index - w - 1] && val > lowData[index - w] && val > lowData[index - w + 1] &&
							val > lowData[index - 1] && val > lowData[index] && val > lowData[index + 1] &&
							val > lowData[index + w - 1] && val > lowData[index + w] && val > lowData[index + w + 1])
						|| // Local min
						(
							val <= -contr_thr &&
							val < highData[index - w - 1] && val < highData[index - w] && val < highData[index - w + 1] &&
							val < highData[index - 1] && val < highData[index] && val < highData[index + 1] &&
							val < highData[index + w - 1] && val < highData[index + w] && val < highData[index + w + 1] &&
							val < currData[index - w - 1] && val < currData[index - w] && val < currData[index - w + 1] &&
							val < currData[index - 1] && val < currData[index + 1] &&
							val < currData[index + w - 1] && val < currData[index + w] && val < currData[index + w + 1] &&
							val < lowData[index - w - 1] && val < lowData[index - w] && val < lowData[index - w + 1] &&
							val < lowData[index - 1] && val < lowData[index] && val < lowData[index + 1] &&
							val < lowData[index + w - 1] && val < lowData[index + w] && val < lowData[index + w + 1]);
					if (bExtrema)
					{
						kpt->octave = i;
						kpt->layer = j;
						kpt->ri = (float)r;
						kpt->ci = (float)c;

						bool bGoodKeypoint = refine_local_extrema(dogPyr, nOctaves, nDogLayers, *kpt);
						if (!bGoodKeypoint)
							continue;

						ImageObj_float *tmp1;
						tmp1 = (ImageObj_float *)vector_get_imagefloat(&grdPyr, i * nGpyrLayers + kpt->layer);
						ImageObj_float *tmp2;
						tmp2 = (ImageObj_float *)vector_get_imagefloat(&rotPyr, i * nGpyrLayers + kpt->layer);

						float max_mag = compute_orientation_hist_with_gradient(*tmp1, *tmp2, *kpt, hist);
						float threshold = max_mag * SIFT_ORI_PEAK_RATIO;

						for (int ii = 0; ii < nBins; ii++)
						{
#define INTERPOLATE_ORI_HIST

#ifndef INTERPOLATE_ORI_HIST
							if (hist[ii] >= threshold)
							{
								kpt.mag = hist[ii];
								kpt.ori = ii * _2PI / nBins;
								kpt_list.push_back(kpt);
							}
#else
							// Use 3 points to fit a curve and find the accurate location of a keypoints
							int left = ii > 0 ? ii - 1 : nBins - 1;
							int right = ii < (nBins - 1) ? ii + 1 : 0;
							float currHist = hist[ii];
							float lhist = hist[left];
							float rhist = hist[right];
							if (currHist > lhist && currHist > rhist && currHist > threshold)
							{
								float accu_ii = ii + 0.5f * (lhist - rhist) / (lhist - 2.0f*currHist + rhist);
								// Since bin index means the starting point of a bin, so the real orientation should be bin index
								// plus 0.5. for example, angles in bin 0 should have a mean value of 5 instead of 0;
								accu_ii += 0.5f;
								accu_ii = accu_ii < 0 ? (accu_ii + nBins) : accu_ii >= nBins ? (accu_ii - nBins) : accu_ii;
								// The magnitude should also calculate the max number based on fitting
								// But since we didn't actually use it in image matching, we just lazily
								// use the histogram value.
								kpt->mag = currHist;
								kpt->ori = accu_ii * _2PI / nBins;
								vector_push_back_keypoint(kpt_list, kpt);
							}
#endif
						}
					}
					free(kpt);
				}
			}
		}
	}
	delete[] hist;
	return 0;
}

// Refine local keypoint extrema.
bool refine_local_extrema(Vector & dogPyr,
	int nOctaves,
	int nDogLayers,
	SiftKeypoint & kpt)
{
	int nGpyrLayers = nDogLayers + 1;

	int w, h;
	int layer_idx;
	int octave = kpt.octave;
	int layer = kpt.layer;
	int r = (int)kpt.ri;
	int c = (int)kpt.ci;

	float * currData;
	float * lowData;
	float * highData;

	int xs_i = 0, xr_i = 0, xc_i = 0;
	float tmp_r, tmp_c, tmp_layer;
	float xr = 0.0f, xc = 0.0f, xs = 0.0f;
	float x_hat[3] = { xc, xr, xs };
	float dx, dy, ds;
	float dxx, dyy, dss, dxs, dys, dxy;

	tmp_r = (float)r;
	tmp_c = (float)c;
	tmp_layer = (float)layer;

	int i = 0;
	for (; i < SIFT_MAX_INTERP_STEPS; i++)
	{
		c += xc_i;
		r += xr_i;

		layer_idx = octave * nDogLayers + layer;

		w = (*(ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_idx)).w;
		h = (*(ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_idx)).h;
		currData = (*(ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_idx)).data;
		lowData = (*(ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_idx - 1)).data;
		highData = (*(ImageObj_float *)vector_get_imagefloat(&dogPyr, layer_idx + 1)).data;

		dx = (get_pixel_f(currData, w, h, r, c + 1) - get_pixel_f(currData, w, h, r, c - 1)) * 0.5f;
		dy = (get_pixel_f(currData, w, h, r + 1, c) - get_pixel_f(currData, w, h, r - 1, c)) * 0.5f;
		ds = (get_pixel_f(highData, w, h, r, c) - get_pixel_f(lowData, w, h, r, c)) * 0.5f;
		float dD[3] = { -dx, -dy, -ds };

		float v2 = 2.0f * get_pixel_f(currData, w, h, r, c);
		dxx = (get_pixel_f(currData, w, h, r, c + 1) + get_pixel_f(currData, w, h, r, c - 1) - v2);
		dyy = (get_pixel_f(currData, w, h, r + 1, c) + get_pixel_f(currData, w, h, r - 1, c) - v2);
		dss = (get_pixel_f(highData, w, h, r, c) + get_pixel_f(lowData, w, h, r, c) - v2);
		dxy = (get_pixel_f(currData, w, h, r + 1, c + 1) - get_pixel_f(currData, w, h, r + 1, c - 1) -
			get_pixel_f(currData, w, h, r - 1, c + 1) + get_pixel_f(currData, w, h, r - 1, c - 1)) * 0.25f;
		dxs = (get_pixel_f(highData, w, h, r, c + 1) - get_pixel_f(highData, w, h, r, c - 1) -
			get_pixel_f(lowData, w, h, r, c + 1) + get_pixel_f(lowData, w, h, r, c - 1)) * 0.25f;
		dys = (get_pixel_f(highData, w, h, r + 1, c) - get_pixel_f(highData, w, h, r - 1, c) -
			get_pixel_f(lowData, w, h, r + 1, c) + get_pixel_f(lowData, w, h, r - 1, c)) * 0.25f;

		// The scale in two sides of the equation should cancel each other.
		float H[3][3] = { { dxx, dxy, dxs },
		{ dxy, dyy, dys },
		{ dxs, dys, dss } };
		float Hinvert[3][3];
		float det;

		// Matrix inversion
		float tmp;
		DETERMINANT_3X3(det, H);
		if (fabsf(det) < FLT_MIN)
			break;

		tmp = 1.0f / (det);
		SCALE_ADJOINT_3X3(Hinvert, tmp, H);
		MAT_DOT_VEC_3X3(x_hat, Hinvert, dD);

		xs = x_hat[2];
		xr = x_hat[1];
		xc = x_hat[0];

		// Update tmp data for keypoint update.
		tmp_r = r + xr;
		tmp_c = c + xc;
		tmp_layer = layer + xs;

		// Make sure there is room to move for next iteration.
		xc_i = ((xc >= SIFT_KEYPOINT_SUBPiXEL_THR && c < w - 2) ? 1 : 0)
			+ ((xc <= -SIFT_KEYPOINT_SUBPiXEL_THR && c > 1) ? -1 : 0);

		xr_i = ((xr >= SIFT_KEYPOINT_SUBPiXEL_THR && r < h - 2) ? 1 : 0)
			+ ((xr <= -SIFT_KEYPOINT_SUBPiXEL_THR && r > 1) ? -1 : 0);

		if (xc_i == 0 && xr_i == 0 && xs_i == 0)
			break;
	}

	// We MIGHT be able to remove the following two checking conditions.
	// Condition 1
	if (i >= SIFT_MAX_INTERP_STEPS)
		return false;
	// Condition 2.
	if (fabsf(xc) >= 1.5 || fabsf(xr) >= 1.5 || fabsf(xs) >= 1.5)
		return false;

	// If (r, c, layer) is out of range, return false.
	if (tmp_layer < 0 || tmp_layer >(nGpyrLayers - 1)
		|| tmp_r < 0 || tmp_r > h - 1
		|| tmp_c < 0 || tmp_c > w - 1)
		return false;

	{
		float value = get_pixel_f(currData, w, h, r, c) + 0.5f * (dx * xc + dy * xr + ds * xs);
		if (fabsf(value) < SIFT_CONTR_THR)
			return false;

		float trH = dxx + dyy;
		float detH = dxx * dyy - dxy * dxy;
		float response = (SIFT_CURV_THR + 1) * (SIFT_CURV_THR + 1) / (SIFT_CURV_THR);

		if (detH <= 0 || (trH * trH / detH) >= response)
			return false;
	}

	// Coordinates in the current layer.
	kpt.ci = tmp_c;
	kpt.ri = tmp_r;
	kpt.layer_scale = SIFT_SIGMA * powf(2.0f, tmp_layer / SIFT_INTVLS);

	int firstOctave = SIFT_IMG_DBL ? -1 : 0;
	float norm = powf(2.0f, (float)(octave + firstOctave));
	// Coordinates in the normalized format (compared to the original image).
	kpt.c = tmp_c * norm;
	kpt.r = tmp_r * norm;
	kpt.rlayer = tmp_layer;
	kpt.layer = layer;

	// Formula: Scale = sigma0 * 2^octave * 2^(layer/S);
	kpt.scale = kpt.layer_scale * norm;

	return true;
}



int extract_descriptor(
	Vector  grdPyr,
	Vector  rotPyr,
	int nOctaves,
	int nGpyrLayers,
	Vector *kpt_list)
{
	int nSubregion = SIFT_DESCR_WIDTH;
	int nHalfSubregion = nSubregion >> 1;

	// Number of histogram bins for each descriptor subregion.
	int nBinsPerSubregion = SIFT_DESCR_HIST_BINS;
	float nBinsPerSubregionPerDegree = (float)nBinsPerSubregion / _2PI;

	int nBins = nSubregion * nSubregion * nBinsPerSubregion;
	int nHistBins = (nSubregion + 2) * (nSubregion + 2) * (nBinsPerSubregion + 2);
	int nSliceStep = (nSubregion + 2) * (nBinsPerSubregion + 2); // 32
	int nRowStep = (nBinsPerSubregion + 2);
	float * histBin = new float[nHistBins];

	float exp_scale = -2.0f / (nSubregion * nSubregion);

	for (unsigned int i = 0; i < (*kpt_list).size; ++i)
	{
		int octave = (*(SiftKeypoint *)vector_get_keypoint(kpt_list, i)).octave;
		int layer = (*(SiftKeypoint *)vector_get_keypoint(kpt_list, i)).layer;

		float  kpt_ori = (*(SiftKeypoint *)vector_get_keypoint(kpt_list, i)).ori;
		float kptr = (*(SiftKeypoint *)vector_get_keypoint(kpt_list, i)).ri;
		float kptc = (*(SiftKeypoint *)vector_get_keypoint(kpt_list, i)).ci;
		float kpt_scale = (*(SiftKeypoint *)vector_get_keypoint(kpt_list, i)).layer_scale;

		// Nearest coordinate of keypoints
		int kptr_i = (int)(kptr + 0.5f);
		int kptc_i = (int)(kptc + 0.5f);

		float d_kptr = kptr_i - kptr;
		float d_kptc = kptc_i - kptc;

		int layer_index = octave * nGpyrLayers + layer;
		int w = (*(ImageObj_float *)vector_get_imagefloat(&grdPyr, layer_index)).w;
		int h = (*(ImageObj_float *)vector_get_imagefloat(&grdPyr, layer_index)).h;
		float * grdData = (*(ImageObj_float *)vector_get_imagefloat(&grdPyr, layer_index)).data;
		float * rotData = (*(ImageObj_float *)vector_get_imagefloat(&rotPyr, layer_index)).data;

		float subregion_width = SIFT_DESCR_SCL_FCTR * kpt_scale;
		int win_size = (int)(SQRT2 * subregion_width * (nSubregion + 1) * 0.5f + 0.5f);

		// Normalized cos() and sin() value.
		float sin_t = sinf(kpt_ori) / (float)subregion_width;
		float cos_t = cosf(kpt_ori) / (float)subregion_width;

		// Re-init histBin
		memset(histBin, 0, nHistBins * sizeof(float));

		// Start to calculate the histogram in the sample region.
		float rr, cc;
		float mag, angle, gaussian_weight;

		// Used for tri-linear interpolation.
		//int rbin_i, cbin_i, obin_i;
		float rrotate, crotate;
		float rbin, cbin, obin;
		float d_rbin, d_cbin, d_obin;

		// Boundary of sample region.
		int r, c;
		int left = max(-win_size, 1 - kptc_i);
		int right = min(win_size, w - 2 - kptc_i);
		int top = max(-win_size, 1 - kptr_i);
		int bottom = min(win_size, h - 2 - kptr_i);

		for (int i = top; i <= bottom; i++) // rows
		{
			for (int j = left; j <= right; j++) // columns
			{
				// Accurate position relative to (kptr, kptc)
				rr = i + d_kptr;
				cc = j + d_kptc;
				// Rotate the coordinate of (i, j)
				rrotate = (cos_t * cc + sin_t * rr);
				crotate = (-sin_t * cc + cos_t * rr);
				// Since for a bin array with 4x4 bins, the center is actually at (1.5, 1.5)
				rbin = rrotate + nHalfSubregion - 0.5f;
				cbin = crotate + nHalfSubregion - 0.5f;

				// rbin, cbin range is (-1, d); if outside this range, then the pixel is counted.
				if (rbin <= -1 || rbin >= nSubregion || cbin <= -1 || cbin >= nSubregion)
					continue;
				// All the data need for gradient computation are valid, no border issues.
				r = kptr_i + i;
				c = kptc_i + j;

				mag = grdData[r * w + c];
				angle = rotData[r * w + c] - kpt_ori;
				float angle1 = (angle < 0) ? (_2PI + angle) : angle; // Adjust angle to [0, 2PI)
				obin = angle1 * nBinsPerSubregionPerDegree;

				int x0, y0, z0;
				int x1, y1, z1;
				y0 = (int)floor(rbin);
				x0 = (int)floor(cbin);
				z0 = (int)floor(obin);
				d_rbin = rbin - y0;
				d_cbin = cbin - x0;
				d_obin = obin - z0;
				x1 = x0 + 1;
				y1 = y0 + 1;
				z1 = z0 + 1;

				// Gaussian weight relative to the center of sample region.
				gaussian_weight = expf((rrotate * rrotate + crotate * crotate) * exp_scale);
				// Gaussian-weighted magnitude
				float gm = mag * gaussian_weight;
				// Tri-linear interpolation
				float vr1, vr0;
				float vrc11, vrc10, vrc01, vrc00;
				float vrco110, vrco111, vrco100, vrco101,
					vrco010, vrco011, vrco000, vrco001;

				vr1 = gm * d_rbin;
				vr0 = gm - vr1;
				vrc11 = vr1 * d_cbin;
				vrc10 = vr1 - vrc11;
				vrc01 = vr0 * d_cbin;
				vrc00 = vr0 - vrc01;
				vrco111 = vrc11 * d_obin;
				vrco110 = vrc11 - vrco111;
				vrco101 = vrc10 * d_obin;
				vrco100 = vrc10 - vrco101;
				vrco011 = vrc01 * d_obin;
				vrco010 = vrc01 - vrco011;
				vrco001 = vrc00 * d_obin;
				vrco000 = vrc00 - vrco001;

				// int idx =  y0  * nSliceStep + x0  * nRowStep + z0;
				// All coords are offseted by 1. so x=[1, 4], y=[1, 4];
				// data for -1 coord is stored at position 0;
				// data for 8 coord is stored at position 9.
				// z doesn't need to move.
				int idx = y1 * nSliceStep + x1 * nRowStep + z0;
				histBin[idx] += vrco000;

				idx++;
				histBin[idx] += vrco001;

				idx += nRowStep - 1;
				histBin[idx] += vrco010;

				idx++;
				histBin[idx] += vrco011;

				idx += nSliceStep - nRowStep - 1;
				histBin[idx] += vrco100;

				idx++;
				histBin[idx] += vrco101;

				idx += nRowStep - 1;
				histBin[idx] += vrco110;

				idx++;
				histBin[idx] += vrco111;
			}
		}
		// Discard all the edges for row and column.
		// Only retrive edges for orientation bins.
		float *dstBins = new float[nBins];
		for (int i = 1; i <= nSubregion; i++) // slice
		{
			for (int j = 1; j <= nSubregion; j++) // row
			{
				int idx = i * nSliceStep + j * nRowStep;
				histBin[idx] = histBin[idx + nBinsPerSubregion];
				if (idx != 0)
					histBin[idx + nBinsPerSubregion + 1] = histBin[idx - 1];

				int idx1 = ((i - 1) *nSubregion + j - 1)* nBinsPerSubregion;
				for (int k = 0; k < nBinsPerSubregion; k++)
				{
					dstBins[idx1 + k] = histBin[idx + k];
				}
			}
		}


		// Normalize the histogram
		float sum_square = 0.0f;
		for (int i = 0; i < nBins; i++)
			sum_square += dstBins[i] * dstBins[i];

		float thr = fast_sqrt_f(sum_square) * SIFT_DESCR_MAG_THR;

		float tmp = 0.0;
		sum_square = 0.0;
		// Cut off the numbers bigger than 0.2 after normalized.
		for (int i = 0; i < nBins; i++)
		{
			tmp = min(thr, dstBins[i]);
			dstBins[i] = tmp;
			sum_square += tmp * tmp;
		}

		// Re-normalize
		// The numbers are usually too small to store, so we use
		// a constant factor to scale up the numbers.
		float norm_factor = SIFT_INT_DESCR_FCTR / fast_sqrt_f(sum_square);
		for (int i = 0; i < nBins; i++)
			dstBins[i] = dstBins[i] * norm_factor;

		memcpy((*(SiftKeypoint *)vector_get_keypoint(kpt_list, i)).descriptors, dstBins, nBins * sizeof(float));
		if (dstBins) delete[] dstBins;
	}

	if (histBin) delete[] histBin;

	return 0;

}

int sift(
	ImageObj_uch image,
	Vector *kpt_list,
	bool bExtractDescriptors)
{
	// Index of the first octave.
	int firstOctave = (SIFT_IMG_DBL) ? -1 : 0;
	// Number of layers in one octave; same as s in the paper.
	int nLayers = SIFT_INTVLS;
	// Number of Gaussian images in one octave.
	int nGpyrLayers = nLayers + 3;
	// Number of DoG images in one octave.
	int nDogLayers = nLayers + 2;
	// Number of octaves according to the size of image.
	int nOctaves = (int)my_log2((float)min(image.w, image.h)) - 3 - firstOctave; // 2 or 3, need further research

	// Build image octaves
	Vector octaves;
	vector_setup(&octaves, nOctaves, sizeof(ImageObj_uch));
	build_octaves(image, &octaves, firstOctave, nOctaves);


#if (DUMP_OCTAVE_IMAGE == 1)
	char foctave[256];
	for (int i = 0; i < nOctaves; i++)
	{
		sprintf(foctave, "octave_Octave-%d.pgm", i);
		write_pgm(foctave, octaves[i].data, octaves[i].w, octaves[i].h);
	}
#endif

	// Build Gaussian pyramid
	Vector gpyr;
	vector_setup(&gpyr, nOctaves * nGpyrLayers, sizeof(ImageObj_float));
	vector_init_imagefloat(&gpyr);
	build_gaussian_pyramid(&octaves, &gpyr, nOctaves, nGpyrLayers);

#if (DUMP_GAUSSIAN_PYRAMID_IMAGE == 1)
	char fgpyr[256];
	for (int i = 0; i < nOctaves; i++)
	{
		for (int j = 0; j < nGpyrLayers; j++)
		{
			sprintf(fgpyr, "gpyr-%d-%d.pgm", i, j);
			write_float_pgm(fgpyr, gpyr[i*nGpyrLayers + j].data, gpyr[i*nGpyrLayers + j].w, gpyr[i*nGpyrLayers + j].h, 1);
		}
	}
#endif

	/*
	printf("------------------gpyr-------------------\n");
	for(int h = 0; h < 42; ++h)
	{
	printf("-----gpyr------%d----------w = %d\n", h, ((ImageObj_float *)vector_get_imagefloat(&gpyr,h))->w);
	}
	*/

	// Build DoG pyramid
	Vector dogPyr;
	vector_setup(&dogPyr, nOctaves * nDogLayers, sizeof(ImageObj_float));
	vector_init_imagefloat(&dogPyr);

	build_dog_pyr(gpyr, dogPyr, nOctaves, nDogLayers);

#if (DUMP_DOG_IMAGE == 1)
	char fdog[256];
	ImageObj<unsigned char> img_dog_t;
	for (int i = 0; i < nOctaves; i++)
	{
		for (int j = 0; j < nDogLayers; j++)
		{
			sprintf(fdog, "dog_Octave-%d_Layer-%d.pgm", i, j);
			img_dog_t = dogPyr[i*nDogLayers + j].to_unsigned char();
			write_pgm(fdog, img_dog_t.data, img_dog_t.w, img_dog_t.h);
		}
	}
#endif

	Vector grdPyr, rotPyr;
	vector_setup(&grdPyr, nOctaves * nGpyrLayers, sizeof(ImageObj_float));
	vector_init_imagefloat_allzero(&grdPyr);
	vector_setup(&rotPyr, nOctaves * nGpyrLayers, sizeof(ImageObj_float));
	vector_init_imagefloat_allzero(&rotPyr);
	build_grd_rot_pyr(gpyr, grdPyr, rotPyr, nOctaves, nLayers);

	// Detect keypoints
	detect_keypoints(dogPyr, grdPyr, rotPyr, nOctaves, nDogLayers, kpt_list);

	// Extract descriptor
	//bug function
	if (bExtractDescriptors)
		extract_descriptor(grdPyr, rotPyr, nOctaves, nGpyrLayers, kpt_list);
	return 0;
}


//////////////////////
// Helper Functions //
//////////////////////

// Combine two images horizontally
int combine_image(
	ImageObj_uch  *out_image,
	ImageObj_uch  image1,
	ImageObj_uch  image2)
{
	int w1 = image1.w;
	int h1 = image1.h;
	int w2 = image2.w;
	int h2 = image2.h;
	int dstW = w1 + w2;
	int dstH = max(h1, h2);

	init_uch_imageobj(out_image, dstW, dstH);

	unsigned char * srcData1 = image1.data;
	unsigned char * srcData2 = image2.data;
	unsigned char * dstData = out_image->data;

	for (int r = 0; r < dstH; r++)
	{
		if (r < h1)
		{
			memcpy(dstData, srcData1, w1 * sizeof(unsigned char));
		}
		else {
			memset(dstData, 0, w1 * sizeof(unsigned char));
		}
		dstData += w1;

		if (r < h2)
		{
			memcpy(dstData, srcData2, w2 * sizeof(unsigned char));
		}
		else {
			memset(dstData, 0, w2 * sizeof(unsigned char));
		}
		dstData += w2;
		srcData1 += w1;
		srcData2 += w2;
	}

	return 0;
}

// Helper callback function for merge match list.
bool same_match_pair(MatchPair first, MatchPair second)
{
	if (first.c1 == second.c1 && first.r1 == second.r1
		&& first.c2 == second.c2 && first.r2 == second.r2)
		return true;
	else
		return false;
}

// Match keypoints from two images, using brutal force method.
// Use Euclidean distance as matching score.

Vector match_keypoints(Vector  *kpt_list1,
	Vector  *kpt_list2)
{

	Vector *match_list = (Vector *)malloc(sizeof(Vector));
	vector_setup(match_list, 1024, sizeof(MatchPair));

	for (unsigned int i = 0; i < (*kpt_list1).size; ++i)
	{
		// Position of the matched feature.
		int r1 = (int)(((SiftKeypoint *)vector_get_keypoint(kpt_list1, i))->r);
		int c1 = (int)(((SiftKeypoint *)vector_get_keypoint(kpt_list1, i))->c);


		float *descr1 = (float *)(((SiftKeypoint *)vector_get_keypoint(kpt_list1, i))->descriptors);
		float score1 = FLT_MAX; // highest score
		float score2 = FLT_MAX; // 2nd highest score

		// Position of the matched feature.
		int r2, c2;
		for (unsigned int j = 0; j < (*kpt_list2).size; ++j)
		{
			float score = 0;
			float * descr2 = (float *)(((SiftKeypoint *)vector_get_keypoint(kpt_list2, j))->descriptors);
			float dif;
			for (int i = 0; i < DEGREE_OF_DESCRIPTORS; i++)
			{
				dif = descr1[i] - descr2[i];
				score += dif * dif;
			}
			if (score < score1)
			{
				score2 = score1;
				score1 = score;
				r2 = (int)(((SiftKeypoint *)vector_get_keypoint(kpt_list2, j))->r);
				c2 = (int)(((SiftKeypoint *)vector_get_keypoint(kpt_list2, j))->c);
			}
			else if (score < score2)
			{
				score2 = score;
			}
		}

		if (fast_sqrt_f(score1 / score2) < SIFT_MATCH_NNDR_THR)
		{
			MatchPair *mp = (MatchPair*)malloc(sizeof(MatchPair));
			mp->r1 = r1;
			mp->c1 = c1;
			mp->r2 = r2;
			mp->c2 = c2;
			vector_push_back_matchpair(match_list, mp);
			free(mp);
		}
	}
	Vector *tmp = (Vector *)malloc(sizeof(MatchPair));
	vector_setup(tmp, 1024, sizeof(MatchPair));
	for (unsigned int i = 0; i < (*match_list).size; ++i)
	{
		unsigned int j = 0;
		while (j < (*match_list).size)
		{
			if (i != j)
			{
				if (same_match_pair(*(MatchPair*)vector_get_matchpair(match_list, i), *(MatchPair*)vector_get_matchpair(match_list, j)))
					break;
			}
			++j;
		}
		if (j == (*match_list).size)
			vector_push_back_matchpair(tmp, (MatchPair*)vector_get_matchpair(match_list, i));
	}
	match_list = tmp;

#if PRINT_MATCH_KEYPOINTS
	int match_idx = 0;
	for (unsigned int i = 0; i < (*match_list).size; ++i)
	{
		printf("\tMatch %3d: (%4d, %4d) -> (%4d, %4d)\n", match_idx, ((MatchPair *)vector_get_matchpair(match_list, i))->r1, ((MatchPair *)vector_get_matchpair(match_list, i))->c1, ((MatchPair *)vector_get_matchpair(match_list, i))->r2, ((MatchPair *)vector_get_matchpair(match_list, i))->c2);
		match_idx++;
	}
#endif
	//free(match_list);
	return *tmp;
}

int  draw_keypoints_to_bmp_file(
	char* out_filename,
	ImageObj_uch  image,
	Vector *kpt_list)
{
	BMP_IMG imgBMP;
	int w = image.w;
	int	h = image.h;
	int r, c;

	/*******************************
	* cR:
	* radius of the circle
	* cR = sigma * 4 * (2^O)
	*******************************/
	int cR;

	// initialize the imgBMP
	imgBMP.w = w;
	imgBMP.h = h;
	imgBMP.img_r = new unsigned char[w * h];
	imgBMP.img_g = new unsigned char[w * h];
	imgBMP.img_b = new unsigned char[w * h];

	int i, j;
	unsigned char * data = image.data;
	// Copy gray PGM images to color PPM images
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			imgBMP.img_r[i * w + j] = data[i * w + j];
			imgBMP.img_g[i * w + j] = data[i * w + j];
			imgBMP.img_b[i * w + j] = data[i * w + j];
		}
	}
	int list_size = (*kpt_list).size;
	for (int it = 0; it < list_size; ++it)
	{
		// derive circle radius cR
		cR = (int)(*(SiftKeypoint *)vector_get_keypoint(kpt_list, it)).scale;
		if (cR <= 1)
		{ // avoid zero radius
			cR = 1;
		}
		r = (int)(*(SiftKeypoint *)vector_get_keypoint(kpt_list, it)).r;
		c = (int)(*(SiftKeypoint *)vector_get_keypoint(kpt_list, it)).c;

		rasterCircle(&imgBMP, r, c, cR);
		rasterCircle(&imgBMP, r, c, cR + 1);
		float ori = (*(SiftKeypoint *)vector_get_keypoint(kpt_list, it)).ori;
		draw_red_orientation(&imgBMP, c, r, ori, cR);
	}
	// write rendered image to output
	write_rgb2bmp(out_filename, imgBMP.img_r, imgBMP.img_g, imgBMP.img_b, w, h);
	// free allocated memory
	delete[] imgBMP.img_r;
	delete[] imgBMP.img_g;
	delete[] imgBMP.img_b;

	return 0;
}
// render()
int export_kpt_list_to_file(
	const char * filename,
	Vector * kpt_list,
	bool bIncludeDescpritor)
{
	FILE * fp;
	fp = fopen(filename, "wb");
	if (!fp) {
		printf("Fail to open file: %s\n", filename);
		//exit(1);
		return -1;
	}

	fprintf(fp, "%d\t%d\n", (*kpt_list).size, 128);

	for (unsigned int i = 0; i < (*kpt_list).size; ++i)
	{
		int tmp_octave = ((SiftKeypoint *)vector_get_keypoint(kpt_list, i))->octave;
		int tmp_layer = ((SiftKeypoint *)vector_get_keypoint(kpt_list, i))->layer;
		float tmp_r = ((SiftKeypoint *)vector_get_keypoint(kpt_list, i))->r;
		float tmp_c = ((SiftKeypoint *)vector_get_keypoint(kpt_list, i))->c;
		float tmp_scale = ((SiftKeypoint *)vector_get_keypoint(kpt_list, i))->scale;
		float tmp_ori = ((SiftKeypoint *)vector_get_keypoint(kpt_list, i))->ori;


		fprintf(fp, "%d\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t", tmp_octave, tmp_layer, tmp_r, tmp_c, tmp_scale, tmp_ori);
		if (bIncludeDescpritor)
		{
			for (int i = 0; i < 128; i++)
			{
				int tmp_desc = (int)(((SiftKeypoint *)vector_get_keypoint(kpt_list, i))->descriptors[i]);
				fprintf(fp, "%d\t", tmp_desc);
			}
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	return 0;
}

// Draw a line on the RGB color image.
int draw_line_to_rgb_image(unsigned char* & data,
	int w, int h,
	MatchPair & mp)
{
	int r1 = mp.r1;
	int c1 = mp.c1;
	int r2 = mp.r2;
	int c2 = mp.c2;

	float k = (float)(r2 - r1) / (float)(c2 - c1);
	for (int c = c1; c < c2; c++)
	{
		// Line equation
		int r = (int)(k * (c - c1) + r1);

		// Draw a blue line
		data[r * w * 3 + 3 * c] = 0;
		data[r * w * 3 + 3 * c + 1] = 0;
		data[r * w * 3 + 3 * c + 2] = 255;
	}

	return 0;
}


// Draw match lines between matched keypoints between two images.
int draw_match_lines_to_bmp_file(char * filename,
	ImageObj_uch  image1,
	ImageObj_uch  image2,
	Vector  match_list)
{
	//ImageObj_uch *tmpImage = (ImageObj_uch *)malloc(sizeof(tmpImage));
	ImageObj_uch *tmpImage = (ImageObj_uch *)malloc(sizeof(ImageObj_uch));
	combine_image(tmpImage, image1, image2);

	int w = tmpImage->w;
	int h = tmpImage->h;
	unsigned char * srcData = tmpImage->data;

	//由于bmp特殊存储方式，需要上下翻转图像
	for (int i = 0; i < h / 2; ++i) {
		for (int j = 0; j < w; ++j)
		{
			unsigned char temp = 0;
			temp = srcData[i*w + j];
			srcData[i*w + j] = srcData[(h - 1 - i)*w + j];
			srcData[(h - 1 - i)*w + j] = temp;
			temp = srcData[i*w + j];
			srcData[i*w + j] = srcData[(h - 1 - i)*w + j];
			srcData[(h - 1 - i)*w + j] = temp;
			temp = srcData[i*w + j];
			srcData[i*w + j] = srcData[(h - 1 - i)*w + j];
			srcData[(h - 1 - i)*w + j] = temp;
		}
	}
	unsigned char * dstData = (unsigned char *)malloc(w*h * 3);
	//存入新数组
	for (int k = 0; k < w * h; k++){
		dstData[k * 3 + 0] = srcData[k];
		dstData[k * 3 + 1] = srcData[k];
		dstData[k * 3 + 2] = srcData[k];
	}

	for (unsigned int i = 0; i < match_list.size; ++i)
	{
		MatchPair *mp = (MatchPair *)malloc(sizeof(MatchPair));
		mp->r1 = ((MatchPair *)vector_get_matchpair(&match_list, i))->r1;
		mp->c1 = ((MatchPair *)vector_get_matchpair(&match_list, i))->c1;
		mp->r2 = ((MatchPair *)vector_get_matchpair(&match_list, i))->r2;
		mp->c2 = ((MatchPair *)vector_get_matchpair(&match_list, i))->c2 + image1.w;
		draw_line_to_rgb_image(dstData, w, h, *mp);
		free(mp);
	}
	write_bmp(filename, dstData, w, h);
	free(dstData);
	free(tmpImage);
	return 0;
}