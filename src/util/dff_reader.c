#include <ctoy.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// DFF Header Struktur
#pragma pack(push, 1) 
typedef struct {
    char type[3];          // Header-Typ (3 Bytes)
    uint16_t offset;       // Offset des Textes (2 Bytes)
    uint8_t length;        // Länge der Nachricht (1 Byte)
} DFFHeader;
#pragma pack(pop)

typedef struct{
    uint8_t *ptr;          // Zeiger auf den Dateiinhalt
    size_t size;           // Größe der Datei
} MEMPTR;

MEMPTR memptr;            // Speicherzeiger für die gelesene Datei
DFFHeader *dffheader;     // Zeiger auf die Header-Struktur

// every func in code:
MEMPTR readFileToMemory(const char *filename);
void assignHead(void);
void printHead(void);
void printDFFMessage(void);
void ctoy_begin(void);
void ctoy_main_loop(void);
void ctoy_end(void);

// Liest die Datei in den Speicher und gibt einen MEMPTR zurück
MEMPTR readFileToMemory(const char *filename){
    MEMPTR mem = {NULL, 0};
    FILE* fp = fopen(filename, "rb");
    if (!fp){
        perror("Datei konnte nicht geöffnet werden");
        return mem;
    }

    // Datei-Größe herausfinden
    if (fseek(fp, 0L, SEEK_END) != 0){
        perror("Fehler beim Suchen der Datei");
        fclose(fp);
        return mem;
    }

    mem.size = ftell(fp);      // Hole die Dateigröße
    printf("%zu\n", mem.size); // Ausgabe der Dateigröße
    rewind(fp);                // Setze den Dateizeiger zurück auf Anfang

    // Speicher für die Datei reservieren
    mem.ptr = malloc(mem.size);
    if (!mem.ptr){
        perror("Speicher konnte nicht zugewiesen werden");
        fclose(fp);
        return mem;
    }

    // Lese die Datei
    if (fread(mem.ptr, 1, mem.size, fp) != mem.size){
        perror("Fehler beim Lesen der Datei");
        free(mem.ptr);
        mem.ptr = NULL;
        mem.size = 0;
    }

    fclose(fp);
    return mem;
}

// Weist den Header und die Datei zu
void assignHead() {
    if (!memptr.ptr || memptr.size < sizeof(DFFHeader)) {
        printf("assignDFFHead(): Ungültiger Zeiger oder Datei ist zu klein\n");
        return;
    }

    // Überprüfen der ersten 3 Bytes auf "DFF"
    if (memptr.ptr[0] != 'D' || memptr.ptr[1] != 'F' || memptr.ptr[2] != 'F') {
        printf("assignDFFHead(): Keine DFF-Datei!\n");
        return;
    }

    // Header zuweisen, wenn gültig
    dffheader = (DFFHeader *)memptr.ptr;
}

// Gibt den Header aus
void printHead() {
    printf("HEADER\n");
    printf("Typ:         %c %c %c\n", 
           dffheader->type[0], dffheader->type[1], dffheader->type[2]);
    printf("Offset:      %u Bytes\n", dffheader->offset);
    printf("Länge:       %u Bytes\n", dffheader->length);
}

// Gibt die Nachricht der DFF-Datei aus
void printDFFMessage() {
    // Überprüfen, ob eine Nachricht vorhanden ist
    if (dffheader->length == 0) {
        return;
    }

    // Überprüfen, ob Offset + Länge innerhalb der Dateigröße liegen
    if (dffheader->offset + dffheader->length > memptr.size) {
        printf("\nprintDFFMessage(): Nachricht außerhalb des gültigen Bereichs!\n");
        return;
    }

    // Nachricht ausgeben
    for (uint8_t i = 0; i < dffheader->length; i++) {
        printf("%c", memptr.ptr[dffheader->offset + i]);
    }
    printf("\n");
}

// Initialisierung der CtoY-Umgebung
void ctoy_begin() {
    // Ausgabe der aktuellen Zeit
    struct tm *tm_info; 
    time_t t = time(NULL); 
    tm_info = localtime(&t); 
    printf("\n=== %02d:%02d:%02d",  
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    const char* filename = "data/data1.dff"; // DFF-Datei
    printf("%s\n\n", filename);
    
    // Datei lesen und Header zuweisen
    memptr = readFileToMemory(filename);
    if (memptr.ptr) {
        assignHead();
        printHead();
        printDFFMessage();
    }
}

// Hauptloop der CtoY-Umgebung (hier leer, da keine Animation)
void ctoy_main_loop() {
    // Kein Inhalt im Hauptloop für diese Version
}

// Beendet die CtoY-Umgebung und gibt Ressourcen frei
void ctoy_end() {
    if (memptr.ptr) {
        free(memptr.ptr); // Speicher freigeben
        memptr.ptr = NULL;
        memptr.size = 0;
    }
}
