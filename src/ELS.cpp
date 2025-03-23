#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>

using namespace std;

// Graph represented as adjacency lists
typedef vector<vector<int>> Graph;

// Global counters for statistics
int total_cliques = 0;
int max_clique_size = 0;
vector<int> largest_clique;

// Store mapping from internal indices to original node IDs
vector<int> index_to_node;

// Function to compute degeneracy ordering
vector<int> degeneracyOrdering(const Graph& graph) {
    int n = graph.size();
    vector<int> ordering;
    ordering.reserve(n);
    
    // Use a set to keep vertices sorted by degree
    vector<set<int>> verticesByDegree(n);
    vector<int> degree(n, 0);
    vector<bool> used(n, false);
    
    // Initialize degrees and verticesByDegree
    for (int i = 0; i < n; i++) {
        degree[i] = graph[i].size();
        verticesByDegree[degree[i]].insert(i);
    }
    
    // Extract vertices in order of degeneracy
    for (int i = 0; i < n; i++) {
        // Find vertex with minimum degree
        int d = 0;
        while (d < n && verticesByDegree[d].empty()) d++;
        
        // Get the vertex and remove it from verticesByDegree
        int v = *verticesByDegree[d].begin();
        verticesByDegree[d].erase(verticesByDegree[d].begin());
        used[v] = true;
        ordering.push_back(v);
        
        // Update degrees of neighbors
        for (int u : graph[v]) {
            if (!used[u]) {
                verticesByDegree[degree[u]].erase(u);
                degree[u]--;
                verticesByDegree[degree[u]].insert(u);
            }
        }
    }
    
    return ordering;
}

// Helper function to report a maximal clique
void reportMaximalClique(const vector<int>& R) {
    // Increment clique counter
    total_cliques++;
    
    // Check if this is the largest clique so far
    if (R.size() > max_clique_size) {
        max_clique_size = R.size();
        largest_clique = R;
    }
    
    // Print progress after every 100,000 cliques
    if (total_cliques % 100000 == 0) {
        cout << "Progress: " << total_cliques << " maximal cliques found so far. Current largest size: " 
             << max_clique_size << endl;
    }
    
    // Uncomment this if you want to print all maximal cliques (could be a lot!)
    /*
    for (int v : R) {
        cout << index_to_node[v] << " "; // Use original node IDs
    }
    cout << endl;
    */
}

// Function to check if a vertex is a neighbor efficiently
bool isNeighbor(const vector<int>& neighbors, int vertex) {
    // Binary search since we keep adjacency lists sorted
    return binary_search(neighbors.begin(), neighbors.end(), vertex);
}

// Modified BronKerboschPivot that uses references and iterators to avoid copying
void BronKerboschPivot(vector<int> P, vector<int>& R, vector<int> X, 
                       const Graph& graph) {
    if (P.empty() && X.empty()) {
        reportMaximalClique(R);
        return;
    }
    
    // Choose pivot to maximize |P ∩ Γ(u)|
    int pivot = -1;
    size_t max_connections = 0;
    
    // Consider pivots from both P and X
    vector<int> combined;
    combined.reserve(P.size() + X.size());
    combined.insert(combined.end(), P.begin(), P.end());
    combined.insert(combined.end(), X.begin(), X.end());
    
    for (int u : combined) {
        size_t connections = 0;
        for (int v : P) {
            // Check if v is a neighbor of u (in Γ(u))
            if (isNeighbor(graph[u], v)) {
                connections++;
            }
        }
        if (connections > max_connections) {
            max_connections = connections;
            pivot = u;
        }
    }
    
    // Create a copy of P to iterate over while modifying P
    vector<int> P_copy = P;
    
    // P \ Γ(pivot) - vertices in P that are not neighbors of pivot
    for (int v : P_copy) {
        if (pivot == -1 || !isNeighbor(graph[pivot], v)) {
            // Add v to R (current clique)
            R.push_back(v);
            
            // P ∩ Γ(v)
            vector<int> new_P;
            for (int u : P) {
                if (isNeighbor(graph[v], u)) {
                    new_P.push_back(u);
                }
            }
            
            // X ∩ Γ(v)
            vector<int> new_X;
            for (int u : X) {
                if (isNeighbor(graph[v], u)) {
                    new_X.push_back(u);
                }
            }
            
            // Recursive call
            BronKerboschPivot(new_P, R, new_X, graph);
            
            // Remove v from R
            R.pop_back();
            
            // Move v from P to X
            P.erase(remove(P.begin(), P.end(), v), P.end());
            X.push_back(v);
        }
    }
}

// Main algorithm as shown in Fig. 4
void BronKerboschDegeneracy(const Graph& graph) {
    int n = graph.size();
    
    // Get degeneracy ordering
    vector<int> ordering = degeneracyOrdering(graph);
    
    // Create ordering index map for O(1) lookup
    vector<int> orderingIndex(n);
    for (int i = 0; i < n; i++) {
        orderingIndex[ordering[i]] = i;
    }
    
    // Process each vertex in degeneracy order
    for (int i = 0; i < n; i++) {
        int vi = ordering[i];
        
        // P = Γ(vi) ∩ {vi+1, ..., vn-1}
        vector<int> P;
        for (int neighbor : graph[vi]) {
            // Check if neighbor comes later in ordering
            if (orderingIndex[neighbor] > i) {
                P.push_back(neighbor);
            }
        }
        
        // X = Γ(vi) ∩ {v0, ..., vi-1}
        vector<int> X;
        for (int neighbor : graph[vi]) {
            // Check if neighbor comes earlier in ordering
            if (orderingIndex[neighbor] < i) {
                X.push_back(neighbor);
            }
        }
        
        // Initialize R with just {vi}
        vector<int> R = {vi};
        
        // Call BronKerboschPivot
        BronKerboschPivot(P, R, X, graph);
    }
}

// Read graph from file with proper handling of non-sequential node IDs
pair<Graph, vector<int>> readGraph(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + filename);
    }
    
    string line;
    int n = 0;
    
    // Skip comments and get number of nodes
    while (getline(file, line)) {
        if (line[0] != '#') break;
        
        // Extract number of nodes from the comment
        if (line.find("Nodes:") != string::npos) {
            size_t pos = line.find("Nodes:") + 6;
            while (pos < line.length() && line[pos] == ' ') pos++;
            sscanf(line.c_str() + pos, "%d", &n);
        }
    }
    
    // First pass: collect all unique node IDs
    set<int> unique_nodes;
    
    // Process the first non-comment line we already read
    int u, v;
    if (sscanf(line.c_str(), "%d %d", &u, &v) == 2) {
        unique_nodes.insert(u);
        unique_nodes.insert(v);
    }
    
    // Read remaining edges and collect unique node IDs
    while (file >> u >> v) {
        unique_nodes.insert(u);
        unique_nodes.insert(v);
    }
    
    // Create a mapping from original node IDs to consecutive indices
    unordered_map<int, int> node_to_index;
    vector<int> index_to_node_map;
    int index = 0;
    for (int node : unique_nodes) {
        node_to_index[node] = index;
        index_to_node_map.push_back(node);
        index++;
    }
    
    // Use the actual number of unique nodes
    n = unique_nodes.size();
    
    cout << "Unique nodes count: " << n << endl;
    
    // Create graph with n nodes
    Graph graph(n);
    
    // Reset file to beginning and skip comments again
    file.clear();
    file.seekg(0);
    while (getline(file, line) && line[0] == '#');
    
    // Process the first non-comment line again
    if (sscanf(line.c_str(), "%d %d", &u, &v) == 2) {
        graph[node_to_index[u]].push_back(node_to_index[v]);
        // For undirected graphs, add both directions
        graph[node_to_index[v]].push_back(node_to_index[u]);
    }
    
    // Read remaining edges
    while (file >> u >> v) {
        graph[node_to_index[u]].push_back(node_to_index[v]);
        // For undirected graphs, add both directions
        graph[node_to_index[v]].push_back(node_to_index[u]);
    }
    
    // Remove duplicate edges and sort adjacency lists
    for (auto& adj : graph) {
        sort(adj.begin(), adj.end());
        adj.erase(unique(adj.begin(), adj.end()), adj.end());
    }
    
    return {graph, index_to_node_map};
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }
    
    string filename = argv[1];
    
    try {
        // Get graph and mapping
        auto [graph, node_mapping] = readGraph(filename);
        index_to_node = node_mapping; // Set global mapping
        
        cout << "Graph loaded: " << graph.size() << " nodes" << endl;
        cout << "Starting maximal clique enumeration..." << endl;
        
        // Reset statistics
        total_cliques = 0;
        max_clique_size = 0;
        
        // Start timer
        clock_t start = clock();
        
        BronKerboschDegeneracy(graph);
        
        // End timer
        clock_t end = clock();
        double time_taken = double(end - start) / CLOCKS_PER_SEC;
        
        // Print statistics
        cout << "Total maximal cliques found: " << total_cliques << endl;
        cout << "Size of largest clique: " << max_clique_size << endl;
        cout << "Largest clique (original node IDs): ";
        for (int v : largest_clique) {
            cout << index_to_node[v] << " ";  // Map back to original node IDs
        }
        cout << endl;
        cout << "Time taken: " << time_taken << " seconds" << endl;
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}