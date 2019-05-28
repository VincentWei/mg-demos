/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2010 ~ 2019 FMSoft (http://www.fmsoft.cn).
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*!============================================================================
 * @file utility.hh
 * @Synopsis
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef  __UTILITY_HH
#define  __UTILITY_HH

#include <string>

std::string stringReplace (const std::string& str, const std::string& oldVal, const std::string& newVal);

struct DeleteObject {
    template <typename T>
        void operator()(const T* ptr) const {
            delete ptr;
        }
};

#endif  /* __UTILITY_HH */

