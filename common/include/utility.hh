/*!============================================================================
 * @file utility.hh 
 * @Synopsis  
 * @author DongKai
 * @version 1.0
 *  Company: Beijing Feynman Software Technology Co., Ltd.
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

