#ifndef FILEMANIP_H
#define FILEMANIP_H

#include <vector>
#include <stdexcept>
#include <fstream>

#include <zlib.h>

std::vector<unsigned char> CompressBlob(std::vector<unsigned char>& blob) {

    uLong src_size = blob.size();
    ulong dest_size = compressBound(src_size);
    std::vector<unsigned char> compressed_blob(dest_size);
    
    int res = compress2(
        compressed_blob.data(),
        &dest_size,
        blob.data(),
        src_size,
        Z_BEST_COMPRESSION
    );
    
    if (res != Z_OK){
        throw std::runtime_error("Compression failed.");
    }

    compressed_blob.resize(dest_size);
    return compressed_blob;
    
}

std::vector<unsigned char> DecompressBlob(std::vector<unsigned char>& compressed_blob, size_t original_size) {

    std::vector<unsigned char> decompressed_blob(original_size);
    uLongf dest_size = original_size;

    int res = uncompress(
        decompressed_blob.data(),
        &dest_size,
        compressed_blob.data(),
        compressed_blob.size()
    );

    if (res != Z_OK) {
        throw std::runtime_error("Decompression failed.");
    }
    
    return decompressed_blob;

}

std::vector<unsigned char> ReadVoidBlobIntoUnsignedCharVector(const void* blob, int size) {

    std::vector<unsigned char> blob_vector;
    const unsigned char* blob_bytes = static_cast<const unsigned char*>(blob);

    blob_vector.assign(blob_bytes, blob_bytes + size);
    return blob_vector;

}

std::vector<unsigned char> readFile(std::string& path) {
    
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;

}

#endif