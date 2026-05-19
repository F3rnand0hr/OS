#include <stdio.h>
#include <stdlib.h>

#include "../include/functions.h"

int main(int argc, char* argv[]) {
    // 1. Argument Validation
  if (argc < 3) {
    printf("Usage: %s <input.bmp> <output.bmp>\n", argv[0]);
    return 1;
  }

  // 2. File Opening (Binary Mode)
  FILE* f_in = fopen(argv[1], "rb");
  FILE* f_out = fopen(argv[2], "wb");

  if (!f_in || !f_out) {
    perror("Error opening files");
    if (f_in) fclose(f_in);
    return 1;
  }

  BMPHeader header;
  BMPInfoHeader info;

  // 3. Metadata Extraction
  // The offset is automatically read into 'header.offset' here
  fread(&header, sizeof(BMPHeader), 1, f_in);
  fread(&info, sizeof(BMPInfoHeader), 1, f_in);

  if (header.type != 0x4D42) {
    fprintf(stderr, "Error: Invalid BMP format.\n");
    fclose(f_in);
    fclose(f_out);
    return 1;
  }

  // ---------------------------------------------------------
  // 4. READING & APPLYING THE OFFSET
  // ---------------------------------------------------------
  // The 'offset' field (bytes 10-13 of the file) tells us where
  // the pixel data starts. We must 'jump' to that position.

  uint32_t pixel_offset = header.offset;
  int32_t seek_pos = (int32_t)pixel_offset;
  printf("Logical Procedure: Jumping to pixel data at byte %u\n", pixel_offset);

  fseek(f_in, seek_pos, SEEK_SET);

  fwrite(&header, sizeof(BMPHeader), 1, f_out);
  fwrite(&info, sizeof(BMPInfoHeader), 1, f_out);
  fseek(f_out, seek_pos, SEEK_SET);
  // ---------------------------------------------------------

  int32_t width = info.width;
  int32_t height = info.height < 0 ? -info.height : info.height;
  int padding = (4 - (width * 3) % 4) % 4;

  for (int32_t row = 0; row < height; row++) {
    for (int32_t col = 0; col < width; col++) {
      uint8_t pixel[3];
      fread(pixel, 1, 3, f_in);
      pixel[0] = (uint8_t)(255 - pixel[0]);
      pixel[1] = (uint8_t)(255 - pixel[1]);
      pixel[2] = (uint8_t)(255 - pixel[2]);
      fwrite(pixel, 1, 3, f_out);
    }
    if (padding > 0) {
      uint8_t pad_buf[3];
      fread(pad_buf, 1, (size_t)padding, f_in);
      fwrite(pad_buf, 1, (size_t)padding, f_out);
    }
  }

  printf("Processing complete: %s -> %s\n", argv[1], argv[2]);

  fclose(f_in);
  fclose(f_out);
  return 0;
}