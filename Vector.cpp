/*----------------------------------
updated by wonderly321 on 4/11/19.
-----------------------------------*/

#include "Vector.h"

int vector_setup(Vector *vector, size_t capacity, size_t element_size)
{
	assert(vector != NULL);
	vector->size = 0;
	vector->capacity = MAX(VECTOR_MINIMUM_CAPACITY, capacity);
	vector->element_size = element_size;
	vector->data = malloc(vector->capacity * element_size);

	return vector->data == NULL ? VECTOR_ERROR : VECTOR_SUCCESS;
}

size_t vector_byte_size(const Vector *vector)
{
	return vector->size * vector->element_size;
}


bool _vector_should_grow(Vector *vector)
{
	assert(vector->size <= vector->capacity);
	return vector->size == vector->capacity;
}


int _vector_reallocate(Vector *vector, size_t new_capacity)
{
	size_t new_capacity_in_bytes;
	void *old;
	assert(vector != NULL);

	if (new_capacity < VECTOR_MINIMUM_CAPACITY)
	{
		if (vector->capacity > VECTOR_MINIMUM_CAPACITY)
		{
			new_capacity = VECTOR_MINIMUM_CAPACITY;
		}
		else
		{
			/* NO-OP */
			return VECTOR_SUCCESS;
		}
	}

	new_capacity_in_bytes = new_capacity * vector->element_size;
	old = vector->data;

	if ((vector->data = malloc(new_capacity_in_bytes)) == NULL)
	{
		return VECTOR_ERROR;
	}

#ifdef __STDC_LIB_EXT1__
	/* clang-format off */
	if (memcpy_s(vector->data,
		new_capacity_in_bytes,
		old,
		vector_byte_size(vector)) != 0) {
		return VECTOR_ERROR;
	}
	/* clang-format on */
#else
	memcpy(vector->data, old, vector_byte_size(vector));
#endif

	vector->capacity = new_capacity;

	free(old);

	return VECTOR_SUCCESS;
}

int _vector_adjust_capacity(Vector *vector)
{
	return _vector_reallocate(vector,
		MAX(1, vector->size * VECTOR_GROWTH_FACTOR));
}


int vector_resize(Vector *vector, size_t new_size)
{
	if (new_size <= vector->capacity * VECTOR_SHRINK_THRESHOLD)
	{
		vector->size = new_size;
		if (_vector_reallocate(vector, new_size * VECTOR_GROWTH_FACTOR) == -1)
		{
			return VECTOR_ERROR;
		}
	}
	else if (new_size > vector->capacity)
	{
		if (_vector_reallocate(vector, new_size * VECTOR_GROWTH_FACTOR) == -1)
		{
			return VECTOR_ERROR;
		}
	}

	vector->size = new_size;

	return VECTOR_SUCCESS;
}


int vector_clear(Vector *vector)
{
	return vector_resize(vector, 0);
}

void *_vector_offset_float(Vector *vector, size_t index)
{
	//printf("----------%x", vector->data);
	return ((char *)vector->data + (index * vector->element_size));
	//return (float *)vector->data + (index * vector->element_size);
}

void _vector_assign_float(Vector *vector, size_t index, void *element)
{
	/* Insert the element */
	void *offset = _vector_offset_float(vector, index);
	memcpy(offset, element, vector->element_size);
}


int vector_push_back_float(Vector *vector, void *element)
{
	assert(vector != NULL);
	assert(element != NULL);

	if (_vector_should_grow(vector))
	{
		if (_vector_adjust_capacity(vector) == VECTOR_ERROR)
		{
			return VECTOR_ERROR;
		}
	}

	_vector_assign_float(vector, vector->size, element);

	++vector->size;

	return VECTOR_SUCCESS;
}

void *vector_get_float(Vector *vector, size_t index) {
	assert(vector != NULL);
	assert(index < vector->size);

	if (vector == NULL)
		return NULL;
	if (vector->element_size == 0)
		return NULL;
	if (index >= vector->size)
		return NULL;

	return _vector_offset_float(vector, index);
}

int vector_init_float(Vector *vector)
{
	float tmp_float = 1024.0;
	int tmp_size = vector->capacity;
	for (int i = 0; i < tmp_size; ++i)
	{
		_vector_assign_float(vector, i, &tmp_float);
		++(vector->size);
	}
	return 0;
}

int vector_init_float2(Vector *vector)
{
	int tmp_cnt = 25;
	for (int j = 0; j < tmp_cnt; ++j)
	{
		Vector *tmp_vector_float = (Vector *)malloc(sizeof(Vector));
		vector_setup(tmp_vector_float, 1024, sizeof(float));
		vector_push_back_float(vector, tmp_vector_float);
		free(tmp_vector_float);
	}
	return 0;
}

int vector_assign_float(Vector *vector, size_t index, float *element)
{
	assert(vector != NULL);
	assert(element != NULL);
	assert(index < vector->size);

	if (vector == NULL)
		return VECTOR_ERROR;
	if (element == NULL)
		return VECTOR_ERROR;
	if (vector->element_size == 0)
		return VECTOR_ERROR;
	if (index >= vector->size)
		return VECTOR_ERROR;

	_vector_assign_float(vector, index, element);

	return VECTOR_SUCCESS;
}

void *_vector_offset_imagefloat(Vector *vector, size_t index)
{
	return (ImageObj_float *)((char *)vector->data + (index * vector->element_size));
	//return (ImageObj_float *)vector->data + (index * vector->element_size);
}

void _vector_assign_imagefloat(Vector *vector, size_t index, void *element)
{
	/* Insert the element */
	void *offset = _vector_offset_imagefloat(vector, index);
	memcpy(offset, element, vector->element_size);
}


void *vector_get_imagefloat(Vector *vector, size_t index) {
	assert(vector != NULL);
	assert(index < vector->size);

	if (vector == NULL)
		return NULL;
	if (vector->element_size == 0)
		return NULL;
	if (index >= vector->size)
		return NULL;

	return _vector_offset_imagefloat(vector, index);
}


int vector_init_imagefloat(Vector *vector)
{
	int tmp_size = vector->capacity;
	for (int i = 0; i < tmp_size; ++i)
	{
		ImageObj_float *tmp_float = (ImageObj_float *)malloc(sizeof(ImageObj_float));
		//printf("------dubug-----address-------%x\n",tmp_float);
		init_float_imageobj(tmp_float, 1024, 1024);
		_vector_assign_imagefloat(vector, i, tmp_float);
		++vector->size;
		free(tmp_float);
	}
	return 0;
}

int vector_init_imagefloat_allzero(Vector *vector)
{
	int tmp_size = vector->capacity;
	for (int i = 0; i < tmp_size; ++i)
	{
		ImageObj_float *tmp_float = (ImageObj_float *)malloc(sizeof(ImageObj_float));
		//printf("------dubug-----address-------%x\n",tmp_float);
		init_float_imageobj(tmp_float, 0, 0);
		_vector_assign_imagefloat(vector, i, tmp_float);
		++vector->size;
		free(tmp_float);
	}
	return 0;
}



void *_vector_offset_imageuch(Vector *vector, size_t index)
{
	return (ImageObj_uch *)((char *)vector->data + (index * vector->element_size));
}

void _vector_assign_imageuch(Vector *vector, size_t index, void *element)
{
	/* Insert the element */
	void *offset = _vector_offset_imageuch(vector, index);
	//printf("#%d\n",((ImageObj_uch*)element)->w);
	memcpy(offset, element, vector->element_size);
	//printf("!%d %d\n",((ImageObj_uch*)offset)->w, index);
}


int vector_push_back_imageuch(Vector *vector, void *element)
{
	assert(vector != NULL);
	assert(element != NULL);

	if (_vector_should_grow(vector))
	{
		if (_vector_adjust_capacity(vector) == VECTOR_ERROR)
		{
			return VECTOR_ERROR;
		}
	}
	_vector_assign_imageuch(vector, vector->size, element);

	++(vector->size);

	return VECTOR_SUCCESS;
}

void *vector_get_imageuch(Vector *vector, size_t index) {
	assert(vector != NULL);
	assert(index < vector->size);

	if (vector == NULL)
		return NULL;
	if (vector->element_size == 0)
		return NULL;
	if (index >= vector->size)
		return NULL;

	return _vector_offset_imageuch(vector, index);
}

void *_vector_offset_keypoint(Vector *vector, size_t index)
{
	return (SiftKeypoint *)((char *)vector->data + (index * vector->element_size));
	//return (ImageObj_uch *)((char *)vector->data + (index * vector->element_size));
}

void _vector_assign_keypoint(Vector *vector, size_t index, void *element)
{
	/* Insert the element */
	void *offset = _vector_offset_keypoint(vector, index);
	memcpy(offset, element, vector->element_size);
}


int vector_push_back_keypoint(Vector *vector, void *element)
{
	assert(vector != NULL);
	assert(element != NULL);

	if (_vector_should_grow(vector))
	{
		if (_vector_adjust_capacity(vector) == VECTOR_ERROR)
		{
			return VECTOR_ERROR;
		}
	}

	_vector_assign_keypoint(vector, vector->size, element);

	++vector->size;

	return VECTOR_SUCCESS;
}

void *vector_get_keypoint(Vector *vector, size_t index) {
	assert(vector != NULL);
	assert(index < vector->size);

	if (vector == NULL)
		return NULL;
	if (vector->element_size == 0)
		return NULL;
	if (index >= vector->size)
		return NULL;

	return _vector_offset_keypoint(vector, index);
}

void *_vector_offset_matchpair(Vector *vector, size_t index)
{
	return (MatchPair *)((char *)vector->data + (index * vector->element_size));
	//return (ImageObj_uch *)((char *)vector->data + (index * vector->element_size));
}

void _vector_assign_matchpair(Vector *vector, size_t index, void *element)
{
	/* Insert the element */
	void *offset = _vector_offset_matchpair(vector, index);
	memcpy(offset, element, vector->element_size);
}


int vector_push_back_matchpair(Vector *vector, void *element)
{
	assert(vector != NULL);
	assert(element != NULL);

	if (_vector_should_grow(vector))
	{
		if (_vector_adjust_capacity(vector) == VECTOR_ERROR)
		{
			return VECTOR_ERROR;
		}
	}

	_vector_assign_matchpair(vector, vector->size, element);

	++vector->size;

	return VECTOR_SUCCESS;
}

void *vector_get_matchpair(Vector *vector, size_t index) {
	assert(vector != NULL);
	assert(index < vector->size);

	if (vector == NULL)
		return NULL;
	if (vector->element_size == 0)
		return NULL;
	if (index >= vector->size)
		return NULL;

	return _vector_offset_matchpair(vector, index);
}

