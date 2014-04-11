
#ifndef __COMMON_DEBUG_H__
#define __COMMON_DEBUG_H__

#ifdef DEBUG
	void debug(const char *fmt, ...);
	void bwarn(const char *fmt, ...);
#else
        #define debug(fmt, ...)
        #define bwarn(fmt, ...)
#endif

#endif	//  __COMMON_DEBUG_H__
