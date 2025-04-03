#include <ctoy.h>
#include <stdio.h> // optional (stdio.h is already included in ctoy.h)

// debugging!!!
#define DEBUGG_MEMPTR_READ 0
#define DEBUGG_BMP_HEADER_READ 0
#define DEBUGG_BMP_INFO_HEADER_READ 1
 
struct m_image framebuffer = M_IMAGE_IDENTITY(); // initialize the struct (all set to zero in this case)

// file struct
typedef struct {
   uint8_t *ptr;
   size_t size;
} MEMPTR;


// Head struct
#pragma pack(push, 1)
typedef struct{
   uint16_t Type;
   uint32_t HSize;
   uint32_t Reserved;
   uint32_t OffBits;
} BMPHeader;
#pragma pack(pop);
BMPHeader *bmpHeader = NULL;


// InfoHead struct
#pragma pack(push, 1)
typedef struct {
    uint32_t biSize;         // Größe des Info-Headers
    int32_t biWidth;         // Breite des Bildes in Pixeln
    int32_t biHeight;        // Höhe des Bildes in Pixeln
    uint16_t biPlanes;       // Anzahl der Farbebenen (muss 1 sein)
    uint16_t biBitCount;     // Farbtiefe (z. B. 24 für 24 Bit = 16,7 Mio Farben)
    uint32_t biCompression;  // Kompressionstyp (0 = keine Kompression)
    uint32_t biSizeImage;    // Größe des Bilddatenbereichs (kann 0 sein)
    int32_t biXPelsPerMeter; // Horizontale Auflösung (Pixel pro Meter)
    int32_t biYPelsPerMeter; // Vertikale Auflösung (Pixel pro Meter)
    uint32_t biClrUsed;      // Anzahl der verwendeten Farben (0 = alle)
    uint32_t biClrImportant; // Anzahl der wichtigen Farben (0 = alle)
} BMPInfoHeader;
#pragma pack(pop)
BMPInfoHeader *bmpInfoHeader = NULL;


// every func in code:
MEMPTR readFileToMemory(const char *filename);
void assHead(MEMPTR* mem);
void printHead();
void assInfoHead(void *ptr);
void printInfoHead();

// Read file section
MEMPTR readFileToMemory(const char *filename) {
   FILE *fp = fopen(filename, "rb");

   MEMPTR mtr = {NULL, 0};

   if (!fp) {
       perror("File opening failed");
       return mtr; 
   }

   if (fseek(fp, 0L, SEEK_END) != 0) {
       perror("File seek failed");
       fclose(fp);
       return mtr;
   }
   
   size_t size = ftell(fp);
   rewind(fp);
   
   uint8_t *ptr = malloc(size);
   if (!ptr) {
       perror("Memory allocation failed");
       fclose(fp);
       return mtr;
   }
   
   if (fread(ptr, sizeof(uint8_t), size, fp) != size) {
       perror("File read failed");
       free(ptr);
       fclose(fp);
       return mtr;
   }
   
   fclose(fp);

   mtr.ptr = ptr;
   mtr.size = size;
   #if DEBUGG_MEMPTR_READ == 1
   printf("Pointer Address: %p\n", (void*)mtr.ptr); // Zeigeradresse ausgeben
   printf("Size: %zu\n", mtr.size);
   #endif
   return mtr;
}


// Head section
void assHead(MEMPTR* mem){
   if(!mem || mem->size < sizeof(BMPHeader)){
       printf("Error: (size = %zu)\n", mem->size);
       return;
   }
   bmpHeader = (BMPHeader *)mem->ptr;
   #if DEBUGG_BMP_HEADER_READ == 1
   printHead();
   #endif
}

void printHead(){
   if(!bmpHeader){
       return;
   }
   printf("BMP Header:\n");
   printf("  Type:      0x%04X\n", bmpHeader->Type);
   printf("  Size:      0x%08X bytes\n", bmpHeader->HSize);
   printf("  Reserved:  0x%08X\n", bmpHeader->Reserved);
   printf("  Offset:    0x%08X bytes\n", bmpHeader->OffBits);
}


// Info Head section
void assInfoHead(void *ptr) {
   if (!ptr) {
       printf("Error: Kein gültiger Speicher für Info Header\n");
       return;
   }
   bmpInfoHeader = (BMPInfoHeader *)ptr;
   #if DEBUGG_BMP_INFO_HEADER_READ == 1
   printInfoHead();
   #endif
}

void printInfoHead() {
   if (!bmpInfoHeader) {
      printf("Error: kein InfoHeader vorhanden!\n");
      return;
   }
   printf("BMP Info Header:\n");
   printf(" Size: %u bytes\n", bmpInfoHeader->biSize);
   printf(" Width: %d px\n", bmpInfoHeader->biWidth);
   printf(" Height: %d px\n", bmpInfoHeader->biHeight);
   printf(" Planes: %u\n", bmpInfoHeader->biPlanes);
   printf(" Bit Count: %u bits per pixel\n", bmpInfoHeader->biBitCount);
   printf(" Compression: %u\n", bmpInfoHeader->biCompression);
   printf(" Image Size: %u bytes\n", bmpInfoHeader->biSizeImage);
   printf(" X Pixels per Meter: %d\n", bmpInfoHeader->biXPelsPerMeter);
   printf(" Y Pixels per Meter: %d\n", bmpInfoHeader->biYPelsPerMeter);
   printf(" Colors Used: %u\n", bmpInfoHeader->biClrUsed);
   printf(" Important Colors: %u\n", bmpInfoHeader->biClrImportant);
}


// ImageDraq Sections
void drawSingleImage() 
{
   int width = 256;
   int height = 256;
   int comp = 3;

   m_image_create(&framebuffer, M_FLOAT, width, height, comp);

   float *pixel = (float *)framebuffer.data; // we cast image data to float in this case
   int y, x;

   for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
         
         pixel[0] = 1.0; // red
         pixel[1] = 1.0; // green
         pixel[2] = 0.0; // blue
         pixel += comp;
      }
   }

   // display the image to the frame buffer
   // float linear is converted to ubyte sRGB on the fly
   ctoy_swap_buffer(&framebuffer);
}


// ctoy section
void ctoy_begin(void)
{
   printf("\nHello World!\n");
   ctoy_window_title("Hello-World!");

   MEMPTR mem = readFileToMemory("data/tetris.bmp");
   if (mem.ptr == NULL) {
      printf("data/tetris.bmp not found\n");
      return;
   }
   assHead(&mem);
   assInfoHead(&mem.ptr + sizeof(BMPHeader));
   drawSingleImage();
}


void ctoy_end(void)
{
   m_image_destroy(&framebuffer);
}

void ctoy_main_loop(void)
{}