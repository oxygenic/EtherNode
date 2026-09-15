#include "fs.h"

/* leeres Dateisystem, damit der Compiler zufrieden ist */
const struct fsdata_file file_NULL[] = {
   { NULL, NULL, NULL, 0, 0 }
};

#define FS_ROOT file_NULL
#define FS_NUMFILES 0

