#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "RKP_header_file.h"

int main(int argc, char *argv[])
{
    // Programnev ellenorzese
    if (strcmp(argv[0], "./chart") != 0)
    {
        fprintf(stderr, "The name of the program is not correct! (chart)\n");
        exit(EXIT_FAILURE);
    }

    // Valtozok inicializalasa
    int file_mode = 0;
    int send_mode = 0;
    int *data = NULL;
    int size;

    // Parancsok feldolgozasa
    Commands(&send_mode, &file_mode, argc, argv);

    // Szignalkezelo beallitasa
    signal(SIGINT,SignalHandler); 
    signal(SIGUSR1,SignalHandler);

    // Hibaellenorzes
    if (file_mode < 0 || send_mode < 0)
    {
        exit(EXIT_FAILURE);
    }

    // Fajlba kuldes modja
    if (file_mode == 0 && send_mode == 0)
    {
        size = Measurement(&data);
        puts("Array generation successful");
        SendViaFile(data, size);
        puts("File sent");
        free(data);
        exit(EXIT_SUCCESS);
    }
    
    // Socketen keresztuli kuldes modja
    else if (file_mode == 1 && send_mode == 0 )
    {
        size = Measurement(&data);
        puts("Array generation successful");
        SendViaSocket(data, size);
        free(data);
        exit(EXIT_SUCCESS);
    }

    // Fajlbol torteno fogadas modja
    else if (file_mode == 0 && send_mode == 1 )
    {
        while (1)
        {
            ReceiveViaFile(signal(SIGUSR1, SignalHandler)); // Szignal beallitasa a fogadashoz
            sleep(2);
        }
        pause(); // Szuneteles
    }

    // Socketen keresztuli fogadas modja
    else if (file_mode == 1 && send_mode == 1)
    {
        ReceiveViaSocket(); // Socketen keresztuli fogadas
    }
    
    return EXIT_SUCCESS;
}