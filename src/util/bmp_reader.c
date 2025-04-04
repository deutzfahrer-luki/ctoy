#include <ctoy.h>
#include <stdio.h> 

// debugging!!!
#define DEBUGG_MEMPTR_READ 0
#define DEBUGG_BMP_HEADER_READ 0
#define DEBUGG_BMP_INFO_HEADER_READ 0
#define DEBUGG_DRAW_ANIMATION 0


// which Programm:
#define SIGNLE_DRAW_PICTURE 1
#define ANIMATION_DRAW_PICTURE !SIGNLE_DRAW_PICTURE


// constants
#define COMP 3
#define SCALE 10

// dimensions
#define FRAME_WIDTH 8
#define FRAME_HEIGHT 8
#define TOTAL_FRAMES 27
#define FRAME_DELAY 1.0f


// index Frame
uint32_t frameIndex = 0;


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

MEMPTR mem;

// every func in code:
MEMPTR readFileToMemory(const char *filename);
void assHead(MEMPTR* mem);
void printHead();
void assInfoHead(void *ptr);
void printInfoHead();
void drawSingleImage(MEMPTR *data);
void DrawAnimation(MEMPTR *data, int32_t frWidth, int32_t frHeight, uint32_t frIndex, uint32_t numFrames);

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

   // Ensure that we are pointing to the correct part of the memory for the BMP info header
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
void drawSingleImage(MEMPTR *data) 
{
   m_image_create(&framebuffer, M_UBYTE, bmpInfoHeader->biWidth, bmpInfoHeader->biHeight, COMP);
   if (!framebuffer.data) {
      printf("Error: Failed to create framebuffer\n");
      printf("Image width: %d, Image height: %d\n", bmpInfoHeader->biWidth, bmpInfoHeader->biHeight);
      return;
   }


   for (size_t y = 0; y < bmpInfoHeader->biHeight; y++) {
      for (size_t x = 0; x < bmpInfoHeader->biWidth; x++) {
         uint8_t *fbpixel = framebuffer.data + (bmpInfoHeader->biWidth*y+x) * COMP;
         uint8_t *pixel = data->ptr + bmpHeader->OffBits 
             + (bmpInfoHeader->biHeight - 1 - y) * (bmpInfoHeader->biWidth * COMP) 
             + x * COMP;
         

         fbpixel[0] = pixel[2];
         fbpixel[1] = pixel[1];
         fbpixel[2] = pixel[0];
      }
   }
   ctoy_swap_buffer(&framebuffer);
}

void DrawAnimation(MEMPTR *data, int32_t frWidth, int32_t frHeight, uint32_t frIndex, uint32_t numFrames)
{
   int framesPerRow = bmpInfoHeader->biWidth / frWidth;
   int row = (numFrames / framesPerRow - 1) - (frIndex / framesPerRow);
   int col = frIndex % framesPerRow;

   #if DEBUGG_DRAW_ANIMATION == 1
   printf("framePerRow: %d\n", framesPerRow);
   printf("col: %d\n", col);
   #endif

   uint8_t *frameStart = data->ptr + bmpHeader->OffBits + (row * frHeight * bmpInfoHeader->biWidth + col * frWidth) * COMP;

   m_image_create(&framebuffer, M_UBYTE, frWidth, frHeight, COMP);

   for (size_t y = 0; y < frHeight; y++) {
      for (size_t x = 0; x < frWidth; x++) {
         uint8_t *fbpixel = framebuffer.data + (frWidth * y + x) * COMP;
         uint8_t *pixel = frameStart + (frHeight - 1 - y) * bmpInfoHeader->biWidth * COMP + x * COMP;
         
         fbpixel[0] = pixel[2];
         fbpixel[1] = pixel[1];
         fbpixel[2] = pixel[0];
      }
   }

}


// ctoy section
void ctoy_begin(void)
{
   printf("Hello World!\n");
   ctoy_window_title("Tetris");

   mem = readFileToMemory("data/tetris.bmp");
   if (mem.ptr == NULL) {
      printf("data/tetris.bmp not found\n");
      return;
   }
   assHead(&mem);
   assInfoHead((void *)(mem.ptr + sizeof(BMPHeader))); 

   if (!bmpHeader) {
      printf("Error: BMP Header is NULL\n");
      return;
   }
   if (!bmpInfoHeader) {
      printf("Error: BMP Info Header is NULL\n");
      return;
   }

   #if SIGNLE_DRAW_PICTURE == 1
   drawSingleImage(&mem);
   #endif

   ctoy_window_size(800, 800);
}


void ctoy_end(void)
{
   m_image_destroy(&framebuffer);
}

void ctoy_main_loop(void)
{
   #if ANIMATION_DRAW_PICTURE == 1
   ctoy_swap_buffer(&framebuffer);
   DrawAnimation(&mem, FRAME_WIDTH, FRAME_HEIGHT, frameIndex, TOTAL_FRAMES);
   frameIndex = (frameIndex + 1) % TOTAL_FRAMES;
   ctoy_sleep(FRAME_DELAY, 0);
   #endif
}