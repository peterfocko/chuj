#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POCET_FARIEB 4
#define POCET_CISIEL 8
typedef enum {ZELEN, CERVEN, ZALUD, GULA} FARBA;
typedef enum {SEDEM, OSEM, DEVAT, DESAT, NIZNIK, VYSNIK, KRAL, ESO} CISLO;
typedef struct {
    FARBA farba;
    CISLO cislo;
} KARTA;
typedef struct {
    KARTA karta[32];
    int pocet;
} BALIK;

const char *farbaRetazec[] = {"Zelen", "Cerven", "Zalud", "Gula"};
const char *farbaPriznak[] = {"\x1b[32m", "\x1b[31m", "\x1b[33m", "\x1b[34m"};
const char *cisloRetazec[] = {"Sedmicka", "Osmicka", "Deviatka", "Desiatka", "Niznik", "Vysnik", "Kral", "Eso"};

void vypisKartuFensi(KARTA karta);
void vypisKartuBejzik(KARTA karta);
BALIK generujBalik();
void zamiesajBalik(BALIK *balik, int pocetVymien);

int main() {
    srand(time(NULL));
    BALIK balik = generujBalik();
    zamiesajBalik(&balik, 10e5);

    for (int i = 0; i < POCET_FARIEB * POCET_CISIEL; i++)
        vypisKartuFensi(balik.karta[i]);
}

BALIK generujBalik() {
    BALIK balik;
    balik.pocet = POCET_FARIEB * POCET_CISIEL;
    for (int f = 0; f < POCET_FARIEB; f++)
        for (int c = 0; c < POCET_CISIEL; c++)
            balik.karta[f * POCET_CISIEL + c] = (KARTA) { f, c };

    return balik;
}

void zamiesajBalik(BALIK *balik, int pocetVymien) {
    for (int i = 0; i < pocetVymien; i++) {
        int prvaKartaIndex = rand() % (POCET_FARIEB * POCET_CISIEL);
        int druhaKartaIndex = rand() % (POCET_FARIEB * POCET_CISIEL);

        KARTA docasnaKarta = balik->karta[prvaKartaIndex];
        balik->karta[prvaKartaIndex] = balik->karta[druhaKartaIndex];
        balik->karta[druhaKartaIndex] = docasnaKarta;
    }
}

void vypisKartuBejzik(KARTA karta) {
    printf("[%d:%d]\n", karta.farba, karta.cislo);
}

void vypisKartuFensi(KARTA karta) {
    printf("%s[%-7s::%s]\x1b[0m\n", farbaPriznak[karta.farba], farbaRetazec[karta.farba], cisloRetazec[karta.cislo]);
}