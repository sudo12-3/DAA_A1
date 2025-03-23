*Graph Clique Listing Algorithms*

This repository contains implementations of three clique listing algorithms: Chiba_arboricity, ELS, and Tomita. 
These algorithms have been tested on three datasets: as-skitter, wiki-Vote, and email-Enron.

*Algorithms*

Chiba_arboricity: This algorithm uses the concept of arboricity to efficiently list cliques in a graph. It is particularly effective for dense graphs.

ELS: Another approach for listing maximal cliques, focusing on efficiency and scalability.

Tomita: A modified version of the Bron-Kerbosch algorithm, optimized for performance by using a pivoting strategy.

*Datasets*

as-skitter: An internet topology graph derived from traceroutes.

wiki-Vote: A social network dataset representing Wikipedia voting data.

email-Enron: A temporal network dataset of email communications within Enron.

Drive link for datasets: https://drive.google.com/drive/folders/1b8vsl_dlOhQd9axMAJ1O2U9T7fpQgLdU

*Execution Instructions:*


1. *Clone the Repository*:
   
   Start by cloning this repository to your local machine:

   bash
   git clone https://github.com/your-username/your-repository.git
   cd your-repository
   
2. **Compilation of the source code**: 
    bash
    
    g++ -o algorithm1 algorithm1.cpp

  where algorithm.cpp is the source file
                     
  algorithm is the name of the executable to be generated after compilation.



3.*To run the implementations on various datasets*:

    
         ./algorithmexe dataset.txt

where algorithmexe is the name of the executable file

dataset.txt is the dataset that the algorithm takes as input.



*Dataset Preparation*

Some of the datasets contain edges of the form (u,v) as well as (v,u). Consider only edges of the form (u,v) to create adjacency lists for all the nodes.


*Individual Contributions*

C++ Implementation of tomita -

C++ Implementation of Chiba_arboricity -

C++ Implementation of ELS -

Integrating the algorithms with the datsets -

Making the final report -

Creation of GitHub repository -