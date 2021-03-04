////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////

#ifndef __VARIANT_H__
#define __VARIANT_H__

#include <interface.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* C ware          */

#define PINS_COUNT 30

#define LED 25

    typedef enum
    {
        ADC_0,
        ADC_1,
        ADC_2,
        ADC_3,
        ADC_T
    } adc_channel_e;

#ifdef __cplusplus
}

/* CPP ware */

#include <Uart.h>
extern Uart Serial;
extern Uart Serial1;

#endif //__cplusplus

#endif /* __VARIANT_H__ */
