#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <chrono>

// Maximum number of vertices (adjusted for 10 lakh nodes)
const int MAX_VERTICES = 1000000;
// Progress reporting interval (1 lakh = 100,000)
const int PROGRESS_INTERVAL = 10000;

class Graph {
private:
    int V;                                      // Number of vertices
    std::vector<std::unordered_set<int>> adj;   // Adjacency list
    std::vector<int> Q;                         // Current clique
    int maxCliqueSize;                          // Track the largest clique size
    long long cliqueCount;                      // Count total cliques found
    long long lastReportedCount;                // Last reported count for progress tracking

public:
    Graph(int vertices) : V(vertices), maxCliqueSize(0), cliqueCount(0), lastReportedCount(0) {
        adj.resize(vertices);
    }

    // Add an edge between vertices u and v
    void addEdge(int u, int v) {
        // Ensure vertices are valid
        if (u >= V) resizeGraph(u + 1);
        if (v >= V) resizeGraph(v + 1);

        adj[u].insert(v);
        adj[v].insert(u); // For undirected graph
    }

    // Resize graph if needed
    void resizeGraph(int newSize) {
        adj.resize(newSize);
        V = newSize;
    }

    // Main function to find all maximal cliques - returns statistics only
    std::pair<long long, int> findMaximalCliques() {
        Q.clear();
        cliqueCount = 0;
        lastReportedCount = 0;
        maxCliqueSize = 0;

        // Create initial sets for EXPAND
        std::vector<bool> SUBG(V, true);
        std::vector<bool> CAND(V, true);

        // Find cliques without writing to file
        EXPAND(SUBG, CAND);

        // Report final progress if not already reported
        if (cliqueCount > lastReportedCount) {
            std::cout << "Progress: " << cliqueCount << " cliques found, current max size: " 
                      << maxCliqueSize << std::endl;
        }

        return {cliqueCount, maxCliqueSize};
    }

    // Return number of vertices
    int getVertexCount() const {
        return V;
    }

private:
    // Optimized EXPAND procedure without file I/O
    void EXPAND(std::vector<bool>& SUBG, std::vector<bool>& CAND) {
        // Check if SUBG is empty
        bool isEmpty = true;
        for (int i = 0; i < V; i++) {
            if (SUBG[i]) {
                isEmpty = false;
                break;
            }
        }

        // If SUBG is empty, we found a maximal clique
        if (isEmpty) {
            cliqueCount++;
            maxCliqueSize = std::max(maxCliqueSize, static_cast<int>(Q.size()));
            
            // Check if we need to report progress
            if (cliqueCount >= lastReportedCount + PROGRESS_INTERVAL) {
                lastReportedCount = cliqueCount;
                std::cout << "Progress: " << cliqueCount << " cliques found, current max size: " 
                          << maxCliqueSize << std::endl;
            }
            return;
        }

        // Select u in SUBG that maximizes |CAND ∩ Γ(u)|
        int u = -1;
        int maxSize = -1;

        for (int i = 0; i < V; i++) {
            if (!SUBG[i]) continue;
            
            int size = 0;
            for (int neighbor : adj[i]) {
                if (neighbor < V && CAND[neighbor]) {
                    size++;
                }
            }
            
            if (size > maxSize) {
                maxSize = size;
                u = i;
            }
        }

        // Process candidates = CAND - Γ(u)
        std::vector<int> candidates;
        
        // Find vertices in CAND that are not neighbors of u
        for (int i = 0; i < V; i++) {
            if (CAND[i] && adj[u].find(i) == adj[u].end()) {
                candidates.push_back(i);
            }
        }

        // Process candidates
        for (int q : candidates) {
            // Add q to Q
            Q.push_back(q);
            
            // Create SUBG_q and CAND_q in-place
            std::vector<bool> SUBG_q(V, false);
            std::vector<bool> CAND_q(V, false);
            
            // Only vertices that are in both original sets and are neighbors of q
            for (int neighbor : adj[q]) {
                if (neighbor < V) {
                    if (SUBG[neighbor]) SUBG_q[neighbor] = true;
                    if (CAND[neighbor]) CAND_q[neighbor] = true;
                }
            }
            
            // Recursive call
            EXPAND(SUBG_q, CAND_q);
            
            // Remove q from CAND
            CAND[q] = false;
            
            // Remove q from Q (backtracking)
            Q.pop_back();
        }
    }
};

// Function to read graph from file - minimal output
bool readGraphFromFile(const std::string& filename, Graph& graph) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return false;
    }

    std::string line;
    int maxNodeId = -1;
    int edgeCount = 0;

    // Skip comment lines
    while (std::getline(file, line)) {
        if (line.empty() || line[0] != '#') {
            break;
        }
    }

    // Process edge lines
    do {
        if (line.empty()) continue;

        std::istringstream iss(line);
        int from, to;

        if (!(iss >> from >> to)) {
            std::cerr << "Error: Invalid format in line: " << line << std::endl;
            continue;
        }

        maxNodeId = std::max(maxNodeId, std::max(from, to));
        graph.addEdge(from, to);
        edgeCount++;

    } while (std::getline(file, line));

    file.close();

    std::cout << "Graph loaded: " << (maxNodeId + 1) << " vertices, " 
              << edgeCount << " edges" << std::endl;
    
    return true;
}

int main(int argc, char* argv[]) {
    std::string inputFilename = "wiki-Vote.txt";
    std::string outputFilename = "clique_results_summary.txt";

    if (argc > 1) inputFilename = argv[1];
    if (argc > 2) outputFilename = argv[2];

    // Initialize graph
    Graph g(0);

    auto loadStart = std::chrono::high_resolution_clock::now();
    if (!readGraphFromFile(inputFilename, g)) {
        return 1;
    }
    auto loadEnd = std::chrono::high_resolution_clock::now();
    auto loadDuration = std::chrono::duration_cast<std::chrono::seconds>(loadEnd - loadStart);
    
    std::cout << "Graph loaded in " << loadDuration.count() << " seconds" << std::endl;
    std::cout << "Finding maximal cliques..." << std::endl;

    // Record start time
    auto globalStartTime = std::chrono::high_resolution_clock::now();

    // Find cliques - only collecting statistics
    auto [cliqueCount, maxCliqueSize] = g.findMaximalCliques();

    // Record end time
    auto globalEndTime = std::chrono::high_resolution_clock::now();
    auto globalDuration = std::chrono::duration_cast<std::chrono::seconds>(globalEndTime - globalStartTime);

    // Only write summary to output file at the end
    std::ofstream outputFile(outputFilename);
    if (outputFile.is_open()) {
        outputFile << "Graph: " << inputFilename << std::endl;
        outputFile << "Vertices: " << g.getVertexCount() << std::endl;
        outputFile << "Total maximal cliques found: " << cliqueCount << std::endl;
        outputFile << "Size of largest clique: " << maxCliqueSize << std::endl;
        outputFile << "Processing time: " << globalDuration.count() << " seconds" << std::endl;
        outputFile.close();
    }

    std::cout << "Total maximal cliques: " << cliqueCount << std::endl;
    std::cout << "Largest clique size: " << maxCliqueSize << std::endl;
    std::cout << "Total execution time: " << globalDuration.count() << " seconds" << std::endl;

    return 0;
}