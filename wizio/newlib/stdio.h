////////////////////////////////////////////////////////////////////////////////////////
//
//      2021 Georgi Angelov
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

#ifndef _PLATFORM_STDIO_H
#define _PLATFORM_STDIO_H
#ifdef __cplusplus
extern "C"
{
#endif
#include_next <stdio.h>

#include <stdbool.h>

/* if stdio is not used - weak */
void stdio_init_all(void) __attribute__((weak)); 

/* wait terminal */
bool stdio_usb_connected(void) __attribute__((weak));
#define stdio_connected() stdio_usb_connected()

#ifdef __cplusplus
}
#endif
#endif /* _PLATFORM_STDIO_H */