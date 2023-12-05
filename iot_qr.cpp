#include <iostream>
#include <cstring>
#include "quirc.h"

int main() {
    // Initialize the quirc library
    struct quirc *qr = quirc_new();
    if (!qr) {
        std::cerr << "Error initializing quirc library" << std::endl;
        return 1;
    }

    // Initialize the quirc data object
    struct quirc_data qd;

    // Load an image (replace "path/to/your/image.png" with the path to your QR code image)
    const char *filename = "path/to/your/image.png";  // Replace with your image file path
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        std::cerr << "Error opening image file" << std::endl;
        quirc_destroy(qr);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char *image = (unsigned char *)malloc(file_size);
    if (!image) {
        std::cerr << "Error allocating memory for image" << std::endl;
        fclose(fp);
        quirc_destroy(qr);
        return 1;
    }

    fread(image, 1, file_size, fp);
    fclose(fp);

    // Load the image into quirc
    if (quirc_resize(qr, 640, 480) < 0) {
        std::cerr << "Error resizing quirc" << std::endl;
        free(image);
        quirc_destroy(qr);
        return 1;
    }

    quirc_end(qr);

    memcpy(quirc_begin(qr), image, file_size);

    // Identify and decode QR codes in the image
    if (quirc_count(qr) < 0) {
        std::cerr << "Error counting QR codes" << std::endl;
        free(image);
        quirc_destroy(qr);
        return 1;
    }

    // Process each identified QR code
    for (int i = 0; i < quirc_count(qr); ++i) {
        quirc_extract(qr, i, &qd);

        std::cout << "Decoded QR code at position (" << qd.x << "," << qd.y << "): ";
        std::cout.write(reinterpret_cast<char *>(qd.payload), qd.payload_len);
        std::cout << std::endl;
    }

    // Clean up
    free(image);
    quirc_destroy(qr);

    return 0;
}