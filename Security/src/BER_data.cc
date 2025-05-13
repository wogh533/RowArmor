#include "BER_data.hh"
#include <stdio.h>
#include <math.h>

BER_Data DoS_attack[MAX_N];
BER_Data target_attack[MAX_N];

// DDR5x4 Configuration (you can change this part if you want)

#define N (1 << 17)  // 2^17
#define R 1


double Padj_cube    = 20.0/N;       // adjacent of target row (para) : 20
double Padj_QOC     = 80.0/N;       // adjacent of target row (QOC)  : 80
double Padj_OOC     = 160.0/N;      // adjacent of target row (OOC)  : 160

double SDC_chipkill = 2.29e-09;                
double SDC_QOC      = 2.33e-09;         
double SDC_OOC      = 5.42e-19;         

double PARA_p       = 1.21e-13;     // 1K (-> 0.1)
double SRS_p        = 9.86e-14;     // K=9
double SRS_dos      = 1.29e-8;     

void caculate_ber_data() {


    double BER_percent[6] = {0.01, 0.1, 1, 2, 4, 10};
    double Symbol_fail_p[6];
    double Symbol_10_fail_p[6];

    for (int i = 0; i < 6; i++) {
        Symbol_fail_p[i] = 1 - pow((100 - BER_percent[i]) / 100.0, 8);
    }  

    for (int i = 0; i < 6; i++) {
        Symbol_10_fail_p[i] = 1 - pow((1 - Symbol_fail_p[i]) , 10);
    }  

    for (int i = 1; i < MAX_N; i++) {

        //Chipkill 
        for (int j = 0; j < 6; j++) {
            target_attack[i].chipkill_ber[j]    = binom_dist_range(i, Symbol_10_fail_p[j], 1, i) * SDC_chipkill;
            DoS_attack[i].chipkill_ber[j]       = binom_dist_range(i, Symbol_10_fail_p[j], 1, i);
        }


        // PARA_CK 
        for (int j = 0; j < 6; j++) {
            if (i==1){
                target_attack[i].para_ber[j]    = binom_dist_range(i, PARA_p, 1, i) * target_attack[i].chipkill_ber[j];
            }
            else{
                target_attack[i].para_ber[j]    = binom_dist_range(2, PARA_p, 1, 2) * target_attack[2].chipkill_ber[j];
            }
            DoS_attack[i].para_ber[j]           = binom_dist_range(i, PARA_p, 1, i) * DoS_attack[i].chipkill_ber[j];
        }


        // SRS 
        for (int j = 0; j < 6; j++) {
            target_attack[i].srs_ber[j]         = binom_dist_range(i, SRS_p, 1, i) * target_attack[i].chipkill_ber[j];
            DoS_attack[i].srs_ber[j]            = binom_dist_range(i, SRS_dos, 1, i) * DoS_attack[i].chipkill_ber[j];
        }


        // Cube
        for (int j = 0; j < 6; j++) {
            if (i == 1){
                target_attack[i].cube_ber[j]    = 0;
                DoS_attack[i].cube_ber[j]       = 0;
            }
            else {
                target_attack[i].cube_ber[j]    = binom_dist_range(i, Padj_cube, 2, i) * binom_dist_range(i, Symbol_fail_p[j], 2, i) * SDC_chipkill;
                DoS_attack[i].cube_ber[j]       = (1-SDC_chipkill) * calculate_probability(2,10,i) * binom_dist_range(i, Symbol_fail_p[j], 2, i);
            }
        }


        // Cube_para
        for (int j = 0; j < 6; j++) {
            if (i == 1){
                target_attack[i].cube_para_ber[j]    = 0;
                DoS_attack[i].cube_para_ber[j]       = 0;
            }
            else {
                target_attack[i].cube_para_ber[j]    = binom_dist_range(2, PARA_p, 1, 2) * target_attack[i].cube_ber[j];
                DoS_attack[i].cube_para_ber[j]       = binom_dist_range(i, PARA_p, 1, i) * DoS_attack[i].cube_ber[j];
            }
        }

        // Cube_srs
        for (int j = 0; j < 6; j++) {
            if (i == 1){
                target_attack[i].cube_srs_ber[j]    = 0;
                DoS_attack[i].cube_srs_ber[j]       = 0;
            }
            else {
                target_attack[i].cube_srs_ber[j]    = binom_dist_range(i, SRS_p, 1, i) * target_attack[i].cube_ber[j];
                DoS_attack[i].cube_srs_ber[j]       = binom_dist_range(i, SRS_dos, 1, i) * DoS_attack[i].cube_ber[j];
            }
        }



        // QOC
        for (int j = 0; j < 6; j++) {
            if (i < 5){
                target_attack[i].qoc_ber[j]    = 0;
                DoS_attack[i].qoc_ber[j]       = 0;
            }
            else {
                target_attack[i].qoc_ber[j]    = binom_dist_range(i, Padj_QOC, 5, i) * binom_dist_range(i, Symbol_fail_p[j], 5, i) * SDC_QOC;
                DoS_attack[i].qoc_ber[j]       = (1-SDC_QOC) * calculate_probability(5,40,i) * binom_dist_range(i, Symbol_fail_p[j], 5, i);
            }
        }


        // OOC
        for (int j = 0; j < 6; j++) {
            if (i < 9){
                target_attack[i].ooc_ber[j]    = 0;
                DoS_attack[i].ooc_ber[j]       = 0;
            }
            else {
                target_attack[i].ooc_ber[j]    = binom_dist_range(i, Padj_OOC, 9, i) * binom_dist_range(i, Symbol_fail_p[j], 9, i) * SDC_OOC;
                DoS_attack[i].ooc_ber[j]       = (1-SDC_OOC) * calculate_probability(9,80,i) * binom_dist_range(i, Symbol_fail_p[j], 9, i);
            }
        }

    }

}


double binom_dist_range(int n, double p, int lower, int upper) {
    double sum_prob = 0.0;
    double log_factorial_n = 0.0;  

    for (int i = 1; i <= n; i++) {
        log_factorial_n += log(i);
    }

    for (int k = lower; k <= upper; k++) {
        double log_factorial_k = 0.0, log_factorial_nk = 0.0;

        for (int i = 1; i <= k; i++) log_factorial_k += log(i);
        for (int i = 1; i <= (n - k); i++) log_factorial_nk += log(i);


        double log_binomial_coeff = log_factorial_n - log_factorial_k - log_factorial_nk;
        double prob = exp(log_binomial_coeff + k * log(p) + (n - k) * log(1 - p));

        sum_prob += prob;
    }

    return sum_prob;
}


long long comb(int n, int k) {
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    long long res = 1;
    for (int i = 0; i < k; i++) {
        res = res * (n - i) / (i + 1);
    }
    return res;
}


long long perm(int n, int k) {
    if (k > n) return 0;
    long long res = 1;
    for (int i = 0; i < k; i++) {
        res *= (n - i);
    }
    return res;
}


double calculate_probability(int error_n, int chip, int K) {
    double prob = 0.0;
    for (int i = error_n; i <= K; i++) {
        if (i == K) {
            prob += (perm(K, i - 1) * pow(R * 2, i) * comb(chip, i)) / pow(N, i - 1);
        } else {
            prob += (perm(i, i - 1) * pow(R * 2, i) * comb(chip, i)) / pow(N, i - 1) * comb(K, i);
        }
    }
    return prob;
}

