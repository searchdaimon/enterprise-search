#ifndef _IR__H_
#define _IR__H_


#include "crc32.h"

#define calcDomainID(x) (crc32boitho(x) % 65535)
#define calcDomainID32(x) (crc32boitho(x))

#endif // _IR__H_
