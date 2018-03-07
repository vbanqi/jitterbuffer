#ifndef __HOLA_UTILS_H_INCLUDED__
#define __HOLA_UTILS_H_INCLUDED__

#include <limits>

namespace hola
{

class Utils
{
public:
	Utils() {}
	virtual ~Utils() {}
    static inline bool IsMoreThan(uint16_t a, uint16_t b)
    {
        uint16_t ret = a - b;
        if (ret < std::numeric_limits<uint16_t>::max() / 2) {
            return true;
        }
        else {
            return false;
        }
    }
    static inline uint16_t GetDistance(uint16_t a, uint16_t b)
    {
        if (IsMoreThan(a, b)) {
            return a - b;
        }
        else {
            return b - a;
        }
    }
    
    static inline uint64_t SystemTimesMillis() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return (uint64_t)tv.tv_sec * 1000 + (uint64_t)tv.tv_usec / 1000; 
    }
}; // class Utils

} // namespace hola

#endif // ifndef __HOLA_UTILS_H_INCLUDED__

