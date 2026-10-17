// Exercises the bundled IJG libjpeg 6b sources. Compresses an 8x8 RGB image
// through the stdio destination and reads the first pixel back.
#include <stdio.h>
#include <string.h>

extern "C" {
#include "jpeglib.h"
}

int main(void) {
    const int width = 8;
    const int height = 8;
    unsigned char pixels[8 * 8 * 3];
    for (int i = 0; i < width * height; ++i) {
        pixels[i * 3 + 0] = 200;
        pixels[i * 3 + 1] = 10;
        pixels[i * 3 + 2] = 40;
    }

    FILE* fp = fopen("roundtrip.jpg", "wb+");
    if (!fp) {
        printf("JPEG_ROUNDTRIP fopen_failed\n");
        return 1;
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 90, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    while (cinfo.next_scanline < cinfo.image_height) {
        JSAMPROW row = pixels + cinfo.next_scanline * width * 3;
        jpeg_write_scanlines(&cinfo, &row, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fflush(fp);
    fseek(fp, 0, SEEK_SET);

    struct jpeg_decompress_struct dinfo;
    dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);
    jpeg_stdio_src(&dinfo, fp);
    if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK) {
        printf("JPEG_ROUNDTRIP header_failed\n");
        fclose(fp);
        return 1;
    }
    jpeg_start_decompress(&dinfo);
    unsigned char out[8 * 8 * 3];
    while (dinfo.output_scanline < dinfo.output_height) {
        JSAMPROW out_row = out + dinfo.output_scanline * 8 * 3;
        jpeg_read_scanlines(&dinfo, &out_row, 1);
    }
    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);
    fclose(fp);

    printf("JPEG_ROUNDTRIP pixel=%u,%u,%u\n", out[0], out[1], out[2]);
    if (out[0] < 180 || out[1] > 40 || out[2] > 70) return 2;
    return 0;
}
