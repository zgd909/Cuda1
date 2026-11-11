#ifndef BFS_CPU_H
#define BFS_CPU_H

#include "graph.h"

// Wynik wyszukiwania ścieżki: wierzchołki od source do target włącznie.
// length == 0 oznacza brak ścieżki.
typedef struct {
    uint32_t *vertices;
    uint32_t length;
} Path;

int bfs_cpu(const Graph *g, uint32_t source, uint32_t target, Path *path);


int build_path_from_parents(const uint32_t *parent, uint32_t source, uint32_t target, Path *path);

void free_path(Path *path);

#endif 
