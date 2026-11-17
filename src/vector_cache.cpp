#include "vector_cache.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

// raw bytes of a value, the file is only read on the machine that wrote it
template <typename T>
static void writeValue(std::ofstream& out, const T& value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
static void readValue(std::ifstream& in, T& value) {
    in.read(reinterpret_cast<char*>(&value), sizeof(T));
}

void saveVectorSet(const std::string& filename, const VectorSet& set) {
    if (set.vectors.size() != set.ids.size()) {
        throw std::invalid_argument("Error: every vector must have exactly one id");
    }

    fs::path path(filename);
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
    }

    // write to a temporary file first, so a stopped run never leaves half a cache file
    std::string tmp = filename + ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary);
        if (!out.is_open()) {
            throw std::runtime_error("Error: Could not write cache file " + tmp);
        }

        uint64_t count = set.vectors.size();
        uint64_t dim = set.vectors.empty() ? 0 : set.vectors[0].size();
        writeValue(out, count);
        writeValue(out, dim);
        for (size_t i = 0; i < set.vectors.size(); ++i) {
            if (set.vectors[i].size() != dim) {
                throw std::invalid_argument("Error: all vectors must have the same dimension");
            }
            uint64_t length = set.ids[i].size();
            writeValue(out, length);
            out.write(set.ids[i].data(), static_cast<std::streamsize>(length));
            out.write(reinterpret_cast<const char*>(set.vectors[i].data()), static_cast<std::streamsize>(dim * sizeof(float)));
        }
        if (!out) {
            throw std::runtime_error("Error: Could not write cache file " + tmp);
        }
    }
    fs::rename(tmp, filename);
}

bool loadVectorSet(const std::string& filename, VectorSet& set) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    uint64_t count = 0;
    uint64_t dim = 0;
    readValue(in, count);
    readValue(in, dim);

    VectorSet loaded;
    for (uint64_t i = 0; i < count && in; ++i) {
        uint64_t length = 0;
        readValue(in, length);
        if (!in || length > 4096) {
            break;
        }
        std::string id(length, '\0');
        in.read(id.data(), static_cast<std::streamsize>(length));

        std::vector<float> vector(dim);
        in.read(reinterpret_cast<char*>(vector.data()), static_cast<std::streamsize>(dim * sizeof(float)));
        loaded.ids.push_back(id);
        loaded.vectors.push_back(vector);
    }

    if (!in || loaded.ids.size() != count) {
        throw std::runtime_error("Error: Damaged cache file " + filename + ", delete it and run again");
    }
    set = loaded;
    return true;
}
