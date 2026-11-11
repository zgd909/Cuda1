#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"

// Graf w formacie CSR - (Compressed Sparse Row)
// Sąsiedzi wierzchołka v to col_indices[row_offsets[v] .. row_offsets[v+1]-1].
// Dla grafu nieskierowanego każda krawędź z pliku jest zapisana w obie strony.
typedef struct {
    uint32_t num_vertices; 
    uint32_t num_arcs;     
    uint64_t file_edges;   
    uint32_t *row_offsets; 
    uint32_t *col_indices; 
} Graph;


typedef struct {
    double parse_time; // odczyt pliku i parsowanie listy krawędzi
    double csr_time;   // budowa struktury CSR z listy krawędzi
} LoadTimes;


int load_graph_txt(const char *path, int undirected, Graph *g, LoadTimes *times);


void free_graph(Graph *g);

#endif 
