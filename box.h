/*
Copyright (c) 2025 broskobandi
Licensed under the MIT License. See LICENSE file for details.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef BOX_H
#define BOX_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum box_status {
	SUCCESS,
	ALLOC_ERROR,
	REALLOC_ERROR,
	NULL_PTR,
	BOX_NOT_EMPTY,
	BOX_EMPTY,
	OUT_OF_BOUNDS,
	DATA_NOT_EMPTY,
} box_status_t;

#define TRY(function)\
	do {\
		box_status_t _tmp = function;\
		if (_tmp) return (int)_tmp;\
	} while (0)

#define AUTO(T) __attribute__((cleanup(destroy_##T))) T
#define AUTO_BOX(T) __attribute__((cleanup(destroy_box_##T))) box_##T##_t
#define BOX(T) box_##T##_t

#define GEN_AUTO(T)\
void destroy_##T(T **ptr);

#define GEN_BOX(T)\
typedef struct box_##T box_##T##_t;\
box_status_t create_box_##T(box_##T##_t **box);\
box_status_t create_box_##T##_from(box_##T##_t **box, T data);\
void destroy_box_##T(box_##T##_t **box);\
box_status_t push_box_##T(box_##T##_t *box, T data);\
box_status_t pop_box_##T(box_##T##_t *box, T *item);\
box_status_t remove_box_##T(box_##T##_t *box, unsigned int index);\
box_status_t get_box_##T(const box_##T##_t *box, T *item, unsigned int index);\
box_status_t set_box_##T(box_##T##_t *box, T data, unsigned int index);\
box_status_t get_box_##T##_data(const box_##T##_t *box, T **data);\
box_status_t get_box_##T##_len(const box_##T##_t *box, unsigned int *len);\
box_status_t cat_box_##T(box_##T##_t *dest, const box_##T##_t *src);\

#define GEN_BOX_PRINT(T)\
box_status_t print_box_##T(const box_##T##_t *box);

#define GEN_AUTO_IMPL(T)\
void destroy_##T(T **ptr) {\
	if (ptr && *ptr) {\
		free(*ptr);\
		*ptr = NULL;\
	}\
}\

#define GEN_BOX_IMPL(T)\
struct box_##T {\
	T *data;\
	unsigned int len;\
	unsigned int capacity;\
};\
box_status_t create_box_##T(box_##T##_t **box) {\
	if (*box) return BOX_NOT_EMPTY;\
	*box = calloc(1, sizeof(box_##T##_t));\
	if (!*box) return ALLOC_ERROR;\
	(*box)->data = NULL;\
	(*box)->len = 0;\
	(*box)->capacity = 1;\
	return SUCCESS;\
}\
box_status_t create_box_##T##_from(box_##T##_t **box, T data) {\
	if (*box) return BOX_NOT_EMPTY;\
	*box = calloc(1, sizeof(box_##T##_t));\
	if (!*box) return ALLOC_ERROR;\
	(*box)->data = calloc(1, sizeof(T));\
	if (!(*box)->data) return ALLOC_ERROR;\
	memcpy((*box)->data, &data, sizeof(data));\
	(*box)->len = 1;\
	(*box)->capacity = 1;\
	return SUCCESS;\
}\
void destroy_box_##T(box_##T##_t **box) {\
	if (box && *box) {\
		if ((*box)->data) free((*box)->data);\
		free(*box);\
		*box = NULL;\
	}\
}\
box_status_t push_box_##T(box_##T##_t *box, T data) {\
	if (!box) return NULL_PTR;\
	if (!box->data) {\
		box->data = calloc(1, sizeof(T));\
		if (!box->data) return ALLOC_ERROR;\
	}\
	if (box->len + 1 > box->capacity) {\
		unsigned int new_capacity =\
			box->capacity > 32 ?\
			(unsigned int)((float)box->capacity * 1.5f) :\
			box->capacity * 2;\
		T *tmp = realloc(box->data, new_capacity * sizeof(T));\
		if (!tmp) return REALLOC_ERROR;\
		box->data = tmp;\
		box->capacity = new_capacity;\
	}\
	box->data[box->len] = data;\
	box->len++;\
	return SUCCESS;\
}\
static box_status_t reduce_box_##T(box_##T##_t *box) {\
	memset(&box->data[box->len - 1], 0, sizeof(T));\
	box->len--;\
	if (box->len < box->capacity / 2) {\
		box->capacity /= 2;\
		T *tmp = realloc(box->data, box->capacity * sizeof(T));\
		if (!tmp) return REALLOC_ERROR;\
		box->data = tmp;\
	}\
	return SUCCESS;\
}\
box_status_t pop_box_##T(box_##T##_t *box, T *item) {\
	if (!box || !item) return NULL_PTR;\
	if (!box->len) return BOX_EMPTY;\
	*item = box->data[box->len - 1];\
	box_status_t stat = reduce_box_##T(box);\
	if (stat) return stat;\
	return SUCCESS;\
}\
box_status_t remove_box_##T(box_##T##_t *box, unsigned int index) {\
	if (!box) return NULL_PTR;\
	if (!box->len) return BOX_EMPTY;\
	if (index > box->len - 1) return OUT_OF_BOUNDS;\
	for (int i = (int)index; i < (int)box->len; i++) {\
		box->data[i] = box->data[i + 1];\
	}\
	box_status_t stat = reduce_box_##T(box);\
	if (stat) return stat;\
	return SUCCESS;\
}\
box_status_t get_box_##T(const box_##T##_t *box, T *item, unsigned int index) {\
	if (!box || !item) return NULL_PTR;\
	if (!box->len) return BOX_EMPTY;\
	if (index > box->len - 1) return OUT_OF_BOUNDS;\
	*item = box->data[index];\
	return SUCCESS;\
}\
box_status_t set_box_##T(box_##T##_t *box, T data, unsigned int index) {\
	if (!box) return NULL_PTR;\
	if (!box->len) return BOX_EMPTY;\
	if (index > box->len - 1) return OUT_OF_BOUNDS;\
	box->data[index] = data;\
	return SUCCESS;\
}\
box_status_t get_box_##T##_data(const box_##T##_t *box, T **data) {\
	if (!box) return NULL_PTR;\
	if (*data) return DATA_NOT_EMPTY;\
	*data = calloc(box->len, sizeof(T));\
	memcpy(*data, box->data, sizeof(T) * box->len);\
	return SUCCESS;\
}\
box_status_t get_box_##T##_len(const box_##T##_t *box, unsigned int *len) {\
	if (!box) return NULL_PTR;\
	if (!box->len) return BOX_EMPTY;\
	*len = box->len;\
	return SUCCESS;\
}\
box_status_t cat_box_##T(box_##T##_t *dest, const box_##T##_t *src) {\
	if (!dest || !src) return NULL_PTR;\
	dest->capacity += src->capacity;\
	T *tmp = realloc(dest->data, dest->capacity * sizeof(T));\
	if (!tmp) return REALLOC_ERROR;\
	dest->data = tmp;\
	memcpy(&dest->data[dest->len], src->data, src->len * sizeof(T));\
	dest->len += src->len;\
	return SUCCESS;\
}\

#define GEN_BOX_PRINT_IMPL(T, format)\
box_status_t print_box_##T(const box_##T##_t *box) {\
	if (!box) return NULL_PTR;\
	if (!box->len) return BOX_EMPTY;\
	for (int i = 0; i < (int)box->len; i++) {\
		printf(format "\n", box->data[i]);\
	}\
	return SUCCESS;\
}

#endif
