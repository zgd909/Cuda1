// Wywołanie:  bfs format method source target input_file output_file
#include "common.h"
#include "graph.h"
#include "bfs_cpu.h"
#include <string.h>

typedef enum { METHOD_CPU, METHOD_GPU1, METHOD_GPU2 } Method;

typedef struct {
    int undirected;          // 0 - txt_dir, 1 - txt_undir
    Method method;
    uint32_t source;
    uint32_t target;
    const char *input_path;
    const char *output_path;
} Options;

static void print_usage(const char *prog)
{
    printf("Usage: %s format method source target input_file output_file\n", prog);
    printf("  format       txt_dir   - directed graph in SNAP text format\n");
    printf("               txt_undir - undirected graph in SNAP text format\n");
    printf("  method       gpu1 | gpu2 - GPU variant, cpu - sequential reference version\n");
    printf("  source       number of the start vertex (non-negative integer)\n");
    printf("  target       number of the end vertex (non-negative integer)\n");
    printf("  input_file   path to the graph description\n");
    printf("  output_file  path to the result file (path vertices, one per line)\n");
    printf("Example: %s txt_dir cpu 0 100 data/web-Google.txt path.txt\n", prog);
}


static int parse_vertex_arg(const char *s, uint32_t *out)
{
    uint64_t val = 0; 
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9')
            return -1;                 
        val = val * 10 + (s[i] - '0');  
        if (val > MAX_VERTEX_ID)
            return -1;                  
    }
    if (i == 0)
        return -1;                      
    *out = (uint32_t)val;
    return 0;
}


static int parse_args(int argc, char **argv, Options *opts)
{
    if (argc != 7) {
        printf("Error: expected 6 arguments, got %d\n", argc - 1);
        return -1;
    }

    if (strcmp(argv[1], "txt_dir") == 0)
        opts->undirected = 0;
    else if (strcmp(argv[1], "txt_undir") == 0)
        opts->undirected = 1;
    else {
        printf("Error: unknown format '%s'\n", argv[1]);
        return -1;
    }

    if (strcmp(argv[2], "cpu") == 0)
        opts->method = METHOD_CPU;
    else if (strcmp(argv[2], "gpu1") == 0)
        opts->method = METHOD_GPU1;
    else if (strcmp(argv[2], "gpu2") == 0)
        opts->method = METHOD_GPU2;
    else {
        printf("Error: unknown method '%s'\n", argv[2]);
        return -1;
    }

    if (parse_vertex_arg(argv[3], &opts->source) != 0) {
        printf("Error: invalid source vertex '%s'\n", argv[3]);
        return -1;
    }
    if (parse_vertex_arg(argv[4], &opts->target) != 0) {
        printf("Error: invalid target vertex '%s'\n", argv[4]);
        return -1;
    }

    opts->input_path = argv[5];
    opts->output_path = argv[6];
    return 0;
}


static int write_path(const char *path_file, const Path *path)
{
    FILE *f = fopen(path_file, "wb");  //write binary (\n)
    if (!f) {
        printf("Error: cannot create output file '%s'\n", path_file);
        return -1;
    }
    for (uint32_t i = 0; i < path->length; i++)
        fprintf(f, "%u\n", path->vertices[i]);
    if (fclose(f) != 0) {
        printf("Error: failed writing output file '%s'\n", path_file);
        return -1;
    }
    return 0;
}

static const char *method_name(Method m)
{
    switch (m) {
    case METHOD_GPU1: return "gpu1";
    case METHOD_GPU2: return "gpu2";
    default: return "cpu";
    }
}

int main(int argc, char **argv)
{
    Options opts;
    if (parse_args(argc, argv, &opts) != 0) {
        print_usage(argv[0]);
        return 1;
    }

    printf("Graph: %s (%s), method: %s, source: %u, target: %u\n", opts.input_path,
           opts.undirected ? "undirected" : "directed", method_name(opts.method), opts.source, opts.target);

    
    Graph g;
    LoadTimes lt;
    double t_load0 = now_seconds();
    if (load_graph_txt(opts.input_path, opts.undirected, &g, &lt) != 0)
        return 1;
    double load_time = now_seconds() - t_load0;
    printf("Loaded graph: %u vertices, %llu edges\n", g.num_vertices, (unsigned long long)g.file_edges);
    printf("  [time] reading + parsing file: %10.4f s\n", lt.parse_time);
    printf("  [time] building CSR:           %10.4f s\n", lt.csr_time);

    
    Path path = {NULL, 0};
    double compute_time = 0.0;
    int status = 0;
    if (opts.source >= g.num_vertices || opts.target >= g.num_vertices) {
        printf("Warning: vertex number out of range (graph has vertices 0..%lld), no path\n",
               (long long)g.num_vertices - 1);
    } 
    else {
        double t0 = now_seconds();
        switch (opts.method) {
        case METHOD_CPU:
            status = bfs_cpu(&g, opts.source, opts.target, &path);
            break;
        case METHOD_GPU1:
            printf("Error: method '%s' is not implemented yet\n", method_name(opts.method));
            status = -1;
            break;
        case METHOD_GPU2:
            printf("Error: method '%s' is not implemented yet\n", method_name(opts.method));
            status = -1;
            break;
        }
        compute_time = now_seconds() - t0;
        printf("  [time] path search:            %10.4f s\n", compute_time);
    }
    free_graph(&g);

    if (status != 0) {
        free_path(&path);
        return 1;
    }

    if (path.length > 0)
        printf("Path found: %u vertices (%u edges)\n", path.length, path.length - 1);
    else
        printf("No path from %u to %u\n", opts.source, opts.target);

    
    double t_write0 = now_seconds();
    status = write_path(opts.output_path, &path);
    double write_time = now_seconds() - t_write0;
    free_path(&path);
    if (status != 0)
        return 1;

    printf("  [time] writing result:         %10.4f s\n", write_time);
    printf("Summary: load %.4f s, compute %.4f s, write %.4f s, total %.4f s\n", load_time, compute_time,write_time, load_time + compute_time + write_time);
    return 0;
}
