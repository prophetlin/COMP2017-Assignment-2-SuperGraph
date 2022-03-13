#ifndef SUPERGRAPH_LOADER_H
#define SUPERGRAPH_LOADER_H

#include <stdlib.h>
#include "supergraph.h"

user* user_loader(size_t* count, const char* filename);
post* post_loader(size_t* count, const char* filename);

#endif
