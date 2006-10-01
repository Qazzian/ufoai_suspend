#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdio.h>

#include "../../../config.h"

#if OSS_HEADER == "sys"
#include <sys/soundcard.h>
#else
#include <linux/soundcard.h>
#endif

#include "../../client/client.h"
#include "../../client/snd_loc.h"

qboolean OSS_SNDDMA_Init (void);
int OSS_SNDDMA_GetDMAPos (void);
void OSS_SNDDMA_Shutdown (void);
void OSS_SNDDMA_Submit (void);
void OSS_SNDDMA_BeginPainting(void);
