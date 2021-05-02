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

#include <interface.h>

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wsized-deallocation"

void *operator new(size_t size)
{
  return std::malloc(size);
}

void *operator new[](size_t size)
{
  return std::malloc(size);
}

void operator delete(void *ptr, __unused std::size_t n) noexcept
{
  std::free(ptr);
}

void operator delete[](void *ptr, __unused std::size_t n) noexcept
{
  std::free(ptr);
}

void operator delete(void *ptr) noexcept
{
  std::free(ptr);
}

void operator delete[](void *ptr) noexcept
{
  std::free(ptr);
}

//#pragma GCC diagnostic pop