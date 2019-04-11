/*----------------------------------
updated by wonderly321 on 4/11/19.
-----------------------------------*/

#ifndef SIFT_SIFT_H
#define SIFT_SIFT_H

#include "image.h"
#include "image_io.h"

#include "Vector.h"
/****************************************
 * Constant parameters
 ***************************************/

// default number of sampled intervals per octave
static int SIFT_INTVLS = 3;

// default sigma for initial gaussian smoothing
static float SIFT_SIGMA = 1.6f;

// the radius of Gaussian filter kernel;
// Gaussian filter mask will be (2*radius+1)x(2*radius+1).
// People use 2 or 3 most.
static float SIFT_GAUSSIAN_FILTER_RADIUS = 3.0f;

// default threshold on keypoint contrast |D(x)|
static float SIFT_CONTR_THR = 8.0f; //8.0f;

// default threshold on keypoint ratio of principle curvatures
static float SIFT_CURV_THR = 10.0f;

// The keypoint refinement smaller than this threshold will be discarded.
static float SIFT_KEYPOINT_SUBPiXEL_THR = 0.6f;

// double image size before pyramid construction?
static bool SIFT_IMG_DBL = false; //true;

// assumed gaussian blur for input image
static float SIFT_INIT_SIGMA = 0.5f;

// width of border in which to ignore keypoints
static int SIFT_IMG_BORDER = 5;

// maximum steps of keypoint interpolation before failure
static int SIFT_MAX_INTERP_STEPS = 5;

// default number of bins in histogram for orientation assignment
static int SIFT_ORI_HIST_BINS = 36;

// determines gaussian sigma for orientation assignment
static float SIFT_ORI_SIG_FCTR = 1.5f; // Can affect the orientation computation.

// determines the radius of the region used in orientation assignment
static float SIFT_ORI_RADIUS = 3 * SIFT_ORI_SIG_FCTR; // Can affect the orientation computation.

// orientation magnitude relative to max that results in new feature
static float SIFT_ORI_PEAK_RATIO = 0.8f;

// maximum number of orientations for each keypoint location
//static const float SIFT_ORI_MAX_ORI = 4;

// determines the size of a single descriptor orientation histogram
static float SIFT_DESCR_SCL_FCTR = 3.f;

// threshold on magnitude of elements of descriptor vector
static float SIFT_DESCR_MAG_THR = 0.2f;

// factor used to convert floating-point descriptor to unsigned char
static float SIFT_INT_DESCR_FCTR = 512.f;

// default width of descriptor histogram array
static int SIFT_DESCR_WIDTH = 4;

// default number of bins per histogram in descriptor array
static int SIFT_DESCR_HIST_BINS = 8;

// default value of the nearest-neighbour distance ratio threshold
// |DR_nearest|/|DR_2nd_nearest|<SIFT_MATCH_NNDR_THR is considered as a match.
static float SIFT_MATCH_NNDR_THR = 0.65f;

#if 0
// intermediate type used for DoG pyramids
typedef short sift_wt;
static const int SIFT_FIXPT_SCALE = 48;
#else
// intermediate type used for DoG pyramids
typedef float sift_wt;
static const int SIFT_FIXPT_SCALE = 1;
#endif

/****************************************
 * Definitions
 ***************************************/


// Combine two images for interest points matching.
// Images are combined horizontally.
int combine_image(
        ImageObj_uch  *out_image,
        ImageObj_uch  image1,
        ImageObj_uch  image2);

// Draw circles to incidate the keypoints.
// Bars in the circle incidate the orientation of the keypoints.
int  draw_keypoints_to_bmp_file(
        char* out_filename,
        ImageObj_uch  image,
        Vector *kpt_list);

// Draw lines between matched keypoints.
// The result image is stored in a ppm file.
int draw_match_lines_to_bmp_file(
        char * filename,
        ImageObj_uch  image1,
        ImageObj_uch  image2,
        Vector  match_list);

// Draw matched lines on a color RGB image.
int draw_line_to_rgb_image(
        unsigned char* & data,
        int w, int h,
        MatchPair & mp);

// Draw matched lines on an ImageObj object.
int draw_line_to_image(
        ImageObj_uch & image,
        MatchPair & mp);


/****************************************
 *  SIFT Processing Functions
 ***************************************/

// Enable doubling of original image.
void double_original_image(bool doubleFirstOctave);

// Efficient Gaussian Blur function.
// 1. Use row buf to handle border pixel.
// 2. hori processing and transpose
int gaussian_blur(
        ImageObj_float & in_image,
        ImageObj_float & out_image,
        Vector coef1d);

// Row filter and then transpose
int row_filter_transpose(
        float * src,
        float * dst,
        int w, int h,
        float * coef1d, int gR);

// Build image octaves during the initialization.
int build_octaves(ImageObj_uch image,
                  Vector *octaves,
                  int firstOctave,
                  int nOctaves);

// Compute Gaussian filter coefficients for Gaussian Blur.
int compute_gaussian_coefs(Vector *gaussian_coefs, int nGpyrLayers);

// Build Gaussian pyramid.
int build_gaussian_pyramid(
        Vector *octaves,
        Vector *gpyr,
        int nOctaves,
        int nGpyrLayers);

// Build DoG pyramid.
int build_dog_pyr(
        Vector & gpyr,
        Vector & dogPyr,
        int nOctaves,
        int nDogLayers);

// Build gradient and rotation pyramid.
int build_grd_rot_pyr(
        Vector & gpyr,
        Vector & grdPyr,
        Vector & rotPyr,
        int nOctaves,
        int nLayers);

// Refine local extrema.
bool refine_local_extrema(
        Vector & dogPyr,
        int nOctaves,
        int nDogLayers,
        SiftKeypoint & kpt);

// Export keypoint list to a file.
int export_kpt_list_to_file(
        const char * filename,
        Vector * kpt_list,
        bool bIncludeDescpritor);

/****************************************
 *  SIFT Core Functions
 ***************************************/
// Detect keypoints.
int detect_keypoints(
        Vector  dogPyr,
        Vector  grdPyr,
        Vector  rotPyr,
        int nOctaves,
        int nDogLayers,
        Vector * kpt_list);

// Extract descriptor.
int extract_descriptor(
        Vector  grdPyr,
        Vector  rotPyr,
        int nOctaves,
        int nGpyrLayers,
        Vector *kpt_list);


/****************************************
 *  SIFT Interface Functions
 ***************************************/
// Detect keypoints and extract descriptor.
int sift(
        ImageObj_uch image,
        Vector  * kpt_list,
        bool bExtractDescriptors);

// Match keypoints from two keypoint lists.
Vector match_keypoints(
        Vector  *kpt_list1,
        Vector  *kpt_list2);

#endif //SIFT_SIFT_H
