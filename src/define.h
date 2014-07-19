/* Copyright zhanyonhu@163.com, Inc. and other Node contributors. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#ifndef _DEFINE_H_
#define _DEFINE_H_

#define MAX_TASK_COUNT						10
#define DEAULT_THREAD_NUM					4
#define DEAULT_TASK_ADD						(MAX_TASK_COUNT/10)
#define CHECK_DELETE_COUNT					(MAX_TASK_COUNT*DEAULT_THREAD_NUM/10)

#define TIMEOUT_FOR_RELEASE					(MAX_TASK_COUNT/10000>0? (MAX_TASK_COUNT/10000) : 1)		/*seconds*/
#ifdef _DEBUG
#define TIMER_TIME_RELASE					10000
#define TIMER_TIME_ADD						10
#else
#define TIMER_TIME_RELASE					10000
#define TIMER_TIME_ADD						100
#endif


#endif /* _DEFINE_H_ */
