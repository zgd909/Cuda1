#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Największy dopuszczalny numer wierzchołka. Wartość UINT32_MAX jest zarezerwowana
// jako znacznik "brak rodzica / nieodwiedzony" w tablicach BFS.
#define MAX_VERTEX_ID (UINT32_MAX - 1u)
#define NO_VERTEX UINT32_MAX


double now_seconds();

#endif 
