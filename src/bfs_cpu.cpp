#include "common.h"
#include "bfs_cpu.h"
#include <queue>

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
    if (!parent) {
        printf("Error: out of memory (CPU BFS)\n");
        return -1;
    }
    for (uint32_t v = 0; v < n; v++)
        parent[v] = NO_VERTEX;

    parent[source] = source;
    int found = (source == target);

    try { // push przy braku pamięci rzuca std::bad_alloc
        std::queue<uint32_t> queue;
        queue.push(source);

        while (!queue.empty() && !found) {
            uint32_t u = queue.front();
            queue.pop();
            for (uint32_t e = g->row_offsets[u]; e < g->row_offsets[u + 1]; e++) {
                uint32_t v = g->col_indices[e];
                if (parent[v] == NO_VERTEX) {
                    parent[v] = u;
                    if (v == target) {
                        found = 1;
                        break;
                    }
                    queue.push(v);
                }
            }
        }
    } catch (const std::bad_alloc &) {
        printf("Error: out of memory (CPU BFS queue)\n");
        free(parent);
        return -1;
    }

    int status = build_path_from_parents(parent, source, target, path);
    free(parent);
    return status;
}

void free_path(Path *path)
{
    free(path->vertices);
    path->vertices = NULL;
    path->length = 0;
}
