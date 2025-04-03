#include <ctoy.h>
#include <stdio.h> // optional (stdio.h is already included in ctoy.h)

// debugging!!!
#define DEBUGG_MEMPTR_READ 1
#define DEBUGG_BMP_HEADER_READ 1
 
struct m_image framebuffer = M_IMAGE_IDENTITY(); // initialize the struct (all set to zero in this case)

typedef struct {
   uint8_t *ptr;
   size_t size;
} MEMPTR;

#pragma pack(push, 1)
typedef struct{
   uint16_t Type;
   uint32_t HSize;
   uint32_t Reserved;
   uint32_t OffBits;
} BMPHeader;
#pragma pack(pop);

BMPHeader *bmpHeader = NULL;


// every func in code:
MEMPTR readFileToMemory(const char *filename);
void assHead(MEMPTR* mem);
void printHead();

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

}

void ctoy_end(void)
{}

void ctoy_main_loop(void)
{}