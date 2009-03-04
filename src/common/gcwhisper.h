#ifndef _COMMON_GCWHISPER_H_
#define _COMMON_GCWHISPER_H_

#define	GCWHISPER_NOTOLD	0x1 /* Documents should not be removed, even though they are too old */

typedef unsigned int whisper_t;

whisper_t gcwhisper_read(char *subname);
void gcwhisper_write(char *subname, whisper_t whisper);


#endif
