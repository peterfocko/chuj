#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POCET_FARIEB 4
#define POCET_CISIEL 8
#define POCET_KARIET_NA_RUKE 4
#define MAX_POCET_HRACOV 4
#define POCET_VYMIEN 10e5

typedef enum {ZELEN, CERVEN, ZALUD, GULA} FARBA;
typedef enum {SEDEM, OSEM, DEVAT, DESAT, NIZNIK, VYSNIK, KRAL, ESO} CISLO;

typedef struct {
    FARBA farba;
    CISLO cislo;
} KARTA;
typedef struct {
    KARTA karta[POCET_FARIEB * POCET_CISIEL];
    int pocet;
} BALIK;
typedef struct {
    KARTA karta[POCET_KARIET_NA_RUKE];
    int pocet;
} RUKA;
typedef struct {
    KARTA karta[POCET_FARIEB * POCET_CISIEL];
    int pocet;
} STOL;
typedef struct {
    RUKA ruka;
    STOL stol;
} HRAC;
typedef struct {
    BALIK balik;
    HRAC hrac[MAX_POCET_HRACOV];
    int pocetHracov;
} ZAPAS;

const char *FARBA_RETAZEC[] = {"Zelen", "Cerven", "Zalud", "Gula"};
const char *FARBA_PRIZNAK[] = {"\x1b[32m", "\x1b[31m", "\x1b[33m", "\x1b[34m"};
const char *CISLO_RETAZEC[] = {"Sedmicka", "Osmicka", "Deviatka", "Desiatka", "Niznik", "Vysnik", "Kral", "Eso"};

BALIK generujBalik();
void inicializujZapas(ZAPAS *zapas);
void zamiesajBalik(BALIK *balik);
void rozdajKartu(BALIK *balik, RUKA *ruka);
void rozdajKarty(ZAPAS *zapas);

void vypisKartu(KARTA karta);
void vypisRuku(RUKA ruka);
void vypisBalik(BALIK *balik);
void vypisHraca(HRAC *hrac);

int main() {
    srand(time(NULL));

    ZAPAS zapas;
    zapas.pocetHracov = 4;
    inicializujZapas(&zapas);
    rozdajKarty(&zapas);
    vypisBalik(&zapas.balik);
    printf("\n");
    for (int i = 0; i < zapas.pocetHracov; i++) {
        printf("Hrac %d:\n", i+1);
        vypisHraca(&zapas.hrac[i]);
    }

    return 0;
}

BALIK generujBalik() {
    BALIK balik;
    balik.pocet = POCET_FARIEB * POCET_CISIEL;
    for (int f = 0; f < POCET_FARIEB; f++)
        for (int c = 0; c < POCET_CISIEL; c++)
            balik.karta[f * POCET_CISIEL + c] = (KARTA) { f, c };

    return balik;
}

void zamiesajBalik(BALIK *balik) {
    for (int i = 0; i < POCET_VYMIEN; i++) {
        int prvaKartaIndex = rand() % (POCET_FARIEB * POCET_CISIEL);
        int druhaKartaIndex = rand() % (POCET_FARIEB * POCET_CISIEL);

        KARTA docasnaKarta = balik->karta[prvaKartaIndex];
        balik->karta[prvaKartaIndex] = balik->karta[druhaKartaIndex];
        balik->karta[druhaKartaIndex] = docasnaKarta;
    }
}

void rozdajKartu(BALIK *balik, RUKA *ruka) {
    ruka->karta[ruka->pocet++] = balik->karta[--balik->pocet];
}

void vypisKartu(KARTA karta) {
    printf("%s[%-7s::%s]\x1b[0m\n", FARBA_PRIZNAK[karta.farba], FARBA_RETAZEC[karta.farba], CISLO_RETAZEC[karta.cislo]);
}

void vypisRuku(RUKA ruka) {
    for (int i = 0; i < ruka.pocet; i++) {
        vypisKartu(ruka.karta[i]);
    }
}

void vypisBalik(BALIK *balik) {
    for (int i = 0; i < balik->pocet; i++)
        vypisKartu(balik->karta[i]);
}

void inicializujZapas(ZAPAS *zapas) {
    zapas->balik = generujBalik();
    zamiesajBalik(&zapas->balik);
    for (int i = 0; i < zapas->pocetHracov; i++) {
        zapas->hrac[i].ruka.pocet = 0;
        zapas->hrac[i].stol.pocet = 0;
    }
}

void vypisHraca(HRAC *hrac) {
    RUKA *ruka = &hrac->ruka;
    for (int i = 0; i < ruka->pocet; i++)
        vypisKartu(ruka->karta[i]);
    printf("\n");
}

void rozdajKarty(ZAPAS *zapas) {
    int pocetKarietNaRozdanie = zapas->balik.pocet / zapas->pocetHracov;
    if (pocetKarietNaRozdanie > POCET_KARIET_NA_RUKE)
        pocetKarietNaRozdanie = POCET_KARIET_NA_RUKE;

    for (int i = 0; i < zapas->pocetHracov; i++)
        for (int j = 0; j < pocetKarietNaRozdanie; j++)
            rozdajKartu(&zapas->balik, &zapas->hrac[i].ruka);
}
