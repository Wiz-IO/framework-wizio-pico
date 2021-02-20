////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Georgi Angelov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////////////

#include <interface.h>

extern void *pvPortMalloc(size_t xWantedSize);
extern void vPortFree(void *pv);

void *malloc(size_t size)
{
    return pvPortMalloc(size);
}
void *_malloc_r(struct _reent *ignore, size_t size) { return malloc(size); }

void free(void *p)
{
    if (p)
        vPortFree(p);
}
void _free_r(struct _reent *ignore, void *ptr) { free(ptr); }

void *realloc(void *mem, size_t newsize)
{
    void *new = malloc(newsize);
    if ((new) && (mem))
        memcpy(new, mem, newsize);
    free(mem);
    return new;
}
void *_realloc_r(struct _reent *ignored, void *ptr, size_t size) { return realloc(ptr, size); }

void *calloc(size_t element, size_t size)
{
    size_t total = element * size;
    void *ret = malloc(total);
    if (ret)
        memset(ret, 0, total);
    return ret;
}
void *_calloc_r(struct _reent *ignored, size_t element, size_t size) { return calloc(element, size); }