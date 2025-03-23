**
Sriharish Ravichandran - 2022A7PS0511H

Parth Mehta - 2022A7PS0043H

Vishwas Vedantham - 2022A7PS0235H

Vadlamani Akhilesh - 2022A7PS0150H

Vinit Chordiya - 2022A7PS0148H

**
Webpage Link-
https://sudo12-3.github.io/DAA_A1/

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

   bash git clone https://github.com/sudo12-3/DAA_A1.git
   
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

C++ Implementation of tomita - Sriharish and Vishwas

C++ Implementation of Chiba_arboricity - Parth Mehta and Sriharish

C++ Implementation of ELS - Akhilesh and Vinith

Integrating the algorithms with the datsets - Parth and Vishwas

Making the final report - Akhilesh and Vinith

Creation of GitHub repository - Everyone