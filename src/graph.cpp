
#include "common.h"
#include "graph.h"
#include <string.h>
#include <vector>


#define MAX_LINE 256 // bufor na jedną linię 


struct EdgeList {
    std::vector<uint32_t> src;
    std::vector<uint32_t> dst;
    uint32_t max_id = 0; // największy numer wierzchołka
};


static void push_edge(EdgeList *el, uint32_t u, uint32_t v)
{
    el->src.push_back(u);
    el->dst.push_back(v);
    if (u > el->max_id)
        el->max_id = u;
    if (v > el->max_id)
        el->max_id = v;
}



static int read_number(const char *line, int &i, uint32_t *out)
{
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    if (line[i] < '0' || line[i] > '9')
        return -1;
    uint64_t val = 0; 
    while (line[i] >= '0' && line[i] <= '9') {
        val = val * 10 + (line[i] - '0');
        if (val > MAX_VERTEX_ID)
            return -1;
        i++;
    }
    *out = (uint32_t)val;
    return 0;
}


static int read_edge_list(const char *path, EdgeList *el)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        printf("Error: cannot open input file '%s'\n", path);
        return -1;
    }

    char line[MAX_LINE];
    uint64_t line_no = 0;
    int status = 0;
    try { 
        while (fgets(line, sizeof(line), f)) { 
            line_no++;

            // Linia dłuższa niż bufor - mozna pominac
            if (strchr(line, '\n') == NULL) {
                int c;
                while ((c = fgetc(f)) != '\n' && c != EOF) {
                }
            }

            int i = 0;
            while (line[i] == ' ' || line[i] == '\t')
                i++;
            if (line[i] == '#' || line[i] == '\n' || line[i] == '\r' || line[i] == '\0')
                continue; // komentarz albo pusta linia

            uint32_t u, v;
            if (read_number(line, i, &u) != 0 || read_number(line, i, &v) != 0) {
                printf("Error: malformed line %llu in input file\n", (unsigned long long)line_no);
                status = -1;
                break;
            }
            push_edge(el, u, v);
        }
    } catch (const std::bad_alloc &) {
        printf("Error: out of memory while reading edges (%zu edges read)\n", el->src.size());
        status = -1;
    }

    if (status == 0 && ferror(f)) {
        printf("Error: failed reading input file '%s'\n", path);
        status = -1;
    }
    fclose(f);
    return status;
}


static int build_csr(const EdgeList *el, int undirected, Graph *g)
{
    size_t count = el->src.size(); 
    uint64_t n = count ? (uint64_t)el->max_id + 1 : 0;
    uint64_t arcs = undirected ? 2 * (uint64_t)count : (uint64_t)count;
    if (arcs > UINT32_MAX) {
        printf("Error: graph too large (%llu arcs, limit is %u)\n", (unsigned long long)arcs, UINT32_MAX);
        return -1;
    }

    g->num_vertices = (uint32_t)n;
    g->num_arcs = (uint32_t)arcs;
    g->file_edges = count;
    g->row_offsets = (uint32_t *)calloc(n + 1, sizeof(uint32_t));
    g->col_indices = (uint32_t *)malloc((arcs ? arcs : 1) * sizeof(uint32_t));
    if (!g->row_offsets || !g->col_indices) {
        printf("Error: out of memory while building CSR\n");
        free_graph(g);
        return -1;
    }

    uint32_t *off = g->row_offsets;
    uint32_t *col = g->col_indices;

    // Zliczenie stopni wyjściowych: off[v+1] = deg(v).
    for (size_t i = 0; i < count; i++) {
        off[el->src[i] + 1]++;
        if (undirected)
            off[el->dst[i] + 1]++;
    }
    //Suma prefiksowa: off[v] = początek listy sąsiadów v.
    for (uint64_t v = 0; v < n; v++)
        off[v + 1] += off[v];
   
    for (size_t i = 0; i < count; i++) {
        uint32_t u = el->src[i], v = el->dst[i];
        col[off[u]++] = v;
        if (undirected)
            col[off[v]++] = u;
    }
    
    //przywracamy poprawnie off[v]
    for (uint64_t v = n; v > 0; v--)
        off[v] = off[v - 1];
    off[0] = 0;
    return 0;
}

int load_graph_txt(const char *path, int undirected, Graph *g, LoadTimes *times)
{
    memset(g, 0, sizeof(*g));
    double t0 = now_seconds();
    double t1, t2;
    int status;
    {
        EdgeList el; 
        if (read_edge_list(path, &el) != 0)
            return -1;
        t1 = now_seconds();
        status = build_csr(&el, undirected, g);
        t2 = now_seconds();
    } 

    times->parse_time = t1 - t0;
    times->csr_time = t2 - t1;
    return status;
}

void free_graph(Graph *g)
{
    free(g->row_offsets);
    free(g->col_indices);
    g->row_offsets = NULL;
    g->col_indices = NULL;
    g->num_vertices = 0;
    g->num_arcs = 0;
}
