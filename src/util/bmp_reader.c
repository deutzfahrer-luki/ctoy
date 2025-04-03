#include <ctoy.h>
#include <stdio.h> // optional (stdio.h is already included in ctoy.h)

// debugging!!!
#define MEMPTR_READ_TEST 1

struct m_image framebuffer = M_IMAGE_IDENTITY(); // initialize the struct (all set to zero in this case)

typedef struct {
   uint8_t *ptr;
   size_t size;
} MEMPTR;

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
   #if MEMPTR_READ_TEST == 1
   printf("Pointer Address: %p\n", (void*)mtr.ptr); // Zeigeradresse ausgeben
   printf("Size: %zu\n", mtr.size);
   #endif
   return mtr;
}



void ctoy_begin(void)
{
   printf("\nHello World!\n");
   ctoy_window_title("Hello-World!");

   MEMPTR mem = readFileToMemory("data/tetris.bmp");
   if (mem.ptr == NULL) {
      printf("data/tetris.bmp not found\n");
      return;
   }
   
}

void ctoy_end(void)
{}

void ctoy_main_loop(void)
{}