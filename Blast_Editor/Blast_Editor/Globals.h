#pragma once

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 )

#define CONSOLE_LOG(format, ...) log(__FILE__, __func__,__LINE__, false, false, false, format, __VA_ARGS__);
#define CONSOLE_WARNING(format, ...) log(__FILE__, __func__,__LINE__, true, false, false, format, __VA_ARGS__);
#define CONSOLE_ERROR(format, ...) log(__FILE__, __func__, __LINE__, false, true, false, format, __VA_ARGS__);
#define CONSOLE_DEBUG(format, ...) log(__FILE__, __func__, __LINE__, false, false, true, format, __VA_ARGS__);

void log(const char file[], const char function[], int line, bool is_warning, bool is_error, bool is_debug, const char* format, ...);

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

typedef unsigned int uint;

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define HALF_PI 1.57079632679489661923f
#define QUARTER_PI 0.78539816339744830961f
#define INV_PI 0.31830988618379067154f
#define INV_TWO_PI 0.15915494309189533576f

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

// Align 16, use if you have math elemtns in your class like float4x4 or AABB
#define ALIGN_CLASS_TO_16 \
	void* operator new(size_t i) { return _aligned_malloc(i,16); }\
    void operator delete(void* p) { _aligned_free(p); }

// Deletes a buffer
#define RELEASE( x )\
    {\
       if( x != nullptr )\
       {\
         delete x;\
	     x = nullptr;\
       }\
    }

// Deletes an array of buffers
#define RELEASE_ARRAY( x )\
	{\
       if( x != nullptr )\
       {\
           delete[] x;\
	       x = nullptr;\
		 }\
	 }