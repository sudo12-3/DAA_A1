/*
Info:
Feel free to use these lines as you wish.
This program iterates over all k-cliques.
This is an implementation of the 1985 paper of Chiba And Nishizeki "Arboricity and subgraph listing".

To compile:
"g++ Arboricity.cpp -O3 -o Arboricity".

To execute:
"./Arboricity edgelist.txt k".
"edgelist.txt" should contain the graph: one edge on each line separated by a space.
Will print the number of k-cliques.
*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>
#include <iomanip>

#define hi(i) printf("hi #%u\n", i);

#define NLINKS 100000000 // maximum number of edges for memory allocation, will increase if needed

typedef struct
{
    unsigned s;
    unsigned t;
} edge;

typedef struct
{
    unsigned node;
    unsigned deg;
} nodedeg;

typedef struct
{
    unsigned n;  // number of nodes
    unsigned e;  // number of edges
    edge *edges; // list of edges

    unsigned *ns;  // ns[l]: number of nodes in G_l
    unsigned **d;  // d[l]: degrees of G_l
    unsigned *cd;  // cumulative degree: (start with 0) length=n+1
    unsigned *adj; // list of neighbors with lower degree

    unsigned char *lab; // lab[i] label of node i
    unsigned **sub;     // sub[l]: nodes in G_l

    // Add fields to maintain the mapping between original IDs and internal indices
    std::vector<unsigned> index_to_node; // Mapping from internal index to original node ID
} specialsparse;

void freespecialsparse(specialsparse *g, unsigned char k)
{
    unsigned char i;
    free(g->ns);
    for (i = 2; i < k + 1; i++)
    {
        free(g->d[i]);
        free(g->sub[i]);
    }
    free(g->d);
    free(g->sub);
    free(g->cd);
    free(g->adj);
    free(g);
}

// Compute the maximum of three unsigned integers.
inline unsigned int max3(unsigned int a, unsigned int b, unsigned int c)
{
    a = (a > b) ? a : b;
    return (a > c) ? a : c;
}

// Improved readedgelist function that handles non-sequential node IDs
specialsparse *readedgelist(const char *edgelist) {
    specialsparse *g = (specialsparse*)malloc(sizeof(specialsparse));
    FILE *file;

    file = fopen(edgelist, "r");
    if (!file) {
        std::cerr << "Error: Cannot open file " << edgelist << std::endl;
        exit(1);
    }

    // First pass: collect all unique node IDs
    std::set<unsigned> unique_nodes;
    unsigned s, t;
    while (fscanf(file, "%u %u", &s, &t) == 2) {
        unique_nodes.insert(s);
        unique_nodes.insert(t);
    }

    // Use the actual number of unique nodes
    g->n = unique_nodes.size();
    std::cout << "Unique nodes count: " << g->n << std::endl;

    // Create a mapping from original node IDs to consecutive indices
    g->index_to_node.resize(g->n);
    std::unordered_map<unsigned, unsigned> node_to_index;
    
    unsigned index = 0;
    for (unsigned node : unique_nodes) {
        node_to_index[node] = index;
        g->index_to_node[index] = node;
        index++;
    }

    // Reset file to beginning for second pass
    rewind(file);
    
    // Second pass: read edges and store them with mapped indices
    unsigned e1 = NLINKS;
    g->edges = (edge*)malloc(e1 * sizeof(edge));
    g->e = 0;
    
    // Use a set to efficiently detect duplicates
    std::set<std::pair<unsigned, unsigned>> edge_set;
    
    while (fscanf(file, "%u %u", &s, &t) == 2) {
        unsigned mapped_s = node_to_index[s];
        unsigned mapped_t = node_to_index[t];
        
        // Ensure s < t to treat (s, t) and (t, s) as the same edge
        if (mapped_s > mapped_t) {
            std::swap(mapped_s, mapped_t);
        }
        
        // Check if the edge already exists
        auto edge_pair = std::make_pair(mapped_s, mapped_t);
        if (edge_set.find(edge_pair) == edge_set.end()) {
            edge_set.insert(edge_pair);
            
            // Add edge to our list
            g->edges[g->e].s = mapped_s;
            g->edges[g->e].t = mapped_t;
            g->e++;
            
            if (g->e == e1) {
                e1 += NLINKS;
                g->edges = (edge*)realloc(g->edges, e1 * sizeof(edge));
            }
        }
    }
    fclose(file);
    
    // Resize the edges array to the actual number of edges
    g->edges = (edge*)realloc(g->edges, g->e * sizeof(edge));
    
    std::cout << "Graph loaded: " << g->n << " nodes, " << g->e << " edges" << std::endl;
    
    return g;
}

// Building the special graph structure
void mkspecial(specialsparse *g, unsigned char k)
{
    unsigned i, ns, max;
    unsigned *d, *sub;
    unsigned char *lab;

    d = (unsigned*)calloc(g->n, sizeof(unsigned));

    for (i = 0; i < g->e; i++)
    {
        d[g->edges[i].s]++;
        d[g->edges[i].t]++;
    }

    g->cd = (unsigned*)malloc((g->n + 1) * sizeof(unsigned));
    ns = 0;
    g->cd[0] = 0;
    max = 0;
    sub = (unsigned*)malloc(g->n * sizeof(unsigned));
    lab = (unsigned char*)malloc(g->n * sizeof(unsigned char));
    for (i = 1; i < g->n + 1; i++)
    {
        g->cd[i] = g->cd[i - 1] + d[i - 1];
        if (d[i - 1] > 0)
        {
            max = (max > d[i - 1]) ? max : d[i - 1];
            sub[ns] = i - 1;
            ns++;
            d[i - 1] = 0;
            lab[i - 1] = k;
        }
    }

    g->adj = (unsigned*)malloc(2 * g->e * sizeof(unsigned));

    for (i = 0; i < g->e; i++)
    {
        g->adj[g->cd[g->edges[i].s] + d[g->edges[i].s]++] = g->edges[i].t;
        g->adj[g->cd[g->edges[i].t] + d[g->edges[i].t]++] = g->edges[i].s;
    }
    free(g->edges);

    g->ns = (unsigned*)malloc((k + 1) * sizeof(unsigned));
    g->ns[k] = ns;

    g->d = (unsigned**)malloc((k + 1) * sizeof(unsigned*));
    g->sub = (unsigned**)malloc((k + 1) * sizeof(unsigned*));
    for (i = 2; i < k; i++)
    {
        g->d[i] = (unsigned*)malloc(g->n * sizeof(unsigned));
        g->sub[i] = (unsigned*)malloc(max * sizeof(unsigned));
    }
    g->d[k] = d;
    g->sub[k] = sub;

    g->lab = lab;
}

void arg_bucket_sort(unsigned *key, unsigned n, unsigned *val)
{
    unsigned i, j;
    static unsigned *c = NULL, *cc = NULL, *key2 = NULL;
    if (c == NULL)
    {
        c = (unsigned*)malloc(n * sizeof(unsigned));    // count
        cc = (unsigned*)malloc(n * sizeof(unsigned));   // cummulative count
        key2 = (unsigned*)malloc(n * sizeof(unsigned)); // sorted array
    }
    memset(c, 0, n * sizeof(unsigned));

    for (i = 0; i < n; i++)
    {
        (c[val[key[i]]])++;
    }
    cc[0] = 0;
    for (i = 1; i < n; i++)
    {
        cc[i] = cc[i - 1] + c[i - 1];
        c[i - 1] = 0;
    }
    c[i - 1] = 0;

    for (i = 0; i < n; i++)
    {
        j = val[key[i]];
        key2[cc[j] + c[j]++] = key[i];
    }

    // Copy sorted data back to original array
    for (i = 0; i < n; i++) {
        key[i] = key2[i];
    }
}

void kclique(unsigned l, specialsparse *g, unsigned long long *n)
{
    unsigned i, j, k, end, u, v, w;

    if (l == 2)
    {
        for (i = 0; i < g->ns[2]; i++)
        { // list all edges
            u = g->sub[2][i];
            end = g->cd[u] + g->d[2][u];
            for (j = g->cd[u]; j < end; j++)
            {
                v = g->adj[j];
                if (v < u)
                {
                    (*n)++; // listing here!!!
                }
            }
        }
        return;
    }

    arg_bucket_sort(g->sub[l], g->ns[l], g->d[l]);

    for (i = 0; i < g->ns[l]; i++)
    {
        u = g->sub[l][i];
        g->ns[l - 1] = 0;
        end = g->cd[u] + g->d[l][u];
        for (j = g->cd[u]; j < end; j++)
        { // relabeling nodes and forming U'.
            v = g->adj[j];
            if (g->lab[v] == l)
            {
                g->lab[v] = l - 1;
                g->sub[l - 1][g->ns[l - 1]++] = v;
                g->d[l - 1][v] = 0; // new degrees
            }
        }
        for (j = 0; j < g->ns[l - 1]; j++)
        { // reodering adjacency list and computing new degrees
            v = g->sub[l - 1][j];
            end = g->cd[v] + g->d[l][v];
            for (k = g->cd[v]; k < end; k++)
            {
                w = g->adj[k];
                if (g->lab[w] == l - 1)
                {
                    g->d[l - 1][w]++;
                }
                else
                {
                    g->adj[k--] = g->adj[--end];
                    g->adj[end] = w;
                }
            }
        }

        kclique(l - 1, g, n);

        for (j = 0; j < g->ns[l - 1]; j++)
        { // moving u to last position in each entry of the adjacency list
            v = g->sub[l - 1][j];
            g->lab[v] = l;
            end = g->cd[v] + g->d[l - 1][v];
            for (k = g->cd[v]; k < end; k++)
            {
                w = g->adj[k];
                if (w == u)
                {
                    g->adj[k] = g->adj[--end];
                    g->adj[end] = w;
                    g->d[l - 1][v]--;
                    break;
                }
            }
        }
        g->lab[u] = l + 1;
    }

    return;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " edgelist.txt" << std::endl;
        return 1;
    }

    unsigned long long* cliqueCount = NULL;
    unsigned max_k = 100;
    cliqueCount = (unsigned long long*)malloc((max_k + 1) * sizeof(unsigned long long));
    if (cliqueCount == NULL) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 1;
    }
    cliqueCount[0] = 0;
    unsigned char k = 2;
    clock_t totalStart, totalEnd;
    totalStart = clock();
    unsigned long long sum = 0;
    while (1)
    {
        specialsparse *g;
        unsigned long long n;
        clock_t iterStart, iterEnd;
        iterStart = clock();
        g = readedgelist(argv[1]);
        
        if (k == 2) {
            cliqueCount[1] = g->n;
        }
        mkspecial(g, k);
        
        n = 0;
        kclique(k, g, &n);
        
        if (k >= max_k) {
            max_k *= 2;
            cliqueCount = (unsigned long long*)realloc(cliqueCount, (max_k + 1) * sizeof(unsigned long long));
            if (cliqueCount == NULL) {
                std::cerr << "Memory reallocation failed" << std::endl;
                return 1;
            }
        }
        cliqueCount[k] = n;
        
        iterEnd = clock();
        double elapsed = (double)(iterEnd - iterStart) / CLOCKS_PER_SEC;
        std::cout << "# Number of " << (unsigned)k << "-cliques: " << n << " (" << elapsed << " seconds)" << std::endl;
        sum += n;
        freespecialsparse(g, k);
        k++;
        if (n == 0)
        {
            std::cout << "\n[SUMMARY] All " << sum << " cliques found." << std::endl;
            std::cout << "Maximal Clique Size : " << (unsigned)(k-2) << std::endl;
            std::cout << "-------------------------------------" << std::endl;
            std::cout << "| Clique Size | Count               |" << std::endl;
            std::cout << "-------------------------------------" << std::endl;
            for (unsigned i = 1; i < k-1; i++) {
                std::cout << "| " << std::left << std::setw(11) << i << " | " << std::setw(20) << cliqueCount[i] << " |" << std::endl;
            }
            std::cout << "-------------------------------------" << std::endl;
            
            totalEnd = clock();
            double totalElapsed = (double)(totalEnd - totalStart) / CLOCKS_PER_SEC;
            std::cout << "\n[INFO] Total Execution Time: " << totalElapsed << " seconds" << std::endl;
            free(cliqueCount);
            break;
        }
    }
    return 0;
}