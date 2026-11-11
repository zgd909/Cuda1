#include "common.h"
#include "bfs_cpu.h"

int build_path_from_parents(const uint32_t *parent, uint32_t source, uint32_t target, Path *path)
{
    path->vertices = NULL;
    path->length = 0;
    if (parent[target] == NO_VERTEX)
        return 0; 
    uint32_t len = 1;
    for (uint32_t v = target; v != source; v = parent[v])
        len++;

    path->vertices = (uint32_t *)malloc(len * sizeof(uint32_t));
    if (!path->vertices) {
        printf("Error: out of memory (path)\n");
        return -1;
    }
    uint32_t v = target;
    for (uint32_t i = len; i > 0; i--) {
        path->vertices[i - 1] = v;
        v = parent[v];
    }
    path->length = len;
    return 0;
}

int bfs_cpu(const Graph *g, uint32_t source, uint32_t target, Path *path)
{
    path->vertices = NULL;
    path->length = 0;

    uint32_t n = g->num_vertices;
    uint32_t *parent = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
    uint32_t *queue = (uint32_t *)malloc((size_t)n * sizeof(uint32_t));
    if (!parent || !queue) {
        printf("Error: out of memory (CPU BFS)\n");
        free(parent);
        free(queue);
        return -1;
    }
    for (uint32_t v = 0; v < n; v++)
        parent[v] = NO_VERTEX;

    // Każdy wierzchołek trafia do kolejki co najwyżej raz, więc tablica rozmiaru n wystarcza.
    uint32_t head = 0, tail = 0;
    parent[source] = source;
    queue[tail++] = source;
    int found = (source == target);

    while (head < tail && !found) {
        uint32_t u = queue[head++];
        for (uint32_t e = g->row_offsets[u]; e < g->row_offsets[u + 1]; e++) {
            uint32_t v = g->col_indices[e];
            if (parent[v] == NO_VERTEX) {
                parent[v] = u;
                if (v == target) { 
                    found = 1;
                    break;
                }
                queue[tail++] = v;
            }
        }
    }

    int status = build_path_from_parents(parent, source, target, path);
    free(parent);
    free(queue);
    return status;
}

void free_path(Path *path)
{
    free(path->vertices);
    path->vertices = NULL;
    path->length = 0;
}
