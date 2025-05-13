#ifndef __BER_HH__
#define __BER_HH__

#define MAX_N 128           

typedef struct {
    // No obfuscation
    double chipkill_ber[6]; 
    double para_ber[6];  
    double srs_ber[6];  

    // obfuscation
    double cube_ber[6];      
    double cube_para_ber[6];      
    double cube_srs_ber[6];      
    double qoc_ber[6];      
    double ooc_ber[6]; 

} BER_Data;


extern BER_Data DoS_attack[MAX_N];
extern BER_Data target_attack[MAX_N];

void caculate_ber_data();
double binom_dist_range(int n, double p, int lower, int upper);
long long comb(int n, int k);
long long perm(int n, int k);
double calculate_probability(int error_n, int chip, int K);


#endif