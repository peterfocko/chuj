#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POCET_FARIEB 4
#define POCET_CISIEL 8
#define POCET_KARIET_NA_RUKE 4
#define MAX_POCET_HRACOV 4
#define POCET_TIMOV 2
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
    RUKA ruka;
    BALIK majetok;
} HRAC;
typedef struct {
    BALIK balik;
    BALIK stol;
    HRAC hrac[MAX_POCET_HRACOV];
    int zacinaKolo;
    int pocetHracov;
} ZAPAS;
typedef struct {
    ZAPAS zapas;
    int zacinaZapas;
    int skore[POCET_TIMOV];
} HRA;

const char *FARBA_RETAZEC[] = {"Zelen", "Cerven", "Zalud", "Gula"};
const char *FARBA_PRIZNAK[] = {"\x1b[32m", "\x1b[31m", "\x1b[33m", "\x1b[34m"};
const char *CISLO_RETAZEC[] = {"Sedmicka", "Osmicka", "Deviatka", "Desiatka", "Niznik", "Vysnik", "Kral", "Eso"};

BALIK generujBalik();
void inicializujHru(HRA *hra, int pocetHracov);
void inicializujZapas(HRA *hra);
void zamiesajBalik(BALIK *balik);
void rozdajKartu(BALIK *balik, RUKA *ruka);
void rozdajKarty(ZAPAS *zapas);
void vylozKartu(ZAPAS *zapas, int hrac, KARTA karta);
RUKA moznaRuka(ZAPAS *zapas);
void uzavriKolo(ZAPAS *zapas);
void zahrajKolo(ZAPAS *zapas);
void zahrajZapas(HRA* hra);
int spocitajMastneKarty(ZAPAS *zapas, int tim);
int spocitajKarty(ZAPAS* zapas, int tim);

void vypisKartu(KARTA karta);
void vypisBalik(BALIK *balik);
void vypisRuku(RUKA *ruka, int dalsieKolo);

int zadajVstup(RUKA *ruka, int dalsieKolo);

int main() {
    srand(time(NULL));

    HRA hra;
    inicializujHru(&hra, 4);
    while (1) {
        zahrajZapas(&hra);
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

void vypisBalik(BALIK *balik) {
    for (int i = 0; i < balik->pocet; i++)
        vypisKartu(balik->karta[i]);
}

void inicializujHru(HRA *hra, int pocetHracov) {
    for (int i = 0; i < POCET_TIMOV; i++)
        hra->skore[i] = 0;
    hra->zacinaZapas = 0;
    hra->zapas.pocetHracov = pocetHracov;
}

void inicializujZapas(HRA *hra) {
    ZAPAS *zapas = &hra->zapas;
    zapas->balik = generujBalik();
    zamiesajBalik(&zapas->balik);
    zapas->stol.pocet = 0;
    zapas->zacinaKolo = hra->zacinaZapas;
    for (int i = 0; i < zapas->pocetHracov; i++) {
        zapas->hrac[i].ruka.pocet = 0;
        zapas->hrac[i].majetok.pocet = 0;
    }
}

void vypisRuku(RUKA *ruka, int dalsieKolo) {
    if (dalsieKolo)
        printf("(0) Ukoncit kolo\n");

    for (int i = 0; i < ruka->pocet; i++) {
        printf("(%d) ", i+1);
        vypisKartu(ruka->karta[i]);
    }
    printf("\n");
}

void rozdajKarty(ZAPAS *zapas) {
    int pocetKarietNaRozdanie = POCET_KARIET_NA_RUKE - zapas->hrac[zapas->zacinaKolo].ruka.pocet;
    if (zapas->balik.pocet / zapas->pocetHracov < pocetKarietNaRozdanie)
        pocetKarietNaRozdanie = zapas->balik.pocet / zapas->pocetHracov;

    for (int i = 0; i < zapas->pocetHracov; i++)
        for (int j = 0; j < pocetKarietNaRozdanie; j++)
            rozdajKartu(&zapas->balik, &zapas->hrac[(zapas->zacinaKolo + i) % zapas->pocetHracov].ruka);
}

int zadajVstup(RUKA *ruka, int dalsieKolo) {
    int min = dalsieKolo ? 0 : 1;
    int vstup;
    while (1) {
        printf("Zadaj kartu: ");
        if (scanf("%d", &vstup) < 1) {
            printf("Nevalidny vstup.\n");
            while (getchar() != '\n');
            continue;
        }
        if (vstup < min || vstup > ruka->pocet) {
            printf("Ocakavane cislo medzi %d a %d.\n", min, ruka->pocet);
            continue;
        }
        break;
    }
    return vstup;
}

void vylozKartu(ZAPAS *zapas, int hrac, KARTA karta) {
    RUKA *ruka = &zapas->hrac[hrac].ruka;
    int kartaIndex = -1;
    for (int i = 0; i < ruka->pocet; i++) {
        KARTA k = ruka->karta[i];
        if (k.cislo == karta.cislo && k.farba == karta.farba) {
            kartaIndex = i;
            break;
        }
    }

    zapas->stol.karta[zapas->stol.pocet++] = ruka->karta[kartaIndex];
    ruka->pocet--;
    for (int i = kartaIndex; i < ruka->pocet; i++)
        ruka->karta[i] = ruka->karta[i+1];
}

RUKA moznaRuka(ZAPAS *zapas) {
    RUKA moznaRuka;
    moznaRuka.pocet = 0;

    CISLO spodneCislo = zapas->stol.karta[0].cislo;
    RUKA *ruka = &zapas->hrac[zapas->zacinaKolo].ruka; 
    for (int i = 0; i < ruka->pocet; i++)
        if (ruka->karta[i].cislo == SEDEM || ruka->karta[i].cislo == spodneCislo)
            moznaRuka.karta[moznaRuka.pocet++] = ruka->karta[i];

    return moznaRuka;
}

void uzavriKolo(ZAPAS* zapas) {
    CISLO spodneCislo = zapas->stol.karta[0].cislo;
    int vitaz = zapas->zacinaKolo;
    for (int i = 1; i < zapas->stol.pocet; i++) {
        CISLO cislo = zapas->stol.karta[i].cislo;
        int poslednaKarta = zapas->balik.pocet == 0 && i != zapas->stol.pocet - 1 && zapas->hrac[zapas->zacinaKolo].ruka.pocet == 0;
        if ((cislo == spodneCislo) || (cislo == SEDEM && !poslednaKarta))
            vitaz = (zapas->zacinaKolo + i) % zapas->pocetHracov;
    }

    for (int i = 0; i < zapas->stol.pocet; i++)
        zapas->hrac[vitaz].majetok.karta[zapas->hrac[vitaz].majetok.pocet++] = zapas->stol.karta[i];
    zapas->stol.pocet = 0;
    zapas->zacinaKolo = vitaz;
}

void zahrajKolo(ZAPAS *zapas) {
    while (zapas->hrac[zapas->zacinaKolo].ruka.pocet > 0)
        for (int i = 0; i < zapas->pocetHracov; i++) {
            int hrac = (zapas->zacinaKolo + i) % zapas->pocetHracov;
            int dalsieKolo = zapas->stol.pocet > 0 && i == 0;
            RUKA ruka = dalsieKolo ? moznaRuka(zapas) : zapas->hrac[hrac].ruka;
            if (ruka.pocet == 0)
                return;

            printf("Stol:\n");
            vypisBalik(&zapas->stol);
            printf("\n");

            printf("Hrac %d (Tim %d):\n", hrac+1, hrac % POCET_TIMOV + 1);
            vypisRuku(&ruka, dalsieKolo);
            int vstup = zadajVstup(&ruka, dalsieKolo);
            printf("\n");
            if (vstup == 0)
                return;

            KARTA karta = ruka.karta[vstup - 1];
            vylozKartu(zapas, hrac, karta);
        }
}

void zahrajZapas(HRA* hra) {
    ZAPAS *zapas = &hra->zapas;
    inicializujZapas(hra);
    int zacinajuciTim = zapas->zacinaKolo % POCET_TIMOV;

    printf("*** Zacina zapas, zacina tim %d ***\n", zacinajuciTim + 1);
    for (int i = 0; i < POCET_TIMOV; i++)
        printf("Skore Tim %d: %d\n", i+1, hra->skore[i]);
    printf("\n");

    while (zapas->balik.pocet > 0 || zapas->hrac[zapas->zacinaKolo].ruka.pocet > 0) {
        rozdajKarty(zapas);
        zahrajKolo(zapas);
        uzavriKolo(zapas);
        printf("*** Koniec kola, vitazi Hrac %d ***\n\n", zapas->zacinaKolo+1);
    }

    printf("*** Koniec zapasu ***\n");
    int mastneKarty[POCET_TIMOV];
    for (int i = 0; i < POCET_TIMOV; i++) {
        mastneKarty[i] = spocitajMastneKarty(zapas, i);
        printf("Mastne karty Tim %d: %d\n", i+1, mastneKarty[i]);
    }

    if (mastneKarty[zacinajuciTim] > 2)
        hra->zacinaZapas = (hra->zacinaZapas + POCET_TIMOV) % hra->zapas.pocetHracov;
    else
        hra->zacinaZapas = (hra->zacinaZapas + 1) % hra->zapas.pocetHracov;

    for (int i = 0; i < POCET_TIMOV; i++)
        if (spocitajKarty(zapas, i) == POCET_FARIEB * POCET_CISIEL) {
            hra->skore[i]++;
            printf("*** Tim %d zobral vsetky karty, ziskava bod! ***\n", i+1);
            break;
        }
    printf("\n");
}

int spocitajMastneKarty(ZAPAS *zapas, int tim) {
    int mastneKarty = 0;
    for (int i = tim; i < zapas->pocetHracov; i+=POCET_TIMOV)
        for (int j = 0; j < zapas->hrac[i].majetok.pocet; j++)
            if (zapas->hrac[i].majetok.karta[j].cislo == DESAT || zapas->hrac[i].majetok.karta[j].cislo == ESO)
                mastneKarty++;
    return mastneKarty;
}

int spocitajKarty(ZAPAS* zapas, int tim) {
    int pocetKariet = 0;
    for (int i = tim; i < zapas->pocetHracov; i+=POCET_TIMOV)
        pocetKariet += zapas->hrac[i].majetok.pocet;
    return pocetKariet;
}
