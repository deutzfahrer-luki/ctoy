#include <ctoy.h>

#define BYTES_PER_PIXEL 3

struct m_image framebuffer = M_IMAGE_IDENTITY();

typedef struct{
	uint8_t *ptr;
	size_t size;
}MEMPTR;

typedef struct {
	uint8_t *data;         // Gesamte Datei (10003 Bytes)
	uint8_t *bitmap;       // Bitmap-Grafikdaten (8000 Bytes)
	uint8_t *screen_ram;   // Screen RAM (1000 Bytes)
	uint8_t *color_ram;    // Color RAM (1000 Bytes)
	uint8_t background_color; // Hintergrundfarbe (1 Byte)
} KLAFile;

KLAFile koala = {NULL};
MEMPTR mem = {NULL, 0};

// RGB-Farben aus C64-Palette
uint8_t c64_colors[16][3] = {
	{0, 0, 0},         // Schwarz
	{255, 255, 255},   // Weiß
	{136, 0, 0},       // Rot
	{170, 255, 238},   // Cyan
	{204, 68, 204},    // Lila
	{0, 204, 85},      // Grün
	{0, 0, 170},       // Blau
	{238, 238, 119},   // Gelb
	{221, 136, 85},    // Orange
	{102, 68, 0},      // Braun
	{255, 119, 119},   // Hellrot
	{51, 51, 51},      // Dunkelgrau
	{119, 119, 119},   // Grau
	{170, 255, 102},   // Hellgrün
	{0, 136, 255},     // Hellblau
	{187, 187, 187}    // Hellgrau
};

// every func in code:
MEMPTR readFileToMemory(const char *filename);
void assignHead(MEMPTR *mem);
void draw_koala(void);
void ctoy_begin(void);
void ctoy_main_loop(void);
void ctoy_end(void);

// Liest Datei in den Speicher (Rohdaten)
MEMPTR readFileToMemory(const char *filename){
	MEMPTR mem = {NULL, 0};
	FILE* fp = fopen(filename, "rb");
	if (!fp){
		perror("Datei konnte nicht geöffnet werden");
		return mem;
	}

	if(fseek(fp, 0L, SEEK_END) != 0){
		perror("Konnte Dateigröße nicht ermitteln");
		fclose(fp);
		return mem;
	}

	mem.size = ftell(fp);
	rewind(fp);

	mem.ptr = malloc(mem.size);
	if(!mem.ptr){
		perror("Speicher konnte nicht reserviert werden");
		fclose(fp);
		return mem;
	}

	if(fread(mem.ptr, 1, mem.size, fp) != mem.size){
		perror("Fehler beim Lesen der Datei");
		free(mem.ptr);
		mem.ptr = NULL;
		mem.size = 0;
	}

	fclose(fp);
	return mem;
}

// Setzt Zeiger auf die einzelnen Koala-Teile
void assignHead(MEMPTR *mem){
	if (!mem || !mem->ptr || mem->size != 10003){
		printf("Ungültige Koala-Datei (Größe = %zu, erwartet 10003)\n", mem ? mem->size : 0);
		return;
	}

	koala.data            = (uint8_t *)mem->ptr;
	koala.bitmap          = koala.data + 2;
	koala.screen_ram      = koala.data + 8002;
	koala.color_ram       = koala.data + 9002;
	koala.background_color = koala.data[10002] & 0x0F;
}

// Zeichnet Koala-Bild auf Framebuffer
void draw_koala(){
	uint8_t *fb_data = (uint8_t *)framebuffer.data;

	uint8_t *bg_rgb = c64_colors[koala.background_color];

	// Hintergrund vorbereiten
	for (int i = 0; i < 320 * 200 * BYTES_PER_PIXEL; i += BYTES_PER_PIXEL){
		fb_data[i]     = bg_rgb[0]; // R
		fb_data[i + 1] = bg_rgb[1]; // G
		fb_data[i + 2] = bg_rgb[2]; // B
	}

	// 40x25 Blöcke je 8x8 Pixel
	for (int block_y = 0; block_y < 25; block_y++){
		for (int block_x = 0; block_x < 40; block_x++){
			int block_index = block_y * 40 + block_x;

			uint8_t screen_byte = koala.screen_ram[block_index];
			uint8_t color_byte  = koala.color_ram[block_index];

			uint8_t pixel_colors[4] = {
				koala.background_color,
				(screen_byte >> 4) & 0x0F,  // hohe 4 Bit
				screen_byte & 0x0F,         // niedrige 4 Bit
				color_byte & 0x0F           // Farbram
			};

			// Pro Block 8 Zeilen 4 Pixel je 2 Bit
			for (int row = 0; row < 8; row++){
				int bitmap_index = block_index * 8 + row;
				uint8_t bitmap_byte = koala.bitmap[bitmap_index];

				for (int px = 0; px < 4; px++){
					int color_index = (bitmap_byte >> (6 - 2 * px)) & 0x03;
					uint8_t *rgb = c64_colors[pixel_colors[color_index]];

					int base_x = block_x * 8 + px * 2;
					int y = block_y * 8 + row;

					for (int i = 0; i < 2; i++){
						int x = base_x + i;
						if (x >= 320 || y >= 200) continue;

						size_t offset = (y * 320 + x) * BYTES_PER_PIXEL;
						fb_data[offset]     = rgb[0]; // R
						fb_data[offset + 1] = rgb[1]; // G
						fb_data[offset + 2] = rgb[2]; // B
					}
				}
			}
		}
	}
}

void ctoy_begin(){
	m_image_create(&framebuffer, M_UBYTE, 320, 200, 3);

	const char *dateiname = "data/panda_gangsta_gardener.kla";
	mem = readFileToMemory(dateiname);

	if (mem.ptr){
		printf("Koala-Datei geladen: %zu Bytes\n", mem.size);
		assignHead(&mem);
		draw_koala();
	} else {
		printf("FEHLER: Datei konnte nicht geladen werden: %s\n", dateiname);
	}
}

void ctoy_main_loop(){
	ctoy_swap_buffer(&framebuffer);
}

void ctoy_end(){
	m_image_destroy(&framebuffer);
	free(koala.data);
}
