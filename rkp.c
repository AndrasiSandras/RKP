#include <stdio.h> // Alapveto bemeneti/kimeneti muveletekhez szukséges fejlec.
#include <stdlib.h> // Altalanos celu fuggvenyekhez szukseges fejlec.
#include <string.h> // Karakterlancmuveletekhez szukséges fejléc.
#include <time.h> // Idomereshez szukseges fejlec.
#include <unistd.h> // POSIX operacios rendszer hivasokhoz szukseges fejlec.
#include <sys/stat.h> // Fajlkezeleshez szukseges fejlec.
#include <fcntl.h> // Fajlkezeleshez szukseges fejlec.
#include <ctype.h> // Karaktermuveletekhez szukseges fejlec.
#include <dirent.h> // Konyvtarkezeléshez szukséges fejlec.
#include <signal.h> // Szignalkezeleshez szukseges fejlec.
#include <sys/socket.h> // Socket programozashoz szukséges fejlec.
#include <netinet/in.h> // Internetes cimekhez es strukturakhoz szukseges fejléec.
#include <arpa/inet.h> // Internetes cimek atalakitasahoz szukseges fejlec.
#include <pthread.h> // Tobbszalu programozashoz szukseges fejlec.
#include <omp.h> // OpenMP paralell programozashoz szukséges fejlec.

#include "RKP_header_file.h"

#define BMP_HEADER_SIZE 62
#define DEFAULT_MODE 0
#define DEFAULT_PROTOCOL 0

#define VERSION_NUMBER "1.0"
#define COMPLETION_DATE "2024.05.01"
#define DEVELOPER_NAME "Toth Zsolt"
#define NEPTUN_CODE "HL50I9"

/*
8. feladat
Az alábbiak szerint módosítsd a főprogram azon részét, amely kizárólag a ”--version” kapcsoló
használata esetén aktív! A kiíratandó összes sort más-más szál (thread) valósítsa meg párhuzamosan,
így a kimenetek a képernyőn „véletlenszerű” sorrendben jelennek meg.
*/

void print_version_info()
{
#pragma omp parallel sections // OpenMP szakaszok kezelese
    {
#pragma omp section // Szakasz definícioja
        printf("Version: %s\n", VERSION_NUMBER);

#pragma omp section // Kovetkezo szakasz definicioja
        printf("Date: %s\n", COMPLETION_DATE);

#pragma omp section // Kovetkezo szakasz definicioja
        printf("Developer and neptun code: %s %s\n", DEVELOPER_NAME, NEPTUN_CODE);
    } 
}

// 1. feladat
void print_help_info()
{
    puts("--help:");
    puts("\t--version : Display version information,");
    puts("\t\t    completion date, developer name and neptun code.");
    puts("");
    puts("\t-send     : Set program mode to send.");
    puts("\t-receive  : Set program mode to receive.");
    puts("\t-file     : Use file communication mode.");
    puts("\t-socket   : Use socket communication mode.");
}

/*
7. feladat
1. lépés
Írj egy szignálkezelő eljárást (természetesen ez is a header állományba kerüljön), amely háromféle
szignál kezelésére képes és az alábbi fejléccel rendelkezik:
void SignalHandler(int sig);
Ha az eljárás megszakítási szignált (SIGINT) kap, akkor írjon ki egy elköszönő üzenetet és a program
álljon le és adjon vissza egy 0 értéket!
Ha az eljárás felhasználói 1-es szignált (SIGUSR1) kap, akkor írjon ki egy hibaüzenetet arra
vonatkozóan, hogy a fájlon keresztüli küldés szolgáltatás nem elérhető!
Ha az eljárás alarm szignált (SIGALRM) kap, akkor írjon ki egy hibaüzenetet arra vonatkozóan, hogy a
szerver nem válaszol (időkereten belül), majd a program egy hibakóddal álljon le!
2. lépés
A főprogramot bármiféle kommunikáció megvalósítása előtt készítsd fel SIGINT és SIGUSR1
szignálok érkezésére, amelyeket a SignalHandler eljárás fog lekezelni!
Módosítsd továbbá a SendViaSocket alprogramot úgy, hogy az első üzenet elküldése után érkező
SIGALRM szignál esetén automatikusan hívja meg a SignalHandler szignálkezelő eljárást, majd
indíts el egy 1 másodperces időzítőt! Amennyiben az első üzenet elküldése utáni egy másodpercben
választ kap a szervertől a program, akkor hagyja figyelmen kívül az érkező alarm szignált!

SIGINT = a felhasznalo megnyomta a ctrl + c kombot (exit 0) 

SIGUSR1 = a fájlon keresztüli küldés szolgáltatás nem elérhető! (exit 1) 

SIGALRM = az időkorlát lejárt (exit 4) (Szerver időtúllépés!) 
*/

void SignalHandler(int sig)
{
    if (sig == SIGINT)
    {
        puts("The process is being stopped");
        exit(EXIT_SUCCESS);
    }
    else if (sig == SIGUSR1)
    {
        fprintf(stderr, "ERROR! (Send via file service is not available)\n");
        exit(EXIT_FAILURE);
    }
    else if (sig == SIGALRM)
    {
        fprintf(stderr, "ERROR! (The server is not responding)\n");
        exit(4);
    }
}

// az uzenet kuldes/fogadas es annak modja (file/socket), a parancssori argumentumok szama es azoknak tombje 
int Commands(int* send_mode, int* file_mode, int argc, char *argv[])
{
    char *arg[] = {"--version", "--help", "-send", "-receive", "-file", "-socket"}; // Elore definialt parancssori argimentumok
    int size = sizeof arg / sizeof arg[0];

    // dinamikusan meghatározza az arg[] tömb méretét    

    int check_count = 0; // Ellenorzott argumentumok szamlaloja, ez keell hogy megnezzuk helyes e az altalunk megadott parancs
    int send_set = 0; // Leellenorizzuk hogy csak egy parancsot adunk meg ami a kuldest/fogadast allitja
    int file_set = 0; // -||- filet/socketet allitja

    // Ha vannak parancssori argumentumok
    if (argc > 1) //belep ha a parancssori argumentumok szama > 1
    {
        for (int k = 0; k < size; ++k) // Parancssori argumentumok feldolgozasa (char *arg[]) meretig megy az az sizeof arg / sizeof arg[0];, az itt dinamikusan lefoglat ertekig
        {
            for (int i = 1; i < argc; ++i) // itt pedig vegigmegyunk az osszes parancssori argumentumon amit megadtott a felhasznalo (argc)
            {
                if (strcmp(arg[k], argv[i]) == 0) // (strcmp = (ket string egyezese) == 0, ha egyezik)
                {
                    check_count++; //egyezes utan megnoveli a valtozot mivel a felhasznalo helyes erteket adott meg

                    /*
                    osszehasonlitja a paracsok tomb (*arg[]) elemeit a felhasznalo altal megadott parancssori
                    argumentum/argumentumokkal es ha egyezes van bealitja az adott modot (send, receive, file, socket)

                    a *send_mode egy mutató, amely tárolja, hogy a program milyen módban fusson: küldési (0) vagy fogadási (1) módban.

                    a *file_mode szintén egy mutató, ami tárolja, hogy a program fájlalapú (0) vagy socketalapú (1) kommunikációban fusson.

                    a send_set es a file_set pedig ellenorzi hogy csak egy koldo/fogado vagy file/socket kapcsolot adott meg a felhasznalo (nem engedi pl a -file -socketet)
                    */
                    if (strcmp(arg[k], "-send") == 0)
                    {
                        *send_mode = 0;
                        send_set++;
                    } 
                    else if (strcmp(arg[k], "-receive") == 0)
                    {
                        *send_mode = 1;
                        send_set++;
                    }
                    else if (strcmp(arg[k], "-file") == 0)
                    {
                        *file_mode = 0;
                        file_set++;
                    }
                    else if (strcmp(arg[k], "-socket") == 0)
                    {
                        *file_mode = 1;
                        file_set++;
                    }
                }
            }
        }

        /*
        ellenorzi nekunk hogy biztos csak egy kuldo/fogado vagy egy file/socket parancsot adtunk e meg
        valamint azt is megnezi hogy ezekbol nincs e esetleg 3 (csak 2 parancsot enged hogy megadjon a felhasznalo)
        */

        if (send_set > 1 || file_set > 1 || send_set + file_set > 2)
        {
            fprintf(stderr, "Invalid combination of arguments.\n");
            print_help_info();

            *send_mode = DEFAULT_MODE;
            *file_mode = DEFAULT_PROTOCOL;

            exit(EXIT_FAILURE);
        }

        /*
        ha nem egyezik az ellenorzott argumentumok szama az osszes parancssori argumentum szamaval -1 (./chart)
        ha nem egyezik akkor hibat irunk a stderr, standerd error outputra, meghivjuk a helpet, visszaallitunk mindent default modba (kuldes, file)
        es egy 1es hibakoddal kilepunk
        */

        if (check_count != argc - 1)
        {
            fprintf(stderr, "Invalid argument.\n");
            print_help_info();

            *send_mode = DEFAULT_MODE;
            *file_mode = DEFAULT_PROTOCOL;

            exit(EXIT_FAILURE);
        }

        /*
        vegig megy a felhasznalo altal megadott parancssori argumentumokon
        es keresi azt ami egyezik a --version es --help stringgel
        ha ezeket megtalalja meghivja az adott eljarast majd 0 koddal kilep
        */

        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "--help") == 0)
            {
                print_help_info();
                exit(EXIT_SUCCESS);
            }
            else if (strcmp(argv[i], "--version") == 0)
            {
                print_version_info();
                exit(EXIT_SUCCESS);
            }
        }
    }
    else // Ha nincsenek parancssori argumentumok
    {
        *send_mode = DEFAULT_MODE;
        *file_mode = DEFAULT_PROTOCOL;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
2. feladat
Egy képzeletbeli szenzor által rendszeres időközönként mért értékeket egy függvénnyel állítjuk elő. Ez
a függvény egy 3 állapotú 1 dimenziós bolyongást implementál, azaz véletlenszerűen előállítja egész
számok egy véges sorozatát, ahol bármely két szomszédos elem különbségének az abszolút érteke
maximum 1. A kezdő érték legyen az x0=0! A további elemeknél az xi+1=xi+1 eset 42,8571%
eséllyel forduljon elő! Az esetek 11/31-ed részében xi+1=xi-1 állítás teljesüljön! A szomszédos
értékek néha lehetnek azonosak is (xi+1=xi). A „mért” értékek darabszáma egyezzen meg a hívás
pillanatában az adott negyedórából eltelt másodpercek számának és a 100-nak a maximumával!
(Például délelőtt 9 óra 41 perc 27 másodperckor 687 darab egész számot kell generálni, de 10 óra 15
perc 24 másodperckor 100 darab egész számot.) A program minden futtatás esetén másik számsort
állítson elő!
A függvény fejléce a következő legyen:
int Measurement(int **Values);Az egyetlen paraméter egy int* típusú pointer címe legyen, mivel ez majd output paraméteként
szolgál (pszeudocím szerinti paraméterátadású mutató). Ebben a függvényben foglalj le dinamikus
memóriafoglalás segítségével egy megfelelő méretű tömböt, ebben legyen tárolva az előállított
véletlenszerű adatsorozat és a terület címe legyen elmentve azon a memóriacímen, amit
paraméterként kap a függvény (ezáltal az itt lefoglalt és inicializált terület címe visszajut a hívó
programegységbe, ami így el tudja érni az adatsort). Végül a függvény térjen vissza az előállított
értékek számával!
*/

int Measurement(int **Values)
{
    // Ido kezeles
    time_t rawTime;
    struct tm *currentTime; // itt meg csak egy pointer, ures

    // Aktualis ido lekerese
    time(&rawTime); // lekerdezzuk az aaktualis idot es elmentjuk a rawTime valtozoba
    currentTime = localtime(&rawTime); // pointer inicializalasa, a local time konvertaljuk az aktualis idot helyi idore majd elmentjuk a currentTime valtozoba

    /*
    mert ertekek szamanak kiszamitasa
    az aktuális idő alapján kiszámoljuk a mérési értékek számát a perc és másodperc összegének szorzatával.

    currentTime -> tm_min: Ez a kifejezés a currentTime pointeren keresztül elérhető tm_min mezőre hivatkozik,
    amely a struct tm struktúrában tárolja az aktuális perc értékét.

    currentTime -> tm_sec: Ez a kifejezés a currentTime pointeren keresztül elérhető tm_sec mezőre hivatkozik,
    amely a struct tm struktúrában tárolja az aktuális másodperc értékét.

    (currentTime -> tm_min * 60): A perc értékét szorozzuk 60-cal, hogy az időt másodpercekben kapjuk meg.
    Mivel egy percben 60 másodperc van, ez a művelet az aktuális percek számát másodpercekben kifejezve.

    A measurementCount változó azt fogja tárolni, hogy az aktuális időben hány másodperc telt el az adott perc kezdetétől.
    */

    int measurementCount = (currentTime -> tm_min * 60) + currentTime -> tm_sec;

    // Korlatozas: maximum 100 meres
    // igazabol nem kell csak a bmp generalasanal azt vettem eszre szukseges mert a pontok "eltevednek" (kep)
    if (measurementCount > 100)
    {
        measurementCount = 100;
    }

    // Random szam generalo inicializalasa aktualis ido alapjan
    srand(time(NULL)); 

    // memoriafoglalas a mert ertekek tarolasara egy mutato tombbe
    *Values = (int *)malloc(measurementCount * sizeof(int));
    (*Values)[0] = 0; // Az elso ertek nullanak allitasa, mivel a malloc csak memóriát foglal, de nem inicializálja az értékeket

    // Adatok generalasa
    for(int i = 1; i < measurementCount; i++)
    {
        /*
        Veletlenszam generalasa es meresi adatok letrehozasa majd normalizalasa,
        azaz skálára hozzuk 0 és 1 között, azáltal, hogy elosztjuk a ((unsigned)RAND_MAX + 1) értékkel.
        */

        double rnd = (double)rand()/((unsigned)RAND_MAX+1);

        if(rnd <= 0.428571) // 42.8571%-os valoszinuseggel novekszik
        {
            (*Values)[i] = (*Values)[i - 1] + 1;
        }
        else if (rnd <= 0.783409) // 78.3409%-os valoszinuseggel csokken
        {
            (*Values)[i] = (*Values)[i - 1] - 1;
        }
        else // egyebkent az elozo ertekkel megegyezo marad
        {
            (*Values)[i] = (*Values)[i - 1];
        }
    }

    // Mert ertekek szamanak visszaadasa
    return measurementCount;
}

/*
3. feladat
Az előbbi feladatban előállított adatsort, mint egy időben változó fizikai mennyiség rendszeres
időközönként megvalósult mérésének az eredményét egy grafikonon szeretnénk ábrázolni. Ehhez az
adatsort, mint folytonos görbét egy BMP képfájl segítségével jelenítjük meg. A képfájlnak a
színmélysége legyen 1, azaz csak két szint tartalmazhasson (az egyik az adatpont, a másik a háttér)!
Nem kötelező, hogy ezek a fekete és a fehér színek legyenek, így a képfájlban definiálj egy
(tetszőleges) kétszínű palettát az RGBA színteret alkalmazva! A kép pixelben megadott szélessége és
magassága legyen egyenlő az ábrázolandó adatok számával! Ennek megfelelően tehát a kép minden
oszlopában pontosan egy pixel lesz előtér színű, az ábrázolandó adat. Az adatsort úgy jelenítsd meg,
hogy a legelső érték (x0=0) az első oszlopnak a (függőlegesen) középső pixele legyen és ennek
megfelelően jelenjen meg a többi érték is! (Amennyiben a kép magassága páros érték, akkor a két
középső pixel bármelyike használható.) Ha valamelyik megjelenítendő érték túl nagy vagy túl kicsi
lenne (azaz kilógna a képből), akkor azt a legfelső, illetve a legalsó pixel segítségével jelenítsd meg!

Egy példa grafikon az alábbi 21 elemű adatsor megjelenítésére:
{0, -1, -2, -1, -1, 0, 1, 0, 1, 2, 3, 3, 4, 3, 3, 4, 5, 6, 5, 5, 6}.
A képet létrehozó eljárás fejléce legyen az alábbi:
void BMPcreator(int *Values, int NumValues);
Az első paraméter az ábrázolandó értékeket tartalmazó tömb kezdőcíme, a második pedig az előbbi
tömbben tárolt egész értékek száma. A létrehozandó képfile neve legyen chart.bmp, abban a
könyvtárban legyen, hol a fogadó program el lett indítva és azt minden felhasználó olvashassa, de
csak a tulajdonosnak legyen írási joga hozzá!
Az egy bit színmélységű tömörítetlen bitmap (BMP) képfájlok felépítéséről a mellékletben olvashatsz.
*/


void BMPcreator(int *Values, int NumValues) 
{
    int imageHeight = NumValues;
    int imageWidth = NumValues;

    /*
    BMP fajl letrehozasa vagy megnyitasa irasra, ha mar letezik
    O_CREAT (lerehozza ha nincs)| O_WRONLY (csak irasra nyitja meg)| O_TRUNC (ez azt jelzi ha a file mar letezik),
    S_IRUSR (a tulajdonosnak ad olvasasi jogot)| S_IROTH (a file olvashato mas felhasznalonak)| S_IWUSR(irhato a file a tulajdonosanak).
    */
   
    int fileHandle = open("chart.bmp", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR| S_IRGRP | S_IROTH | S_IWUSR);
    
    // Hibaellenorzes: ha a fajl megnyitasa nem sikerult
    if (fileHandle == -1)
    {
        fprintf(stderr, "Failed to open BMP file!\n");
        exit(EXIT_FAILURE);
    }

    // Kep meretenek kiszamitasa
    unsigned int imageSize = (imageHeight * imageWidth/8)+BMP_HEADER_SIZE;

    /*
    BMP fejlec elokeszitese
    Ez a két bájt (hexadecimális értéke: 0x42 és 0x4d) azonosítja a fájlt, mint BMP fájlt.
    imageSize: A kép mérete byte-ban. Ez az érték tartalmazza a teljes képméretet, beleértve a fejlécet is.
    
    imageWidth%256, imageWidth/256: A kép szélességének bájtban kifejezett értéke
    imageHeight%256, imageHeight/256: A kép magasságának bájtban kifejezett értéke
    
    0x00, 0xff, 0xff, 0xff: A hatter szine.
    0xff, 0x00, 0x00, 0xff: A graf szine.
    */

    unsigned char bmpHeader[] = {0x42, 0x4d, imageSize, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x3e, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, imageWidth%256, imageWidth/256,
                                0x00, 0x00, imageHeight%256, imageHeight/256, 0x00, 0x00, 1, 0x00, 1, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x0f,
                                0x00, 0x00, 0x61, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 
                                0x00, 0xff, 0xff, 0xff, 
                                0xff, 0x00, 0x00, 0xff};
    
    // Minden egyes iterációjában egy bájt értékét írja ki a fájlba.
    for (int i = 0; i < BMP_HEADER_SIZE; i++)
    {
        /*
        A write függvény írja ki a bmpHeader tömb aktuális i indexén található bájtot a fájlba.
        
        A függvény a fileHandle file leíróval hivatkozott fájlba írja ki az adott bájtot.
        A második paraméter a bmpHeader tömb aktuális i indexű elemére mutató pointer,
        a harmadik paraméter pedig a bájt méretét adja meg,amely egy bájt az unsigned char adattípusban. 
        */

        write(fileHandle, &bmpHeader[i], sizeof(unsigned char));
    }

    int line = NumValues;
    int colum = NumValues;

    // Ha nem oszthato, akkor a line es colum valtozokat a legkozelebbi 32-vel oszthato ertekre noveli
    // Ez biztositja, hogy a kep sorai 32 bajt hosszusaguak legyenek, ami a hatekony feldolgozashoz szukseges
    if (NumValues % 32 != 0)
    {
        line = imageWidth + (32 - (imageWidth % 32));
        colum = imageHeight + (32 - (imageHeight % 32));
    }

    // Ez a sor kiszamitja a kep pixeladatainak meretet bajtban (minden pixel 1 bit informaciot tartalmaz)
    int pixelArraySize = (line * colum) / 8;

    // Ez a sor lefoglal egy 'pixels' nevu tombot 'pixelArraySize' meretben, és minden elemet 0-ra inicializal
    // A 'calloc' fuggvenyt hasznaljuk a memoria lefoglalásara
    unsigned char *pixels = calloc(pixelArraySize, sizeof(unsigned char));

    int pixelMemoryLocation;

    // Ez a ciklus a kép magasságán (y koordináta) iterál végig.
    for (int y = 0; y < imageHeight; y++)
    {
        // Ez a belso ciklus bejarja a kep szelesseget (x koordinata)
        for (int x = 0; x < imageWidth; x++)
        {
            // Ez a sor kiszamitja a pixel memoriacimet a 'pixels' tombben az 'x' es 'y' koordinatak alapjan
            pixelMemoryLocation = (y * line + x) / 8;

            /*
            Ez ellenorzi, hogy a sor megegyezik-e az adatok tömbének x. eleme es a kep magassag felenek osszegevel

            Ha ez a feltétel igaz, akkor az adott pixel értékét beállítjuk a pixels tömbben.

            A beállítás úgy történik, hogy a pixels tömb megfelelő pozíciójához hozzáadjuk az 1-et a megfelelő helyen.
            A pixelMemoryLocation változó tárolja a pixels tömbben a megfelelő pixel memóriacímét. A pixels tömb egyedi bites reprezentációt használ,
            és az x koordinátától függően beállítja a megfelelő helyi értéket a memóriában. A x koordináta bináris kifejezése (7 - (x % 8)) meghatározza,
            hogy hol található az adott pixel a byte-ban, és az 1 << (7 - (x % 8)) kifejezés segítségével állítjuk be a megfelelő bitet.
            A bitmanipulációs műveletekkel csak az adott pixel értékét módosítjuk a tömbben, anélkül, hogy érintenénk a többi pixelt.
            */

            if (y == Values[x] + imageHeight / 2)
            {
                // Ez a sor beallitja a megfelelo pixel erteket a 'pixels' tombben
                pixels[pixelMemoryLocation] += (1 << (7 - (x % 8)));
            }
        }
    }

    // Pixeladatok irasa a fajlba
    write(fileHandle, pixels, pixelArraySize);

    // Dinamikusan foglalt memoria felszabaditasa
    free(pixels);

    // Fajl bezarasa
    close(fileHandle);                   
}

/*
4. feladat
Írj egy függvényt, amely a Linux fájlrendszer gyökerében lévő ”/proc” könyvtárnak az
alkönyvtáraiban található ”status” nevű fájloknak a tartalmát vizsgálja meg. (Azok az alkönyvtárak
tartalmaznak ilyen fájlokat, amelyek neve számjegy karakterrel kezdődik.) A fájl első sorának a
formátuma: ”Name:\t%s\n”. Ha a tabulátor és az újsor karakter között a ”bash”
karaktersorozatot található, akkor keressen az adott fájlban olyan sort, amely a ”Pid:\t” sztriggel
kezdődik majd ezt egy egész szám követi. A függvény térjen vissza ezzel az egész számmal, ha pedig
egyáltalán nem talál ilyen fájlt egyik alkönyvtárban sem, akkora -1 értékkel! (Persze a megnyitott
fájlokat és könyvtárakat zárja be!) A függvény fejléce legyen a következő:
int FindPID();
*/

int FindPID()
{
    char path_buffer[300];  // Eleresi ut tarolasara szolgal
    char name_buffer[256];  // Process name
    int own_pid = getpid(); // Sajat pid lekerese
    int pid = -1;           // Keresett pid (-1el inicializalom, ha nem lenne meg ezzel ter vissza)
    int tmp = -1;           // Ideiglenes valtozo a PID tarolasara
    FILE *f;                // Fajl mutato a fileok kezelesere
    DIR *d;                 // Konyvtar mutato a konyvtarak kezelesere
    struct dirent *entry;   // Konyvtarbejegyzes struktura

    // Belep a /proc mappaba
    d = opendir("/proc");
    // Ez a ciklus addig fut, amíg a readdir függvény vissza nem tér NULL értékkel, ami azt jelzi, hogy a következő bejegyzés nincs.
    while ((entry = readdir(d)) != NULL)
    {
        // Ellenőrzi, hogy a bejegyzés neve csak szám-e. A isdigit függvény segítségével megvizsgálja, hogy az első karakter egy szám-e.
        if (isdigit((*entry).d_name[0]))
        {
            // Ez a sor egy karakterláncot készít, amely az adott folyamat állapotfájljához vezető elérési utat tartalmazza. 
            snprintf(path_buffer, sizeof(path_buffer), "/proc/%s/status", (*entry).d_name);

            // Ez a sor megnyitja az előző lépésben összeállított elérési útvonalon található állapotfájlt olvasásra. 
            // A fopen függvény visszatér egy fájlmunka mutatóval, amelyet később használnak a fájl olvasására.
            f = fopen(path_buffer, "r");

            // Ez a sor beolvassa a folyamat nevét a status fájlból és eltárolja azt a name_buffer tömbben.
            // A fscanf függvény az adott formátum szerint olvassa be az adatokat a fájlból.
            fscanf(f, "Name:\t%s\n", name_buffer);
            
            // Ha a processz neve megegyezik a keresett nevevel ("chart")
            if (strcmp(name_buffer, "chart") == 0) // process nev = a keresett nevvel
            {
                // Ez a sor elolvassa és eldob minden sort a fájlból, amíg el nem éri a sort, ami a "Pid:" sort tartalmazza.
                // Ezáltal ugratja a felesleges sorokat.

                fscanf(f, "%*[^\n]\n"); // elspkippel 4 sort + 1 mert az elsot mar elhagytuk
                fscanf(f, "%*[^\n]\n"); // igy a Pid-es sorra er
                fscanf(f, "%*[^\n]\n"); // %* --> nem menti el
                fscanf(f, "%*[^\n]\n"); // [^\n]\n --> minden ami nem \n + a vegen \n
                fscanf(f, "Pid:\t%d\n", &tmp);

                // Ha a PID nem azonos a sajat PID-vel, akkor eltaroljuk
                if (tmp != own_pid)
                {
                    pid = tmp;
                }
            }

            // Fajl bezarasa
            fclose(f);
        }
    }

    // Konyvtar bezarasa
    closedir(d);

    // Visszaadjuk a keresett PID-t
    return pid;
}

/*
5. feladat
1. lépés
A FindPID nevű processzus azonosítót kereső függvényt módosítsd úgy, hogy ne a ”bash” nevet,
hanem a ”chart” nevet figyelje/keresse, valamint hagyja figyelmen kívül azt az esetet, amikor egy
”status” fájlban megtalált érték megegyezik a program saját PID-jével!
2. lépés
Hozz létre egy eljárást, az alábbi paraméterezéssel:
void SendViaFile(int *Values, int NumValues);
A Values mutató egy egészeket tartalmazó tömb kezdőcímét kapja paraméterátadás során, míg a
NumValues változó fogja tárolni a tömbben lévő egészek darabszámát. Az eljárás hozzon létre egy
”Measurement.txt” nevű szöveges fájlt az adott felhasználó saját alapértelmezett könyvtárában
és soronként írja bele a tömbben lévő értékeket a fájlba (”%d\n” formátummal)! Miután bezárta a
fájlt, hívja meg az eljárás a FindPID nevű függvényt! Ha a függvény -1 értékkel tér vissza, akkor a
program írjon ki egy hibaüzenetet, arra vonatkozóan, hogy nem talál fogadó üzemmódban működő
folyamatot (process-t) majd a program álljon le egy hibakóddal (nem 0 visszatérési értékkel)!
Amennyiben a FindPID más értékkel tér vissza, az eljárás küldjön neki egy felhasználói 1-es szignált
(SIGUSR1)!
3. lépés
Írj egy eljárást, amely az adott felhasználó alapértelmezett könyvtárában lévő
”Measurement.txt” nevű szöveges fájlt megnyitja, a tartalmát beolvassa és eltárolja egy
memóriaterületre, melyet szükség esetén dinamikus memóriafoglalással bővít (mivel nem tudjukelőre az elemek számát). Ezt követően az eljárás hívja meg a BMPcreator eljárást és adja át neki a
beolvasott értékeket és azok darabszámát! Végül a dinamikusan lefoglalt memóriaterület legyen
felszabadítva! Az eljárás fejléce, legyen az alábbi (a paraméter értékét nem fogjuk felhasználni, de
szükséges):
void ReceiveViaFile(int sig);
4. lépés
Módosítsd a főprogramot úgy, hogy a küldő üzemmódban futó és fájlon keresztüli kommunikációt
folytató folyamatban hívd meg a Measurement függvényt, majd az általa előállított adatokat add át
paraméterként a SendViaFile eljárásnak, végül szabadítsd fel azt a dinamikusan foglalt területet,
ahol a „mérési” adatok tárolva vannak! (Ezt követően a program álljon le!)
Módosítsd továbbá a főprogramot úgy, hogy a fogadó üzemmódban futó és fájlon keresztüli
kommunikációt folytató folyamatban készülj fel felhasználói 1-es szignál érkezésére, ilyen esetben a
ReceiveViaFile eljárás kerüljön automatikusan meghívásra. Addig is a folyamat kerüljön
(szignálra) várakozó állapotba egy végtelen ciklusban, azaz szignál érkezése után sem álljon le a
program, hanem a szigálkezelő lefutása után várjon újabb szignálra!
*/

void SendViaFile(int *Values, int NumValues)
{
    char *filename = "/Measurement.txt"; // Fájlnev inicializalasa
    char *home_dir = getenv("HOME"); // Felhasznaloi konyvtar eleresi utvonalanak lekerdezese
    char *filepath = malloc(strlen(home_dir) + strlen(filename) + 1); // Teljes eleresi utvonal lefoglalasa
    puts("Sending file...");

    strncpy(filepath, home_dir, strlen(home_dir) + 1); // Felhasznaloi konyvtar eleresi utvonalanak masolasa
    strncat(filepath, filename, strlen(filename) + 1); // Fajlnev hozzaadasa az eleresi utvonalhoz

    FILE *myFile = fopen(filepath, "w"); // Fajl megnyitasa irasra
    for (int i = 0; i < NumValues; i++) // Egy ciklusban végigiterálunk az Values tömbön, és minden elemét kiírjuk a megnyitott fájlba
    {
        fprintf(myFile,"%d\n",*(Values + i)); // Meresi eredmenyek irasa a fajlba
    }
    fclose(myFile); // Fajl bezarasa
    free(filepath); // Eleresi utvonal felszabaditasa

    int pid = FindPID(); // Fogado folyamat PID-jenek lekerese
    printf("PID found: %d\n", pid);  // Fogado folyamat PID-jének kiirasa
    if(pid == -1)
    {
        fprintf(stderr, "Could not find a process working in receiving mode!");  // Kiirja a hibat, ha nem talalhato fogado folyamat
        exit(6); // Kilep a programbol hibakoddal
    }
    else
    {
        puts("Reciever found"); // Kiirja, hogy megtalalta a fogado folyamatot
    }
}

void ReceiveViaFile(int sig)
{
    printf("PID: %d", getpid()); // Kiirja a sajat PID-et
    puts("Receiving file");

    char *filename = "/Measurement.txt"; // Fajlnev inicializalása
    char *home_dir = getenv("HOME"); // Felhasznaloi konyvtar eleresi utvonalanak lekerdezese
    char *filepath = malloc(strlen(home_dir) + strlen(filename) + 1); // Teljes eleresi utvonal lefoglalasa

    strncpy(filepath, home_dir, strlen(home_dir) + 1); // Felhasznaloi konyvtar eleresi utvonalanak masolasa
    strncat(filepath, filename, strlen(filename) + 1); // Fajlnev hozzaadasa az eleresi utvonalhoz

    puts("Waiting for file...");

    // Amig a fajl nem letezik, addig varakozik (3 masodpercenkent ellenoriz)
    while (access(filepath, F_OK) == -1)
    {
        sleep(3);
    }

    FILE *file = fopen(filepath, "r"); // Fajl megnyitasa olvasasra
    char line[100]; // Sor beolvasasara szolgalo buffer inicializalasa
    int x = 0; //X szamolja a measurement sorait

    int *result = (int *)malloc(sizeof(int)); // Meresi eredmenyek tombjenek inicializalasa

    /*
    Fajlbol beolvas minden sort, es atkonvertalja egesz szamma

    A beolvasott sor számmá alakítása és hozzáadása a result tömbhöz. Az atoi függvény (ASCII to Integer) átalakítja a sztringet egész számmá.
    Az x az aktuális index a result tömbben.

    x++: Az x változót növeli, hogy az új adatokat a következő indexre helyezze a result tömbben.

    result = (int *)realloc(result, (sizeof(int) * (x + 1))): Dinamikusan újraallokálja a result tömböt, hogy elférjen az új adatokkal.
    Az újraallokálás az x + 1 méretűre történik, mivel az x az aktuális index, ami egy nagyobb méretű tömböt eredményez.
    Az x + 1 annak a számnak az eggyel nagyobb méretét jelenti, amely megfelel a jelenlegi indexnek plusz egy új elemnek.
    Az x a tömbben tárolt elemek számát jelzi. Az realloc a memóriaterületet újraallokálja az új méretűre, és a meglévő adatokat átmásolja az új területre.
    Az realloc-nak ezután vissza kell térnie a result új címével, amit ebben az esetben a result változónak kell újra hozzárendelni.
    */
    while (fgets(line, sizeof(int), file) != NULL) // addig olvas meg a vegere nem er (NULL)
    {
        *(result + x) = atoi(line);
        x++;
        result = (int *)realloc(result, (sizeof(int) * (x + 1)));
    }
    puts("Generating BMP...");
    BMPcreator(result, x); // BMP letrehozasa a meresi eredmenyek alapjan
    puts("Generation successful!");

    remove(filepath); // Fajl torlese

    free(result); // Tomb felszabaditasa
    puts("");
}

/*
6. feladat
1. lépés
Írj egy eljárást, amely UDP protokoll segítségével a localhost (IPv4 cím: 127.0.0.1) 3333-as
portját figyelő fogadó üzemmódú szoftverrel kommunikál. Az eljárás fejléce így nézzen ki:
void SendViaSocket(int *Values, int NumValues);
Itt az első paraméterként kapott memóriacím egy tömb kezdőcíme, a második pedig a tömbben lévő
egész típusú értékek száma. Az eljárás a socketen keresztül küldje el a fogadónak a NumValues
változó értékét (32 bites fix pontos egészként)! Ezután várjon a szerver válaszára, ami szintén
egyetlen 4 bájtos egész szám lesz (int). Ha a küldött és a kapott értékek eltérőek, akkor egy
hibaüzenet után a program egy eddigiektől eltérő hibakóddal álljon le! Ha az értékek megegyeznek,
akkor az eljárás a Values címen kezdődő tömb NumValues darab int típusú értékét küldje át
egyetlen üzenetben a fogadónak. Ezután ismét várjon egy válaszüzenetre, ami egy 4 bájtos egész
szám lesz. Ha a küldött adatok bájtban megadott mérete és a most kapott értékek eltérőek, akkor is
egy hibaüzenet után hibakóddal álljon le a program!
2. lépés
Írj egy másik socket programozáson alapuló eljárást is, amely egy végtelen ciklusban UDP
szegmenseket vár a 3333-as porton. Az első kapott szegmensben feltételezzük, hogy mindig egy 4
bájtos hasznos tartalom van (egy int változó értéke). Az eljárás nyugtaként küldje vissza a kapott
értéket a küldő üzemmódú kliens folyamatnak, valamint dinamikusan foglaljon le ennyi darab egész
szám számára egy folyamatos memóriaterületet! Ide tárolja el a második üzenetben kapott adatokat,
amelyeknek a bájtban megadott méretét nyugtaként juttassa vissza a küldőnek! A kapott adatokkal
kerüljön meghívásra a BMPcreator eljárás, végül a lefoglalt memóriaterületek legyenek
felszabadítva és az eljárás várjon újabb üzenetet egy küldőtől! (Feltehetjük, hogy egy adott
pillanatban csak egyetlen küldő aktív és az mindig elküldi mind a két üzenetet.). Az eljárás fejléce
egyszerűen legyen ez:
void ReceiveViaSocket();
3. lépés
Módosítsd a főprogramot úgy, hogy a küldő üzemmódban futó és socketen keresztüli kommunikációt
folytató folyamatban hívd meg a Measurement függvényt, majd az általa előállított adatokat add átparaméterként a SendViaSocket eljárásnak, végül szabadítsd fel azt a dinamikusan foglalt
területet, ahol a mérési adatok tárolva vannak! (Ezt követően a program álljon le!)
Módosítsd továbbá a főprogramot úgy, hogy a fogadó üzemmódban futó és socketen keresztüli
kommunikációt folytató folyamatban a ReceiveViaSocket eljárás kerüljön meghívásra.
*/

void SendViaSocket(int *Values, int NumValues)
{
    // Deklaraciok
    int s;                      // Socket azonositoja
    int bytes;                  // Kuldott bajtok szama
    int flag;                   // Atviteli zaszlo
    char on;                    // Socket beallitasok
    unsigned int server_size;   // Szerver sockaddr_in hossza
    struct sockaddr_in server;  // Szerver cime
    int response;               // Szerver valasza

    // Inicializalas
    on   = 1;                                           // Beallitasok inicializalasa
    flag = 0;                                           // Atviteli zaszlo inicializalasa
    server.sin_family      = AF_INET;                   // Beállítja a szerver címének típusát, IPv4 protokoll hasznalata
    server.sin_addr.s_addr = inet_addr("127.0.0.1");    // Szerver IP-cimenek beallitasa
                                                        //Ez a függvény az IPv4 címet átalakítja hálózati bájtsorozattá.
                                                        //Az "127.0.0.1" az IPv4 címet jelöli, amely azonos gépre hivatkozik (a lokális gépre).
    server.sin_port        = htons(3333);               // Port beállitasa, Ez a függvény a portszámot átalakítja hálózati bájtsorozattá
    server_size = sizeof server;                        // Szerver sockaddr_in meretenek kiszamitasa

    // Socket letrehozasa
    s = socket(AF_INET, SOCK_DGRAM, 0 ); // socket(címcsalád, típus, használt protokollt)
    // Ha 0, akkor a rendszer automatikusan kiválasztja a megfelelő protokollt a megadott címcsalád és socket típus alapján
    if (s < 0)
    {
        fprintf(stderr, "The socket could not be created!\n");
        exit(2);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on); // Socket beallitasokat modositja (a socket opciószintet határozza meg,
    //ocket opció, amely lehetővé teszi, hogy ugyanazon a címen más socket is fogadja a bejövő csatlakozásokat, n változó címét adja meg,
    //ami a beállítás engedélyezését vagy letiltását jelzi)

    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on); // Socket beallitasok, SO_KEEPALIVE opciót, ami azt jelzi a rendszernek,
    //hogy automatikusan ellenőrizze és fenntartsa az összeköttetést az inaktív állapotban lévő socketeken keresztül is. Ez segíthet abban,
    //hogy a rendszer időben észlelje a hálózati problémákat és megszakadt kapcsolatokat.

    // UDP csomagot küld a sendto függvény segítségével
    bytes = sendto(s, &NumValues, sizeof(int), flag, (struct sockaddr *) &server, server_size);
    /*
    bytes: tárolja a küldött bájtok számát.
    s: Ez a socket azonosítója, amelyen keresztül a csomagot küldjük.
    &NumValues: Az adatokat tartalmazó változó címe, amelyet elküldünk.
    sizeof(int): A küldött adatok mérete bájtban.
    flag: A küldési művelet opcióit jelző zászlók.
    server_size: A server struktúra mérete bájtban.
    */
    puts("Sending array size...");
    if (bytes <= 0)
    {
        fprintf(stderr, "Sending failed!\n");
        exit(5);
    }
    signal(SIGALRM,SignalHandler);
    alarm(1);
    /*"Ha az eljaras alarm szignalt (SIGALRM) kap, akkor irjon ki egy hibauzenetet arra vonatkozoan, hogy a
    szerver nem valaszol (idokereten belul), majd a program egy hibakoddal alljon le"*/

    //Adatok fogadasa, ellenorzese
    bytes = recvfrom(s, &response, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
    /*
    bytes: Ez a változó tárolja a fogadott bájtok számát.
    recvfrom: Ez a függvény az adatok fogadására szolgál egy adott socketen keresztül.
    s: Ez a socket azonosítója, amelyen keresztül a csomagot fogadjuk.
    &response: A fogadott adatokat tartalmazó változó címe.
    sizeof(int): A fogadott adatok mérete bájtban.
    flag: A fogadási művelet opcióit jelző zászlók.
    *(struct sockaddr ) &server: A forrásadatcím struktúrájának címe, ahonnan a csomagot fogadjuk.
    A server struktúra tartalmazza a forrás IP-címet és portszámot.
    &server_size: A server struktúra mérete bájtban, amelyet a függvény kitölt a fogadáskor a forrás címmel.
    */
    puts("Checking reserved size......");
    if (bytes < 0)
    {
        fprintf(stderr, "The receiving failed!\n");
        exit(6);
    }

    alarm(0); // Idokeret megszuntetese
    
    //Meret teszt
    if (NumValues != response)
    {
        fprintf(stderr, "Size error!\n");
        exit(7);
    }
    puts("The sent and received size are the same.");

    //Tombadatok kuldese
    bytes = sendto(s, Values, (NumValues * sizeof(int)), flag, (struct sockaddr *) &server, server_size);
    /*
    bytes: Ez a változó tárolja a kiküldött bájtok számát.
    sendto: Ez a függvény az adatok küldésére szolgál egy adott socketen keresztül.
    s: Ez a socket azonosítója.
    Values: A küldendő adatokat tartalmazó tömb.
    (NumValues * sizeof(int)): A küldendő adatok mérete bájtban.
    flag: Az adatküldési művelet opcióit jelző zászlók.
    *(struct sockaddr ) &server: A célállomás címének struktúrájának címe, ahova a csomagot küldjük.
    server_size: A server struktúra mérete bájtban.
    */
    if ( bytes <= 0 )
    {
        fprintf(stderr, "Sending failed!\n");
        exit(5);
    }
    puts("Sending array data...");

    //Kuldott fogadasi bajtok merete
    int s_bytes = recvfrom(s, &response, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
    /*
    s_bytes: Ez a változó tárolja a fogadott bájtok számát.
    recvfrom: Ez a függvény a beérkező adatok fogadására szolgál egy adott socketen keresztül.
    s: Ez a socket azonosítója, amelyen keresztül a csomagot fogadjuk.
    &response: Ebben a változóban tároljuk majd az fogadott adatokat.
    sizeof(int): A fogadott adatok mérete bájtban.
    flag: A fogadási művelet opcióit jelző zászlók.
    *(struct sockaddr ) &server: Az üzenet forrásának címének struktúrájának címe, ahonnan az üzenet érkezett.
    &server_size: A server struktúra mérete bájtban, amelyet a függvény kitölt a fogadás során az üzenet forrásának címével.
    */
    if (bytes <= 0)
    {
        fprintf(stderr,"Size error\n");
        exit(7);
    }
    if (response != bytes)
    {
        fprintf(stderr, "Size error!\n");
        exit(7);
    }
    puts("Checking the sent and received bytes...");
    puts("File sent successfully.");
    
    close(s); //Socket bezarasa 
}

void ReceiveViaSocket()
{
    // Deklaraciok
    int bytes;                  // Fogadott bajtok szama
    int err;                    // Hiba kodja
    int flag;                   // Atviteli zaszlo
    char on;                    // Socket beallitasok
    unsigned int server_size;   // Szerver sockaddr_in hossza
    unsigned int client_size;   // Ugyfel sockaddr_in hossza
    struct sockaddr_in server;  // Szerver cime
    struct sockaddr_in client;  // Ugyfel cime
    int response;

    //Inicializalas
    on   = 1;                               // Beallitasok inicializalasa
    flag = 0;                               // Atviteli zaszlo inicializalasa
    server.sin_family      = AF_INET;       // IPv4 protokoll hasznalata
    server.sin_addr.s_addr = INADDR_ANY;    // Barmely interfesz
    server.sin_port        = htons(3333);   // Port beallitasa
    server_size = sizeof server;            // Szerver sockaddr_in meretenek kiszamitasa
    client_size = sizeof client;            // Ugyfel sockaddr_in meretenek kiszamitasa

    //Socket letrehozasa
    int s = socket(AF_INET, SOCK_DGRAM, 0 );
    if (s < 0)
    {
        fprintf(stderr, "Error while creating socket!\n");
        exit(2);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /*
    Ez a rész a socketet a szerver címéhez és portjához köti.

    err: Ez a változó tárolja a bind függvény hívásának eredményét, azaz hogy sikeres volt-e vagy sem.
    s: A socket azonosítója, amelyet korábban létrehoztunk.
    server: A sockaddr_in típusú struktúra, amely a szerver címét és portját tárolja.
    server_size: A server struktúra mérete bájtban.

    Az err változóban tárolt érték a bind hívás sikerességét jelzi.
    Ha az érték kisebb, mint 0, akkor hiba történt a socket kötése közben.
    */

    err = bind(s, (struct sockaddr *) &server, server_size);
    if (err < 0)
    {
        fprintf(stderr,"Socket connection error!\n");
        exit(3);
    }

    //Adatok fogadasa
    while(1) //Folyamatos szerver mukodes
    { 
        puts("Waiting for data...");

        //Fogadasi adatok merete
        bytes = recvfrom(s, &response, sizeof(int), flag, (struct sockaddr *) &client, &client_size );
        if (bytes < 0)
        {
            fprintf(stderr, "Receiving error!\n");
            exit(6);
        }

        // A data tömb lefoglalása a fogadott adatok tárolására szolgál.
        int* data = malloc(response * sizeof(int));
        puts("Allocating the size of the array...");

        //Tombkiosztasi meret elkuldese
        bytes = sendto(s, &response, sizeof(int), flag, (struct sockaddr *) &client, client_size);
        puts("Checking the size of an allocated array...");
        if (bytes <= 0)
        {
            fprintf(stderr, "Sending error!\n");
            exit(5);
        }

        //Tombadatok fogadasa
        bytes = recvfrom(s, data, response * sizeof(int), flag, (struct sockaddr *) &client, &client_size );
        puts("Receiving array data....");
        if (bytes < 0)
        {
            fprintf(stderr, "Receiving error!\n");
            exit(6);
        }

        //Fogadasi bajtok kuldese
        bytes = sendto(s, &bytes, sizeof(int), flag, (struct sockaddr *) &client, client_size);
        puts("Checking the bytes of received data...");
        if (bytes < 0)
        {
            fprintf(stderr, "Sending error!\n");
            exit(5);
        }

        //BMP fajl keszites
        puts("Generating BMP...");
        BMPcreator(data, response);
        puts("Generating done!");

        free(data);
        puts("");
    }

    close(s);   
}

/*
----------------------------------------------------------------------------------------------------------

A snprintf függvény szövegformázást végez, és a formázott szöveget egy célbufferbe írja.
Fontos, hogy a célbuffernek kell lennie, ahova a formázott szöveget szeretnénk írni.
A függvény aláírása a következő:

int snprintf(char *str, size_t size, const char *format, ...);

str: Célbuffer, amelybe a formázott szöveg íródik.
size: A címben meghatározott célbuffer mérete.
format: A formátumstring, amelyet a printf formátumában kell megadni.
...: Az opcionális argumentumok, amelyek a formázásban használt értékeket adják meg.

----------------------------------------------------------------------------------------------------------

Az fscanf függvény szövegbeolvassást végez egy formázott bemeneti szövegből.
A formázott szöveget az általános bemeneti fájlból olvassa be.
A függvény aláírása a következő:

int fscanf(FILE *stream, const char *format, ...);

stream: Az input fájl mutatója.
format: A formátumstring, amelyet a printf formátumában kell megadni.
...: Az opcionális argumentumok, amelyekbe az olvasott értékek kerülnek.

----------------------------------------------------------------------------------------------------------

A strncpy függvény karakterláncok másolására szolgál. A működése nagyon hasonló a strcpy függvényéhez,
azonban a strncpy lehetővé teszi az első n karakter másolását egy forrásláncból egy céláncba.
Itt van egy általános alakja:

char *strncpy(char *destination, const char *source, size_t num);

destination: A cél karaktertömb, amelybe másolni szeretnénk a karaktereket.
source: A forráslánc, amelyből másolni szeretnénk a karaktereket.
num: Az a maximális számú karakter, amelyet másolni szeretnénk a forrásláncból a céláncba.

----------------------------------------------------------------------------------------------------------

A strncat függvény karakterláncok összefűzésére szolgál.
Az összefűzés során a forrás karakterláncot hozzáfűzi a cél karakterlánc végéhez.
Itt van az általános alakja:

char *strncat(char *destination, const char *source, size_t num);

destination: A cél karaktertömb, amelyhez hozzáfűzzük a forrásláncot.
source: A forráslánc, amelyet hozzáfűzünk a célánc végéhez.
num: Az a maximális számú karakter, amelyet hozzáfűzünk a forrásláncból a célánc végéhez.

----------------------------------------------------------------------------------------------------------

Az access függvény az adott fájl vagy mappa hozzáférhetőségét ellenőrzi az adott elérési útvonalon.
A függvény szintaxisa a következő:

int access(const char *path, int mode);

A path paraméter az ellenőrizni kívánt fájl vagy mappa elérési útvonala, míg a mode paraméter az ellenőrzés típusát határozza meg. A mode lehet az alábbiak közül:

F_OK: Ellenőrzi, hogy a fájl létezik-e.
R_OK: Ellenőrzi, hogy a fájl olvasható-e.
W_OK: Ellenőrzi, hogy a fájl írható-e.
X_OK: Ellenőrzi, hogy a fájl futtatható-e.

Az access függvény visszatérési értéke -1 (vagy false), ha az ellenőrzött feltétel nem teljesül, vagy 0 (vagy true), ha igen.
*/