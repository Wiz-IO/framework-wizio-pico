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

#ifndef __MY_LOCK_H__
#define __MY_LOCK_H__

//#undef _RETARGETABLE_LOCKING
//#define _RETARGETABLE_LOCKING 0
#include_next <sys/lock.h>
//#warning LOCK TEST

#include "pico/mutex.h"
struct __lock
{
    mutex_t pico;
    int cnt;
};

typedef struct __lock S_Mutex;
typedef struct __lock *pS_Mutex;

#endif