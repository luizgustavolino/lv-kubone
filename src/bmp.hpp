
#pragma once
#include <fstream>
#include <vector>
#include <stdexcept>
#include <unistd.h>

#pragma pack(push, 1)

struct BMPFileHeader {
    uint16_t file_type{ 0x4D42 }; // File type always BM which is 0x4D42,
    uint32_t file_size{ 0 };      // Size of the file (in bytes),
    uint32_t reserved{ 0 };       // Reserved, always 0,
    uint32_t offset_data{ 0 };    // Start position of pixel data.
};

struct BMPInfoHeader {

    uint32_t size{ 0 };   // Size of this header (in bytes),
    int32_t  width{ 0 };  // width of bitmap in pixels,
    int32_t  height{ 0 }; // width of bitmap in pixels,
    uint16_t planes{ 1 }; // No. of planes for the target device,

    uint16_t bit_count{ 24 };  // No. of bits per pixel,
    uint32_t compression{ 0 }; // 0 or 3 - uncompressed,
    uint32_t size_image{ 0 };  // 0 - for uncompressed images,

    int32_t  x_dpi{ 0 }; // X dpi,
    int32_t  y_dpi{ 0 }; // Y dpi,

    uint32_t colors_used{ 0 };      // No. color indexes in the color table,
    uint32_t colors_important{ 0 }; // No. of colors used for displaying.
};

#pragma pack(pop)

struct BMP {

    BMPFileHeader  file_header;
    BMPInfoHeader  info_header;
    std::vector<uint8_t> data;

    BMP(int32_t width, int32_t height) {

        if (width <= 0 || height <= 0) {
            throw std::runtime_error("Invalid image size");
        }

        info_header.width       = width;
        info_header.height      = height;
        info_header.size        = sizeof(BMPInfoHeader);

        row_stride = width * 3;
        data.resize(row_stride * height);
        uint32_t new_stride = make_stride_aligned(4);

        file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
        file_header.file_size  = file_header.offset_data + (unsigned int) data.size();
        file_header.file_size += info_header.height * (new_stride - row_stride);

    }

    void writeToFD(int fd) {

        if (info_header.width % 4 == 0) {
            write_headers_and_data(fd);
        } else {
            uint32_t new_stride = make_stride_aligned(4);
            std::vector<uint8_t> padding_row(new_stride - row_stride);
            write_headers(fd);

            for (int y = 0; y < info_header.height; ++y) {
                write(fd, (const char*)(data.data() + row_stride * y), row_stride);
                write(fd, (const char*)padding_row.data(), padding_row.size());
            }
        }
    }

    void fill_region(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h,
                     uint8_t R, uint8_t G, uint8_t B) {

        if (x0 + w > (uint32_t)info_header.width || y0 + h > (uint32_t)info_header.height) {
            throw std::runtime_error("The region does not fit in the image!");
        }

        uint32_t channels = info_header.bit_count / 8;
        for (uint32_t y = y0; y < y0 + h; ++y) {
            for (uint32_t x = x0; x < x0 + w; ++x) {
                data[channels * (y * info_header.width + x) + 0] = B;
                data[channels * (y * info_header.width + x) + 1] = G;
                data[channels * (y * info_header.width + x) + 2] = R;
            }
        }
    }

    void set_pixel(uint32_t x, uint32_t y, uint8_t R, uint8_t G, uint8_t B) {

        if (x > (uint32_t) info_header.width || y > (uint32_t) info_header.height) {
            throw std::runtime_error("The pixel is outside bounds");
        }

        uint32_t channels = info_header.bit_count / 8;
        data[channels * (y * info_header.width + x) + 0] = B;
        data[channels * (y * info_header.width + x) + 1] = G;
        data[channels * (y * info_header.width + x) + 2] = R;
    }

    private:

        uint32_t row_stride{ 0 };

        uint32_t make_stride_aligned(uint32_t align_stride) {
            uint32_t new_stride = row_stride;
            while (new_stride % align_stride != 0) new_stride++;
            return new_stride;
        }

        void write_headers(int fd) {
            write(fd, (const void*)&file_header, sizeof(file_header));
            write(fd, (const void*)&info_header, sizeof(info_header));
        }

        void write_headers_and_data(int fd) {
            write_headers(fd);
            write(fd, (const void*) data.data(), data.size());
        }

};