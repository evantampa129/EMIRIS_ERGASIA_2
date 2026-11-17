#include "preprocess.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm> // for the shuffle func
#include <random>    // for the shuffle func
#include <limits>    // for max double value, to initialize min distance in k-means
#include <opencv2/opencv.hpp>

// randomly selects up to S descriptors from a single images descriptor matrix
std::vector<std::vector<float>> sampleDescriptors(const std::vector<std::vector<float>>& descriptors, int S, int seed) {
    // if the image has less than S descriptors, get all of them
    if (descriptors.size() <= static_cast<size_t>(S)) {
        return descriptors;
    }

    // create a list of all row indices
    std::vector<int> row_indices(descriptors.size());
    for (size_t i = 0; i < descriptors.size(); ++i) {
        row_indices[i] = i;
    }

    // shuffle the row indices using seed for reproducibility
    std::mt19937 rng(seed);
    std::shuffle(row_indices.begin(), row_indices.end(), rng);

    // create new vector for tje now sampled descriptors
    std::vector<std::vector<float>> sampled;
    sampled.reserve(S);

    // transfer the sampled descriptors to the new vector
    for (int i = 0; i < S; ++i) {
        int random_row = row_indices[i];
        sampled.push_back(descriptors[random_row]);
    }

    return sampled;
}

// find k cluster centers and return them in a matrix k x 128
std::vector<std::vector<float>> kmeans(const std::vector<std::vector<float>>& data, int K, int max_iter, double epsilon, int seed) {
    int N = data.size();
    int dims = data.empty() ? 0 : data[0].size();
    
    // initialize centers matrix with zeros, size K x dims
    std::vector<std::vector<float>> centers(K, std::vector<float>(dims, 0.0f));
    
    // initialize the first K centers by randmly selecting K rows from the matrix using seed again for reproducibility of results
    std::mt19937 rng(seed);
    std::vector<int> indices(N);
    
    // initialize indices
    for (int i = 0; i < N; ++i) {
        indices[i] = i;
    }

    // shuffle to get random ones
    std::shuffle(indices.begin(), indices.end(), rng);
    
    // initialize the centers with the first k shuffled indices
    for (int i = 0; i < K; ++i) {
        centers[i] = data[indices[i]];
    }

    // squared epsilon to check against squared distances
    double epsilon_sq = epsilon * epsilon;

    // start iterating to find the best centers
    for (int iter = 0; iter < max_iter; ++iter) {

        // counts points to each center
        std::vector<int> counts(K, 0);

        //holds the sum of points assigned to each center
        std::vector<std::vector<float>> new_centers(K, std::vector<float>(dims, 0.0f));

        // iterate through each point in data to find closest center
        for (int i = 0; i < N; ++i) {
            int best_c = 0;
            double min_dist = std::numeric_limits<double>::max();

            for (int c = 0; c < K; ++c) {
                double dist = 0.0;
                // calculate squared euclidean distance between point and center
                for (int d = 0; d < dims; ++d) {
                    double diff = data[i][d] - centers[c][d];
                    dist += diff * diff;
                }
                
                // find the closest center for this point
                if (dist < min_dist) {
                    min_dist = dist;
                    best_c = c;
                }
            }
            
            counts[best_c]++;
            
            // add point to new center sum to calculate the avg later
            for (int d = 0; d < dims; ++d) {
                new_centers[best_c][d] += data[i][d];
            }
        }

        double max_shift_sq = 0.0;

        // if points were assigned to a center, calculate the new center as the average of the assigned points,
        // otherwise re-initialize the center with a random point
        for (int c = 0; c < K; ++c) {
            if (counts[c] > 0) {
                double shift_sq = 0.0;
                for (int d = 0; d < dims; ++d) {
                    new_centers[c][d] /= counts[c]; // divide the center sum by the number of points assigned to it to get the average
                    double diff = new_centers[c][d] - centers[c][d]; // check how much the center has shifted
                    shift_sq += diff * diff;
                }
                
                // store new max shift squared
                if (shift_sq > max_shift_sq) {
                    max_shift_sq = shift_sq;
                }
            } else {
                // re-initialize empty center with random point
                int rand_idx = rng() % N;
                new_centers[c] = data[rand_idx];
                max_shift_sq = std::numeric_limits<double>::max(); 
            }
        }

        centers = new_centers;

        // convergence check: if the maximum shift of any center is less than epsilon
        // or if reached max iterations
        if (max_shift_sq < epsilon_sq) {
            std::cout << "Kmeans converged with " << iter + 1 << " iterations.\n";
            break;
        }
        if (iter == max_iter - 1) {
            std::cout << "Kmeans reached max iterations: " << max_iter << ".\n";
        }
    }
    
    return centers;
}

// extracts all SIFT descriptors from a single image and returns a standard vector of vectors, instead of openCV matrix
std::vector<std::vector<float>> extractSIFT(const std::string& img_path, cv::Ptr<cv::SIFT>& sift) {
    // read image in grayscale
    cv::Mat image = cv::imread(img_path, cv::IMREAD_GRAYSCALE);

    // if image didnt load, return empty vector, so it can be skipped
    if (image.empty()) {
        return {};
    }

    // create vector for keypoints and openCV matrix for descriptors
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat cv_descriptors;

    // run sift detection and compute descriptors, noArray means whole image (no mask)
    sift->detectAndCompute(image, cv::noArray(), keypoints, cv_descriptors);

    // if no descriptors were found return empty vector
    if (cv_descriptors.empty()) {
        return {};
    }

    // immediately convert openCV matrix into standard vector of vectors because of assignment restrictions
    std::vector<std::vector<float>> std_descriptors;
    std_descriptors.reserve(cv_descriptors.rows);

    // copy onto standard vector
    for (int r = 0; r < cv_descriptors.rows; ++r) {
        std::vector<float> row(128);
        const float* row_ptr = cv_descriptors.ptr<float>(r);
        for (int c = 0; c < 128; ++c) {
            row[c] = row_ptr[c];
        }
        std_descriptors.push_back(row);
    }

    return std_descriptors;
}

// loops through all 70 training folders, extracts SIFT, and runs k-means to find K centers
std::vector<std::vector<float>> buildVocabulary(const std::string& hp_path, 
                                                const std::vector<std::string>& train_seqs, 
                                                int vocab_size, 
                                                int S, 
                                                int seed) {
    
    // create openCV sift object
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

    // this vector will store all sampled descriptors from all training images
    std::vector<std::vector<float>> all_pooled_descriptors;

    // debug, how many training sequences are we processing
    std::cout << "Extracting SIFT descriptors from " << train_seqs.size() << " training sequences.\n";

    // outer loop: go through th 70 training folders
    for (size_t i = 0; i < train_seqs.size(); ++i) {
        std::string folder_name = train_seqs[i];
        
        // inner loop: go through the 6 images in folder
        for (int img_num = 1; img_num <= 6; ++img_num) {
            
            // build full path to the image file
            std::string img_path = hp_path + "/" + folder_name + "/" + std::to_string(img_num) + ".ppm";
            
            // use our help func to extract all descriptors from the image to standard vector
            std::vector<std::vector<float>> std_descriptors = extractSIFT(img_path, sift);

            // if image failed to load or had 0 descriptors, skip
            if (std_descriptors.empty()) {
                continue;
            }

            // sample descriptors from image
            int image_seed = seed + (i * 10) + img_num;
            std::vector<std::vector<float>> sampled = sampleDescriptors(std_descriptors, S, image_seed);

            // add sampled descriptors to the pooled vector
            all_pooled_descriptors.insert(
                all_pooled_descriptors.end(), 
                sampled.begin(), 
                sampled.end()
            );
        }

        // debug, print progress every 10 folders
        if ((i + 1) % 10 == 0 || i + 1 == train_seqs.size()) {
            std::cout << "  Processed " << (i + 1) << "/" << train_seqs.size() 
                      << " folders (total descriptors collected: " << all_pooled_descriptors.size() << ")\n";
        }
    }

    // if 0 descriptors were collected, throw an error before k-means crashes
    if (all_pooled_descriptors.empty()) {
        throw std::runtime_error("Error: No descriptors extracted, check -hp path.");
    }

    // debug
    std::cout << "Running custom k-means to find K = " << vocab_size << " centers ...\n";

    // run custom k-means clustering
    std::vector<std::vector<float>> centers = kmeans(
        all_pooled_descriptors, 
        vocab_size, 
        100,      // max iterations
        0.001,    // epsilon convergence threshold
        seed      // use the same seed for reproducibility
    );

    // debug, print the size of the centers matrix
    // rows = vocab_size, cols = 128
    std::cout << "Vocabulary built! Centers matrix size: " 
              << centers.size() << " rows x " << (centers.empty() ? 0 : centers[0].size()) << " cols\n";

    return centers;
}

// returns the BoVW vector for a single image
// it is a frequency histogram of descriptors assigned to each cluster center,
// normalized so that all values sum to 1
std::vector<float> computeBoVWVector(const std::vector<std::vector<float>>& descriptors,
                                     const std::vector<std::vector<float>>& centers) {
    // k is the number of cluster centers
    // n_i is the total number of descriptors in this image
    int K = centers.size();
    int n_I = descriptors.size();

    // initialize histogram vector of size k with zeroes to count how many descriptors match each center, h_j from the assignment
    std::vector<float> histogram(K, 0.0f);

    // safety check in case descriptors or centers are empty
    // if either is empty, return the zero histogram
    if (n_I == 0 || K == 0) {
        return histogram;
    }

    // loop through all descriptors of the image (assignment notes all descriptors, not S here)
    for (int i = 0; i < n_I; ++i) {
        int best_c = 0;
        double min_dist = std::numeric_limits<double>::max();

        // compare current descriptor against all k centers to find the closest one
        for (int c = 0; c < K; ++c) {
            double dist = 0.0;

            // calculate squared euclidean distance
            for (int d = 0; d < 128; ++d) {
                double diff = descriptors[i][d] - centers[c][d];
                dist += diff * diff;
            }

            // update closest center if found a smaller distance
            if (dist < min_dist) {
                min_dist = dist;
                best_c = c;
            }
        }

        // increment count h_j for the closest center
        histogram[best_c] += 1.0f;
    }

    // normalize each bin by dividing h_j by total descriptors n_i so all coordinates sum to 1
    for (int c = 0; c < K; ++c) {
        histogram[c] /= static_cast<float>(n_I);
    }

    return histogram;
}

// convert images to BoVW vectors and store them in queries and database vectors
void buildQueriesAndDatabase(const Config& config,
                             const std::vector<std::string>& active_seqs,
                             const std::vector<std::vector<float>>& centers,
                             std::vector<ImageVector>& queries,
                             std::vector<ImageVector>& database) {
    // create openCV sift object
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

    // reserve space upfront since we can calculate: 23 queries and (23 * 5 + D) database vectors
    // that way no unnecessary reallocations will occur
    queries.reserve(active_seqs.size());
    database.reserve(active_seqs.size() * 5 + config.D);

    // debug
    std::cout << "Building query and HPatches database vectors from " << active_seqs.size() << " sequences.\n";

    // process hpatches
    // validation or test sequences, each sequence has 6 images, image 1 is the query and images 2-6 are relevant database images
    for (size_t i = 0; i < active_seqs.size(); ++i) {
        std::string seq_name = active_seqs[i];

        // loop through images 1 to 6
        for (int img_num = 1; img_num <= 6; ++img_num) {
            std::string img_path = config.hp_path + "/" + seq_name + "/" + std::to_string(img_num) + ".ppm";

            // extract all sift descriptors from the image
            std::vector<std::vector<float>> desc = extractSIFT(img_path, sift);
            if (desc.empty()) {
                std::cerr << "Error: No descriptors extracted from HPatches image: " << img_path << "\n";
                continue;
            }

            // convert descriptors into a k-dimensional normalized bovw vector x_i
            std::vector<float> vec = computeBoVWVector(desc, centers);

            // image 1, the query
            // the format of the id is just the sequence name
            if (img_num == 1) {
                queries.push_back({seq_name, vec});
            }
            // images 2 to 6 are the 5 relevant images placed in the database
            // the format of the id is "<sequence_name>/<image_number>", so we append the image number to the sequence name with a slash in between
            else {
                std::string db_id = seq_name + "/" + std::to_string(img_num);
                database.push_back({db_id, vec});
            }
        }
    }

    // debug
    std::cout << "Building MIRFlickr database vectors (collecting D = " << config.D << " valid images).\n";

    // now process mirflickr dataset
    // we need to collect exactly D valid images, so we will loop through the images in ascending order until we have D valid ones
    // skipping any that fail to load or have 0 descriptors
    int collected_mir = 0;
    int file_num = 1;
    const int max_mir_files = 25000; // total images in mirflickr dataset

    // keep reading images in order until we have collected exactly D valid vectors
    while (collected_mir < config.D && file_num <= max_mir_files) {
        // build the filename and path for the current image
        // format is "im<file_num>.jpg"
        std::string filename = "im" + std::to_string(file_num) + ".jpg";
        std::string img_path = config.mir_path + "/" + filename;
        file_num++;

        // extract all sift descriptors from the mirflickr image
        std::vector<std::vector<float>> desc = extractSIFT(img_path, sift);

        // assignment rule: if an image produces 0 sift descriptors (or fails to load), reject it and move to the next one in order
        if (desc.empty()) {
            continue;
        }

        // convert descriptors into a k-dimensional bovw vector using the same hpatches centers
        std::vector<float> vec = computeBoVWVector(desc, centers);

        // store in database with filename: "im<file_num>.jpg" as id
        database.push_back({filename, vec});
        collected_mir++;

        // debug, print progress every 2500 images
        if (collected_mir % 2500 == 0 || collected_mir == config.D) {
            std::cout << "  Collected " << collected_mir << "/" << config.D << " MIRFlickr images\n";
        }
    }

    // no valid image at all means a wrong -mir path
    if (config.D > 0 && collected_mir == 0) {
        throw std::runtime_error("Error: Could not collect any valid MIRFlickr image. Check -mir path.");
    }

    // with all 25000 files read there is no image left to replace the rejected ones, so every valid image is used
    if (collected_mir < config.D) {
        std::cout << "  Only " << collected_mir << " of the " << max_mir_files
                  << " MIRFlickr images have SIFT descriptors, all of them are used\n";
    }

    // debug, print final counts of queries and database vectors
    std::cout << "Preprocessing complete! Total queries: " << queries.size() 
              << ", Total database size: " << database.size() << "\n";
}