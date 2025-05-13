#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>


# include "BER_data.hh"

void print_table(int N, int BER, char *ber);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <N> <BER>\n", argv[0]);
        printf("N (Attacker) <= 128\n");
        printf("BER(%%)       : 0.01, 0.1, 1, 2, 4, 10\n");

        exit(EXIT_FAILURE);
    }

    int N = atoi(argv[1]); 


    if (N < 1 || N > MAX_N) {
        printf("N: Enter a value between 1 and %d: ", MAX_N);
        exit(EXIT_FAILURE);
    }

    int BER;
    if (strcmp(argv[2], "0.01") == 0) {
        BER = 0;
    }
    else if (strcmp(argv[2], "0.1") == 0) {
        BER = 1;
    }
    else if (strcmp(argv[2], "1") == 0) {
        BER = 2;
    }
    else if (strcmp(argv[2], "2") == 0) {
        BER = 3;
    }
    else if (strcmp(argv[2], "4") == 0) {
        BER = 4;
    }
    else if (strcmp(argv[2], "10") == 0) {
        BER = 5;
    }
    else {
        printf("Unknown BER: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }


    caculate_ber_data();
    print_table(N, BER, argv[2]);


    return 0;
}

void print_table(int N, int BER, char *ber) {



    printf("\nN   (Attacker)          : %d\n", N);
    printf("BER (Bir Error Rate)    : %s%%\n", ber);

    printf("================================================================================================================================================================================\n");
    printf("                                                                           Target CASE                                                                                          \n");
    printf("================================================================================================================================================================================\n");

    printf("%-6s | %-60s | %-102s |\n", "", "No Obfuscation", "Obfuscation");
    printf("-------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|\n");
    printf("%-6s | %-18s | %-18s | %-18s | %-18s | %-18s | %-18s | %-18s | %-18s |\n",
           "N", "Chipkill", "PARA", "SRS", "Cube",
           "Cube_para", "Cube_srs", "QOC", "OOC");

    printf("-------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|\n");

    // Target CASE
    for (int i = 1; i <= N; i++) {
        printf("%-6d | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e |\n",
               i,
               target_attack[i].chipkill_ber[BER],
               target_attack[i].para_ber[BER],
               target_attack[i].srs_ber[BER],
               target_attack[i].cube_ber[BER],
               target_attack[i].cube_para_ber[BER],
               target_attack[i].cube_srs_ber[BER],
               target_attack[i].qoc_ber[BER],
               target_attack[i].ooc_ber[BER]);
    }
    printf("================================================================================================================================================================================\n");

    printf("\n\n================================================================================================================================================================================\n");
    printf("                                                                           DoS CASE                                                                                             \n");
    printf("================================================================================================================================================================================\n");


    printf("%-6s | %-60s | %-102s |\n", "", "No Obfuscation", "Obfuscation");
    printf("-------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|\n");
    printf("%-6s | %-18s | %-18s | %-18s | %-18s | %-18s | %-18s | %-18s | %-18s |\n",
           "N", "Chipkill", "PARA", "SRS", "Cube",
           "Cube_para", "Cube_srs", "QOC", "OOC");
    printf("-------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|--------------------|\n");

    // DoS CASE 
    for (int i = 1; i <= N; i++) {
        printf("%-6d | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e | %-18.2e |\n",
               i,
               DoS_attack[i].chipkill_ber[BER],
               DoS_attack[i].para_ber[BER],
               DoS_attack[i].srs_ber[BER],
               DoS_attack[i].cube_ber[BER],
               DoS_attack[i].cube_para_ber[BER],
               DoS_attack[i].cube_srs_ber[BER],
               DoS_attack[i].qoc_ber[BER],
               DoS_attack[i].ooc_ber[BER]);
    }
    printf("================================================================================================================================================================================\n");
}
