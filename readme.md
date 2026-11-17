2η Προγραμματιστική Εργασία, Κ23γ

Ευάγγελος Ταμπαχανιώτης
ΑΜ: 1115202100181

Δημήτριος Μαλλίδης
ΑΜ: 1115202300109

Language: C++20

Compilation: Makefile with g++, flags -std=c++20 -Wall -Wextra -Wpedantic -O2

OpenCV used only for reading images/extracting SIFT descriptor.

This assignment extends our first assignment:
https://github.com/DimMallidis/EMIRIS_ERGASIA_1

The current revision contains the original search baseline. The preprocessing,
BoVW representation and five search methods are reused from assignment 1.
Geometric verification will be added in separate commits.

We use the same include/src layout, Makefile, code formatting and readme style
as assignment 1. The report will use the same IEEE Conference format.

The proposed responsibilities are descriptor matching, command-line options,
output, reranking, experiments and documentation for Evangelos, and feature
storage, DLT, RANSAC, geometric evaluation and integration for Dimitrios.

Implementation files:
- "src/main.cpp": validates the configuration, prepares the data, builds the selected index, runs the queries, and writes the results.
- "src/config.cpp": parses command-line options and reads the file that assigns sequences to the train, validation, and test sets.
- "src/preprocess.cpp": reads images, extracts SIFT descriptors, builds the visual-word vocabulary, and converts images into normalized BoVW vectors.
- "src/cached_preprocess.cpp": loads preprocessing results from cache or runs preprocessing when the required cache is not available.
- "src/vector_cache.cpp": stores and loads vectors and their image IDs in binary cache files.
- "src/distance.cpp": provides squared Euclidean distance calculations used by all search methods.
- "src/kmeans.cpp": implements seeded k-means++ initialization and Lloyd iterations for clustering vectors.
- "src/index.cpp": selects and constructs the search index requested by the user.
- "src/exact.cpp": searches the complete database and returns the nearest neighbors by exact distance.
- "src/lsh.cpp": builds hash tables with random projections and searches the candidate points found in those tables.
- "src/hypercube.cpp": projects vectors into a binary hypercube and searches selected nearby vertices.
- "src/ivfflat.cpp": trains coarse centroids, assigns every database vector to an inverted list, and searches selected lists exactly.
- "src/ivfpq.cpp": builds inverted lists and product-quantization codebooks, then ranks candidates using approximate distances.
- "src/projection.cpp": generates seeded random projections and computes the hash values used by LSH and Hypercube.
- "src/metrics.cpp": identifies relevant images and calculates Recall@5, Recall@10, AP@10, and mean values.
- "src/evaluation.cpp": executes every query, measures only the search time, maps result indices to image IDs, and computes metrics.
- "src/output.cpp": writes per-query rankings, metric values, mean results, and timing summaries.

Build the project with the Makefile:
make

This creates the executable:
./search

To remove the executable and build files:
make clean

The following example runs the IVFFlat search method on the validation set,
using 10,000 MIRFlickr images, 10 clusters, and 2 probed clusters. Replace
the dataset paths with the paths on your system:
./search -hp <path-to-hpatches> -mir <path-to-mirflickr> \
-split <path-to-split.txt> -vocab 256 -S 500 -D 10000 \
-set validation -o output.txt \
-ivfflat -kclusters 10 -nprobe 2 -seed 1

The main options are:
- "-hp": path to the HPatches dataset
- "-mir": path to the MIRFlickr dataset
- "-split": path to the dataset split file
- "-vocab": vocabulary size
- "-S": maximum sampled SIFT descriptors per training image
- "-D": number of MIRFlickr images added to the database
- "-set": "validation" or "test"
- "-o": output file
- "-seed": fixed random seed

Choose one search method: "-exact", "-lsh", "-hypercube", "-ivfflat", or "-ivfpq".

Method parameters (defaults in parentheses):

- "-exact": exhaustive search and no parameters
- "-lsh": "-k" hash functions (4), "-L" hash tables (5), "-w" cell width
  (0.05).
- "-hypercube": "-kproj" projection dimension (10), "-w" cell width
  (0.05), "-M" maximum candidates (500), "-probes" vertices checked (10).
- "-ivfflat": "-kclusters" coarse clusters (implementation default:
  "round(n^(1/4))", minimum 5), "-nprobe" lists checked (5).
- "-ivfpq": "-kclusters" coarse clusters (implementation default:
  "round(n^(1/4))", minimum 5), "-nprobe" lists checked (5), "-M"
  subvectors (16), "-nbits" bits per codebook (8, or "2^nbits"
  codewords).