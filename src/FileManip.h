#ifndef FILEMANIP_H
#define FILEMANIP_H

#include <vector>
#include <stdexcept>
#include <fstream>

#include <zlib.h>

#include "GLFWAssets.h"
#include "mupdf/fitz.h"

const int COVER_PIXEL_WIDTH = 80;
const int COVER_PIXEL_HEIGHT = 100;
const float COVER_SCALE = 0.25f;

struct CoverImage {

    std::vector<unsigned char> pixels;
    int pixmap_width = 0;
    int pixmap_height = 0;

};

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

CoverImage ExtractCover(fz_context* context, const std::string& path) {

    fz_document* docu = fz_open_document(context, path.c_str());
    if (!docu) {
        throw std::runtime_error("Failed to open file: " + path + "\n");
    }

    CoverImage NewCover;

    fz_page* page = fz_load_page(context, docu, 0);
    fz_rect bounds;
    bounds = fz_bound_page(context, page);
    fz_matrix transform = fz_scale(COVER_SCALE, COVER_SCALE);
    fz_rect transformed_bounds = fz_transform_rect(bounds, transform);
    fz_irect bbox = fz_round_rect(transformed_bounds);

    fz_pixmap* pix = fz_new_pixmap_with_bbox(context, fz_device_rgb(context), bbox, NULL, 1);
    fz_clear_pixmap_with_value(context, pix, 0xFF);

    fz_device* dev = fz_new_draw_device(context, transform, pix);
    fz_run_page(context, page, dev, fz_identity, NULL);
    fz_drop_device(context, dev);

    NewCover.pixmap_width = fz_pixmap_width(context, pix);
    NewCover.pixmap_height = fz_pixmap_height(context, pix);
    int stride = fz_pixmap_stride(context, pix);  
    int n = fz_pixmap_components(context, pix);
    
    unsigned char* rgb = fz_pixmap_samples(context, pix);
    std::vector<unsigned char> pixels(NewCover.pixmap_width * NewCover.pixmap_height * 4);
    for (int y = 0; y < NewCover.pixmap_height; ++y) {
        for (int x = 0; x < NewCover.pixmap_width; ++x) {
            unsigned char r = rgb[y * stride + x * n + 0];
            unsigned char g = rgb[y * stride + x * n + 1];
            unsigned char b = rgb[y * stride + x * n + 2];
            unsigned char a = (n == 4) ? rgb[y * stride + x * n + 3] : 255;

            int idx = (y * NewCover.pixmap_width + x) * 4;
            pixels[idx + 0] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
            pixels[idx + 3] = a;
        }
    }

    NewCover.pixels = std::move(pixels);

    fz_drop_pixmap(context, pix);
    fz_drop_page(context, page);
    fz_drop_document(context, docu);

    return NewCover;
    
}

GLuint CreateTextureFromRGBA(const std::vector<unsigned char>& pixels, int width, int height)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}



#endif