/*----------------------------------
updated by wonderly321 on 4/11/19.
-----------------------------------*/

#ifndef SIFT_VECTOR_H
#define SIFT_VECTOR_H
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "image.h"

#define VECTOR_MINIMUM_CAPACITY 2
#define VECTOR_GROWTH_FACTOR 2
#define VECTOR_SHRINK_THRESHOLD (1 / 4)

#define VECTOR_ERROR -1
#define VECTOR_SUCCESS 0

#define VECTOR_UNINITIALIZED NULL

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define VECTOR_INITIALIZER            \
    {                                 \
        0, 0, 0, VECTOR_UNINITIALIZED \
    }
#define DEGREE_OF_DESCRIPTORS (128)
typedef struct _SiftKeypoint
{
    int octave; // octave number
    int layer;  // layer number
    float rlayer; // real number of layer number

    float r; // normalized row coordinate
    float c; // normalized col coordinate
    float scale; // normalized scale

    float ri;	//row coordinate in that layer.
    float ci;	//column coordinate in that layer.
    float layer_scale; // the scale of that layer

    float ori; // orientation in degrees.
    float mag; // magnitude

    float descriptors[DEGREE_OF_DESCRIPTORS];
} SiftKeypoint;

// Match pair structure. Use for interest point matching.
typedef struct _MatchPair
{
    int r1;
    int c1;
    int r2;
    int c2;
} MatchPair;




typedef struct Vector
{
    size_t size;
    size_t capacity;
    size_t element_size;

    void *data;
} Vector;


int vector_setup(Vector *vector, size_t capacity, size_t element_size);

size_t vector_byte_size(const Vector *vector);

bool _vector_should_grow(Vector *vector);

int _vector_reallocate(Vector *vector, size_t new_capacity);

int _vector_adjust_capacity(Vector *vector);

int vector_resize(Vector *vector, size_t new_size);

int vector_clear(Vector *vector);


void *_vector_offset_float(Vector *vector, size_t index);

void _vector_assign_float(Vector *vector, size_t index, void *element);

int vector_push_back_float(Vector *vector, void *element);

void *vector_get_float(Vector *vector, size_t index);

int vector_init_float(Vector *vector);

int vector_init_float2(Vector *vector);

int vector_assign_float(Vector *vector, size_t index, float *element);


void *_vector_offset_imagefloat(Vector *vector, size_t index);

void _vector_assign_imagefloat(Vector *vector, size_t index, void *element);

void *vector_get_imagefloat(Vector *vector, size_t index);


int vector_init_imagefloat(Vector *vector);


int vector_init_imagefloat_allzero(Vector *vector);


void *_vector_offset_imageuch(Vector *vector, size_t index);

void _vector_assign_imageuch(Vector *vector, size_t index, void *element);

int vector_push_back_imageuch(Vector *vector, void *element);

void *vector_get_imageuch(Vector *vector, size_t index);


void *_vector_offset_keypoint(Vector *vector, size_t index);

void _vector_assign_keypoint(Vector *vector, size_t index, void *element);

int vector_push_back_keypoint(Vector *vector, void *element);

void *vector_get_keypoint(Vector *vector, size_t index);


void *_vector_offset_matchpair(Vector *vector, size_t index);

void _vector_assign_matchpair(Vector *vector, size_t index, void *element);

int vector_push_back_matchpair(Vector *vector, void *element);

void *vector_get_matchpair(Vector *vector, size_t index);


#endif //SIFT_VECTOR_H
