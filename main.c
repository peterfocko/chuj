#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define POCET_FARIEB 4
#define POCET_CISIEL 8
#define POCET_KARIET_NA_RUKE 4
#define MAX_POCET_HRACOV 4
#define POCET_TIMOV 2
#define POCET_VYMIEN 10e5

#define PROTOKOL "TCP"
#define PORT 1337u
#define VELKOST_BUFFERA 80
#define SPRAVA_NOVY_KLIENT "AHOJ"
#define SPRAVA_OK "OK"

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
int zistiServer();
int zistiPocetHracov();
in_addr_t zistiAdresuServera();

void spustiServer();
void spustiKlient();

int vytvorServerSocket(int pocetHracov);
void serializujBalik(BALIK *balik, char *buffer);
void pripojKlientov(int serverSoket, int *klientSocket, int pocetHracov);
void precitajSpravu(int soket, char *buffer);
void posliSpravu(int soket, char *buffer);

int main() {
    srand(time(NULL));

    printf("*** CHUJ ****\n");
    if (zistiServer())
        spustiServer();
    else
        spustiKlient();

    return 0;

    // int pocetHracov = 4;

    // HRA hra;
    // inicializujHru(&hra, 4);
    // inicializujZapas(&hra);
    // zamiesajBalik(&hra.zapas.balik);

    // char buffer[VELKOST_BUFFERA + 1];
    // serializujBalik(&hra.zapas.balik, buffer);
    // for (int i = 0; i < pocetHracov - 1; i++) {
    //     posliSpravu(klientSoket[i], buffer);
    //     char vstupnyBuffer[VELKOST_BUFFERA + 1];
    //     precitajSpravu(klientSoket[i], vstupnyBuffer);
    //     if (strcmp(vstupnyBuffer, SPRAVA_OK) != 0) {
    //         printf("Invalidna sprava od Hrac %d: %s\n", i+2, vstupnyBuffer);
    //         exit(EXIT_FAILURE);
    //     }
    // }

    // for (int i = 0; i < pocetHracov - 1; i++)
    //     close(klientSoket[i]);
    // close(serverSocket);

    // HRA hra;
    // inicializujHru(&hra, 4);
    // while (1) {
    //     zahrajZapas(&hra);
    // }
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
            while (getchar() != '\n');
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
    zamiesajBalik(&hra->zapas.balik);
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

void precitajSpravu(int socket, char *buffer) {
    int i;
    for (i = 0; i < VELKOST_BUFFERA; i++) {
        int pocet = read(socket, &buffer[i], 1);
        if (pocet == -1) {
            perror("Chyba citania socketu");
            exit(EXIT_FAILURE);
        }
        if (pocet == 0 || buffer[i] == '\n')
            break;
    }
    buffer[i] = '\0';
}

void posliSpravu(int soket, char *buffer) {
    if (write(soket, buffer, strlen(buffer)) == -1) {
        perror("Chyba zapisu socketu");
        exit(EXIT_FAILURE);
    }
}

int vytvorServerSocket(int pocetHracov) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, getprotobyname(PROTOKOL)->p_proto);
    if (serverSocket == -1) {
        perror("Vytvorenie socketu zlyhalo");
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt(SO_REUSEADDR) zlyhalo");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in adresaServera;
    adresaServera.sin_family = AF_INET;
    adresaServera.sin_addr.s_addr = htonl(INADDR_ANY);
    adresaServera.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr *) &adresaServera, sizeof(adresaServera)) == -1) {
        perror("Pripojenie socketu zlyhalo");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, pocetHracov - 1) == -1) {
        perror("Pocuvanie na sockete zlyhalo");
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

int vytvorKlientSocket(in_addr_t serverHost) {
    int klientSocket = socket(AF_INET, SOCK_STREAM, getprotobyname(PROTOKOL)->p_proto);
    if (klientSocket == -1) {
        perror("Vytvorenie socketu zlyhalo");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in adresaServera;
    adresaServera.sin_family = AF_INET;
    adresaServera.sin_addr.s_addr = serverHost;
    adresaServera.sin_port = htons(PORT);

    if (connect(klientSocket, (struct sockaddr *) &adresaServera, sizeof(adresaServera)) == -1) {
        perror("Pripojenie na server zlyhalo");
        exit(EXIT_FAILURE);
    }

    return klientSocket;
}

void pripojKlientov(int serverSocket, int *klientSocket, int pocetHracov) {
    int hrac = 0;
    while (hrac < pocetHracov - 1) {
        printf("Cakam pripojenie Hrac %d\n", hrac+2);
        struct sockaddr_in adresaKlienta;
        socklen_t klientVelkost = (socklen_t) sizeof(adresaKlienta);
        klientSocket[hrac] = accept(
            serverSocket,
            (struct sockaddr*)&adresaKlienta,
            &klientVelkost
        );

        char buffer[VELKOST_BUFFERA + 1];
        precitajSpravu(klientSocket[hrac], buffer);
        if (strcmp(buffer, SPRAVA_NOVY_KLIENT) != 0) {
            printf("Invalidna sprava: %s, zatvaram socket.\n", buffer);
            close(klientSocket[hrac]);
            continue;
        }

        sprintf(buffer, "H%dI%d\n", pocetHracov, hrac+1);
        posliSpravu(klientSocket[hrac], buffer);

        precitajSpravu(klientSocket[hrac], buffer);
        if (strcmp(buffer, SPRAVA_OK) != 0) {
            printf("Invalidna sprava: %s, zatvaram socket.\n", buffer);
            close(klientSocket[hrac]);
            continue;
        }
        printf("Hrac %d uspesne pripojeny.\n", hrac+2);
        hrac++;
    }
}

void serializujBalik(BALIK *balik, char *buffer) {
    for (int i = 0; i < POCET_FARIEB * POCET_CISIEL; i++) {
        buffer[i*2] = '0' + balik->karta[i].farba;
        buffer[i*2+1] = '0' + balik->karta[i].cislo;
    }
    buffer[POCET_FARIEB * POCET_CISIEL] = '\n';
    buffer[POCET_FARIEB * POCET_CISIEL + 1] = '\0';
}

int zistiServer() {
    printf("Spustit: \n(S) Server\n(K) Klient\n");
    while (1) {
        char vstup;
        printf("Zadaj moznost S|K: ");
        if (scanf("%c", &vstup) < 1) {
            printf("Nevalidny vstup.\n");
            while (getchar() != '\n');
            continue;
        }
        if (vstup != 'S' && vstup != 'K' && vstup != 's' && vstup != 'k') {
            printf("Ocakavana moznost S alebo K.\n");
            while (getchar() != '\n');
            continue;
        }
        printf("\n");
        return vstup == 'S' || vstup == 's' ? 1 : 0;
    }
}

int zistiPocetHracov() {
    while (1) {
        int vstup;
        printf("Zadaj pocet hracov 2|4: ");
        if (scanf("%d", &vstup) < 1) {
            printf("Nevalidny vstup.\n");
            while (getchar() != '\n');
            continue;
        }
        if (vstup != 2 && vstup != 4) {
            printf("Ocakavana moznost 2 alebo 4.\n");
            while (getchar() != '\n');
            continue;
        }
        return vstup;
    }
}

in_addr_t zistiAdresuServera() {
    while (1) {
        printf("Zadaj IP servera: ");
        char vstup[20];
        struct hostent* host;
        in_addr_t adresa;
        if (scanf("%s", vstup) < 1 || (host = gethostbyname(vstup)) == NULL || (adresa = inet_addr(inet_ntoa(*(struct in_addr *) *(host->h_addr_list)))) == (in_addr_t)-1) {
            printf("Nevalidny vstup.\n");
            while (getchar() != '\n');
            continue;
        }
        return adresa;
    }
}

void spustiServer() {
    printf("*** CHUJ SERVER ***\n");
    int pocetHracov = zistiPocetHracov();
    printf("Startujem server pre %d hracov.\n", pocetHracov);

    int serverSocket = vytvorServerSocket(pocetHracov);
    printf("Pocuvam na porte %u\n\n", PORT);

    int klientSocket[pocetHracov - 1];
    pripojKlientov(serverSocket, klientSocket, pocetHracov);
}

void spustiKlient() {
    printf("*** CHUJ SERVER ***\n");
    in_addr_t serverHost = zistiAdresuServera();
    int klientSocket = vytvorKlientSocket(serverHost);
}
