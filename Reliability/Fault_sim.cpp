#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctime>
#include<iostream>
#include<cstdlib>
#include <vector>
#include <set>
#include <algorithm>
#include <math.h>
#include <cstring>
#include <fstream>



// Configuration

#define CHANNEL_WIDTH 40
#define CHIP_NUM 10
#define OPC_CHIP_NUM 20
#define DATA_CHIP_NUM 8 // data chip (sub-channel) 
#define CHIP_WIDTH 4
#define BLHEIGHT 16 // Rank-level ECC (RECC)Burst length 

#define OECC_CW_LEN 64 // AMD, QPC OECC codeword length (bit)
#define OPC_OECC_CW_LEN 32 // OPC codeword length (bit)
#define OECC_DATA_LEN 64 // OECC dataward length (bit)
#define OECC_REDUN_LEN 4 // OECC redundancy length (bit)


#define RECC_CW_LEN 80 // RL-ECC codeword length
#define RECC_DATA_LEN 64 // RL-ECC dataword length
#define RECC_REDUN_LEN 16 // RL-ECC redundancy length

#define AMDCHIPKILL_CW_LEN 80 // AMD codeword length (bit)
#define QPC_CW_LEN 320 // QPC codeword length (bit)
#define OPC_CW_LEN 640 // OPC codeword length (bit)

#define SYMBOL_SIZE 8 // RECC ymbol size (GF(2^8)) -> optional use
#define RECC_REDUN_SYMBOL_NUM 8 // rank-level ecc redundancy length (symbol) -> optional use

#define AMDCHIPKILL_CW_SYMBOL_NUM 10 // AMD ecc codeword 길이 (symbol) -> optional use
#define QPC_CW_SYMBOL_NUM 40 // QPC ecc codeword length (symbol) -> optional use
#define OPC_CW_SYMBOL_NUM 80 // OPC ecc codeword length (symbol) -> optional use



#define RUN_NUM 100000000 // iteration


#define CONSERVATIVE_MODE 1 // 1: Conservavie mode, 0: Restrained mode

// -------------------------------
#define mm  8           /* RS code over GF(2**8) - change to suit */
#define nn  255          /* nn=2**mm -1   length of codeword */
#define OPC_tt  8           /* number of errors that can be corrected */
#define OPC_kk  239           /* kk = nn-2*tt  */
#define OPC_nn_short  80      /* length of codeword (shortened) */
#define QPC_tt  4           /* number of errors that can be corrected */
#define QPC_kk  247           /* kk = nn-2*tt  */
#define QPC_nn_short  40      /* length of codeword (shortened) */
//----------------------------------




//configuration over

using namespace std;
unsigned int H_Matrix_OECC[OECC_REDUN_LEN][OECC_CW_LEN]; // 8 x 64
unsigned int H_Matrix_RECC[RECC_REDUN_LEN][RECC_CW_LEN]; // 8 x 40

unsigned int primitive_poly[16][256]={0,}; // primitive polynomial (ex : primitive_poly[4][254] = a^254, primitive_poly[4][255] = 0 (prim_num=4, primitive_poly = x^8+x^6+x^4+x^3+x^2+x^1+1))
enum OECC_TYPE {OECC_OFF=0, OECC_ON=1}; // oecc_type
enum FAULT_TYPE {SBE=0, PIN_1=1, SCE=2, DBE=3, TBE=4, SCE_SBE=5, SCE_DBE=6, SCE_SCE=7, RANK=8};
enum RECC_TYPE {RECC_OFF=0, OPC=1, QPC=2, AMDCHIPKILL=3}; // recc_type
enum RESULT_TYPE {NE=0, CE=1, DUE=2, SDC=3}; // result_type


unsigned int getAbit(unsigned short x, int n) { 
  return (x & (1 << n)) >> n;
}


unsigned int conversion_to_int_format(char *str_read, int m)
{
    unsigned int primitive_value=0;
    if(strstr(str_read,"^7")!=NULL)
        primitive_value+=int(pow(2,7));
    if(strstr(str_read,"^6")!=NULL)
        primitive_value+=int(pow(2,6));
    if(strstr(str_read,"^5")!=NULL)
        primitive_value+=int(pow(2,5));
    if(strstr(str_read,"^4")!=NULL)
        primitive_value+=int(pow(2,4));
    if(strstr(str_read,"^3")!=NULL)
        primitive_value+=int(pow(2,3));
    if(strstr(str_read,"^2")!=NULL)
        primitive_value+=int(pow(2,2));
    if(strstr(str_read,"^1+")!=NULL) 
        primitive_value+=int(pow(2,1));
    if(strstr(str_read,"+1")!=NULL)
        primitive_value+=int(pow(2,0));
    

    return primitive_value;
}

// primitive polynomial table 
void generate_primitive_poly(unsigned int prim_value, int m, int prim_num)
{
    unsigned int value = 0x1; // start value (0000 0001)
    int total_count = int(pow(2,m));
    int count=0;
    while (count<total_count-1){ // count : 0~254
        primitive_poly[prim_num][count]=value;
        if(value>=0x80){ 
            value=value<<(32-m+1);
            value=value>>(32-m);

            value=value^prim_value;
        }
        else 
            value=value<<1;
        
        count++;
    }

    return;
}


void oecc_recc_fault_type_assignment(string &OECC, string &FAULT, string &RECC, int *oecc_type, int*fault_type, int*recc_type, int oecc, int fault, int recc)
{
    // 1. OECC TYPE  (ON/OFF)
    switch (oecc){
        case OECC_OFF:
            OECC.replace(OECC.begin(), OECC.end(),"OECC_OFF");
            *oecc_type=OECC_OFF;
            break;
        case OECC_ON:
            OECC.replace(OECC.begin(), OECC.end(),"OECC_ON");
            *oecc_type=OECC_ON;
            break;
        default:
            break;
    }
    switch (fault){
        case SBE:
            FAULT.replace(FAULT.begin(), FAULT.end(),"SBE");
            *fault_type=SBE;
            break;
        case PIN_1:
            FAULT.replace(FAULT.begin(), FAULT.end(),"PIN_1");
            *fault_type=PIN_1;
            break;
        case SCE:
            FAULT.replace(FAULT.begin(), FAULT.end(),"SCE");
            *fault_type=SCE;
            break;
        case DBE:
            FAULT.replace(FAULT.begin(), FAULT.end(),"DBE");
            *fault_type=DBE;
            break;
        case TBE:
            FAULT.replace(FAULT.begin(), FAULT.end(),"TBE");
            *fault_type=TBE;
            break;
        case SCE_SBE:
            FAULT.replace(FAULT.begin(), FAULT.end(),"SCE_SBE");
            *fault_type=SCE_SBE;
            break;
        case SCE_DBE:
            FAULT.replace(FAULT.begin(), FAULT.end(),"SCE_DBE");
            *fault_type=SCE_DBE;
            break;            
        case SCE_SCE:
            FAULT.replace(FAULT.begin(), FAULT.end(),"SCE_SCE");
            *fault_type=SCE_SCE;
            break;            
        case RANK:
            FAULT.replace(FAULT.begin(), FAULT.end(),"RANK");
            *fault_type=RANK;
            break;                                
        default:
            break;
    }    
    // 2. RECC TYPE  (ON/OFF)
    switch (recc){
        case RECC_OFF: 
            RECC.replace(RECC.begin(), RECC.end(),"RECC_OFF");
            *recc_type=RECC_OFF;
            break;
        case OPC:
            RECC.replace(RECC.begin(), RECC.end(),"OPC");
            *recc_type=OPC;
            break;
        case QPC:
            RECC.replace(RECC.begin(), RECC.end(),"QPC");
            *recc_type=QPC;
            break;
        case AMDCHIPKILL:
            RECC.replace(RECC.begin(), RECC.end(),"AMDCHIPKILL");
            *recc_type=AMDCHIPKILL;          
            break;            
        default:
            break;
    }
    return;
}

unsigned int index_of(unsigned int value){
    unsigned int p;
    if (value == 0){
        return -1;
    }
    for(int prim_exponent=0; prim_exponent<255; prim_exponent++){
        if(value==primitive_poly[0][prim_exponent])
            p=prim_exponent;
        }
    return p;   
}



// SE injection (Single Error injection)
void error_injection_SE(unsigned int Chip_array[][OECC_CW_LEN], int recc_type)
{

    if (recc_type==OPC){
        int Fault_Chip_position = rand() % OPC_CHIP_NUM;
        int Fault_bit_position = rand() % OPC_OECC_CW_LEN;

        Chip_array[Fault_Chip_position][Fault_bit_position]=1;
        return;
    }
    else if(recc_type==QPC){
        int Fault_Chip_position = rand() % CHIP_NUM;
        int Fault_bit_position = rand() % OECC_CW_LEN;

        Chip_array[Fault_Chip_position][Fault_bit_position]=1;
        return;
    }
    else if(recc_type==AMDCHIPKILL){
        int Fault_Chip_position = rand() % CHIP_NUM;
        int Fault_bit_position = rand() % OECC_CW_LEN;

        Chip_array[Fault_Chip_position][Fault_bit_position]=1;
        return; 
    }
    
}



// DE injection (Double Error injection)
void error_injection_DBE(unsigned int Chip_array[][OECC_CW_LEN], int recc_type)
{
    if (recc_type==OPC){
        int count = 0;
        while (count<2){
            int Fault_Chip_position = rand() % OPC_CHIP_NUM;
            int Fault_bit_position = rand() % OPC_OECC_CW_LEN;

            if (Chip_array[Fault_Chip_position][Fault_bit_position] != 1){
                Chip_array[Fault_Chip_position][Fault_bit_position] = 1;
                count++;
            }
        }
        return;
    }
    else if(recc_type==QPC){
        int count = 0;
        while (count<2){
            int Fault_Chip_position = rand() % CHIP_NUM;
            int Fault_bit_position = rand() % OECC_CW_LEN;

            if (Chip_array[Fault_Chip_position][Fault_bit_position] != 1){
                Chip_array[Fault_Chip_position][Fault_bit_position] = 1;
                count++;
            }
        }
        return;
    }
    else if(recc_type==AMDCHIPKILL){
        int count = 0;
        while (count<2){
            int Fault_Chip_position = rand() % CHIP_NUM;
            int Fault_bit_position = rand() % OECC_CW_LEN;

            if (Chip_array[Fault_Chip_position][Fault_bit_position] != 1){
                Chip_array[Fault_Chip_position][Fault_bit_position] = 1;
                count++;
            }
        }
        return;
    }
}


// TBE injection (Triple Error injection)
void error_injection_TBE(unsigned int Chip_array[][OECC_CW_LEN], int recc_type)
{
    if (recc_type==OPC){
        int count = 0;
        while (count<3){
            int Fault_Chip_position = rand() % OPC_CHIP_NUM;
            int Fault_bit_position = rand() % OPC_OECC_CW_LEN;

            if (Chip_array[Fault_Chip_position][Fault_bit_position] != 1){
                Chip_array[Fault_Chip_position][Fault_bit_position] = 1;
                count++;
            }
        }
        return;
    }
    else if(recc_type==QPC){
        int count = 0;
        while (count<3){
            int Fault_Chip_position = rand() % CHIP_NUM;
            int Fault_bit_position = rand() % OECC_CW_LEN;

            if (Chip_array[Fault_Chip_position][Fault_bit_position] != 1){
                Chip_array[Fault_Chip_position][Fault_bit_position] = 1;
                count++;
            }
        }
        return;
    }
    else if(recc_type==AMDCHIPKILL){
        int count = 0;
        while (count<3){
            int Fault_Chip_position = rand() % CHIP_NUM;
            int Fault_bit_position = rand() % OECC_CW_LEN;

            if (Chip_array[Fault_Chip_position][Fault_bit_position] != 1){
                Chip_array[Fault_Chip_position][Fault_bit_position] = 1;
                count++;
            }
        }
        return;
    }
}


// Chipkill injection
void error_injection_CHIPKILL(int Fault_Chip_position, unsigned int Chip_array[][OECC_CW_LEN], int recc_type)
{
    // 50% error.
    for(int Fault_pos=0; Fault_pos<OECC_CW_LEN; Fault_pos++){ // 0~63
        if(rand()%2!=0) // 0(no error) 'or' 1(error)
            Chip_array[Fault_Chip_position][Fault_pos]^=1;
    }
    
    return;

    if (recc_type==OPC){
        for(int Fault_pos=0; Fault_pos<OPC_OECC_CW_LEN; Fault_pos++){ // 0~63
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position][Fault_pos]^=1;
        }
        for(int Fault_pos=0; Fault_pos<OPC_OECC_CW_LEN; Fault_pos++){ // 0~63
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position+10][Fault_pos]^=1;
        }

        return;
    }
    else if(recc_type==QPC){
        for(int Fault_pos=0; Fault_pos<OECC_CW_LEN; Fault_pos++){ // 0~63
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position][Fault_pos]^=1;
        }
        
        return;
    }
    else if(recc_type==AMDCHIPKILL){
        for(int Fault_pos=0; Fault_pos<OECC_CW_LEN; Fault_pos++){ // 0~63
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position][Fault_pos]^=1;
        }
        
        return;
    }    
}



// Pin_error injection
void error_injection_pin(int Fault_Pin_position, unsigned int Chip_array[][OECC_CW_LEN], int recc_type)
{


    int Fault_Chip_position = Fault_Pin_position/4;
    int Fault_Chip_line = Fault_Pin_position % 4;

    if (recc_type==OPC){
        for(int Fault_pos=0; Fault_pos<8; Fault_pos++){ 
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position][8*Fault_Chip_line+Fault_pos]=1;
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position+10][8*Fault_Chip_line+Fault_pos]=1;
        }
        return;
    }
    else if(recc_type==QPC){
        for(int Fault_pos=0; Fault_pos<8; Fault_pos++){
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position][8*Fault_Chip_line+Fault_pos]=1;
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position][32+8*Fault_Chip_line+Fault_pos]=1;
        }
        return;
    }
    else if(recc_type==AMDCHIPKILL){
        for(int Fault_pos=0; Fault_pos<16; Fault_pos++){ 
            if(rand()%2!=0) // 0(no error) 'or' 1(error)
                Chip_array[Fault_Chip_position][4*Fault_pos+Fault_Chip_line]=1;
        }
    }

    
    return;
}



// RANK_error injection
void error_injection_rank(unsigned int Chip_array[][OECC_CW_LEN], int recc_type)
{

    if (recc_type==OPC){
        for(int Fault_Chip_position=0; Fault_Chip_position<OPC_CHIP_NUM; Fault_Chip_position++){    
            for(int Fault_pos=0; Fault_pos<OPC_OECC_CW_LEN; Fault_pos++){ 
                if(rand()%2!=0) // 0(no error) 'or' 1(error)
                    Chip_array[Fault_Chip_position][Fault_pos]=1;
            }
        }
        return;
    }
    else if(recc_type==QPC){
        for(int Fault_Chip_position=0; Fault_Chip_position<CHIP_NUM; Fault_Chip_position++){    
            for(int Fault_pos=0; Fault_pos<OECC_CW_LEN; Fault_pos++){ 
                if(rand()%2!=0) // 0(no error) 'or' 1(error)
                    Chip_array[Fault_Chip_position][Fault_pos]=1;
            }
        }
        return;
    }
    else if(recc_type==AMDCHIPKILL){
        for(int Fault_Chip_position=0; Fault_Chip_position<CHIP_NUM; Fault_Chip_position++){    
            for(int Fault_pos=0; Fault_pos<OECC_CW_LEN; Fault_pos++){ 
                if(rand()%2!=0) // 0(no error) 'or' 1(error)
                    Chip_array[Fault_Chip_position][Fault_pos]=1;
            }
        }
        return;
    }


}




// OECC 1bit correction
void error_correction_oecc(unsigned int *codeword)
{
    unsigned int Syndromes[OECC_REDUN_LEN]; // 8 x 1
    
    // Syndromes = H * C^T
    for(int row=0; row<OECC_REDUN_LEN; row++){
        unsigned int row_value=0;
        for(int column=0; column<OECC_CW_LEN; column++)
            row_value=row_value^(H_Matrix_OECC[row][column] * codeword[column]);
        Syndromes[row]=row_value;
    }

    // error correction (Check Syndromes)
    int cnt=0;
    for(int error_pos=0; error_pos<OECC_CW_LEN; error_pos++){
        cnt=0;
        for(int row=0; row<OECC_REDUN_LEN; row++){
            if(Syndromes[row]==H_Matrix_OECC[row][error_pos])
                cnt++;
            else
                break;
        }
        // 1-bit error -> error correction 
        if(cnt==OECC_REDUN_LEN){
            codeword[error_pos]^=1;
            return;
        }
    }

    return;
}


/*------------------------------------------------------------------
                        OPC Correction
-------------------------------------------------------------------*/
int error_correction_OPC(unsigned int *codeword_OPC)
{
   register int i,j,u,q ;

   int elp[nn-OPC_kk+2][nn-OPC_kk], d[nn-OPC_kk+2], l[nn-OPC_kk+2], u_lu[nn-OPC_kk+2], s[nn-OPC_kk+1] ; 
   int count=0, syn_error=0, root[OPC_tt], loc[OPC_tt], z[OPC_tt+1], err[nn], reg[OPC_tt+1]; 
   unsigned int recd[OPC_nn_short] = {0};

  for (i=1; i<=2*OPC_tt; i++){
    s[i] = 0;

    for(int symbol_index=0; symbol_index<OPC_CW_SYMBOL_NUM; symbol_index++){ 
        unsigned exponent=255; 
        unsigned symbol_value=0; 

        for(int symbol_value_index=0; symbol_value_index<SYMBOL_SIZE; symbol_value_index++){ 
            symbol_value^=(codeword_OPC[symbol_index*8+symbol_value_index] << (SYMBOL_SIZE-1-symbol_value_index)); 
        }

        recd[symbol_index] = symbol_value;

        for(int prim_exponent=0; prim_exponent<255; prim_exponent++){
            if(symbol_value==primitive_poly[0][prim_exponent]){
                exponent=prim_exponent;
                break;
            }
        }
        
        if(exponent!=255)
            s[i]^=primitive_poly[0][(exponent+i*symbol_index)%255];
    }

    if (s[i]!=0){
        syn_error=1;
    }    
    s[i] = index_of(s[i]);
  }

 // Syndrome != 0 -> CE 'or' DUE 'or' SDC
  if (syn_error)       /* if errors, try and correct */
  {
/* initialise table entries */
      d[0] = 0 ;           /* index form */
      d[1] = s[1] ;        /* index form */
      elp[0][0] = 0 ;      /* index form */
      elp[1][0] = 1 ;      /* polynomial form */
      for (i=1; i<nn-OPC_kk; i++)
        { elp[0][i] = -1 ;   /* index form */
          elp[1][i] = 0 ;   /* polynomial form */
        }
      l[0] = 0 ;     
      l[1] = 0 ;     
      u_lu[0] = -1 ; /* index form */
      u_lu[1] = 0 ;  /* index form */
      u = 0 ; //step

      do
      {
        u++ ;
        if (d[u]==-1)  // s[i] == 0
          { l[u+1] = l[u];
            for (i=0; i<=l[u]; i++)
             {  elp[u+1][i] = elp[u][i] ;
                elp[u][i] = index_of(elp[u][i]) ;
             }
          }
        else
/* search for words with greatest u_lu[q] for which d[q]!=0 */
          { q = u-1 ;
            while ((d[q]==-1) && (q>0)) q-- ;
/* have found first non-zero d[q]  */
            if (q>0)
             { j=q ;
               do
               { j-- ;
                 if ((d[j]!=-1) && (u_lu[q]<u_lu[j]))
                   q = j ;
               }while (j>0) ;
             }

/* have now found q such that d[u]!=0 and u_lu[q] is maximum */
/* store degree of new elp polynomial */
            if (l[u]>l[q]+u-q)  l[u+1] = l[u] ;
            else  l[u+1] = l[q]+u-q ;

/* form new elp(x) */
            for (i=0; i<nn-OPC_kk; i++)    elp[u+1][i] = 0 ;
            for (i=0; i<=l[q]; i++)
              if (elp[q][i]!=-1)
                elp[u+1][i+u-q] = primitive_poly[0][(d[u]+nn-d[q]+elp[q][i])%nn] ;
            for (i=0; i<=l[u]; i++)
              { elp[u+1][i] ^= elp[u][i] ;
                elp[u][i] = index_of(elp[u][i]) ;  /*convert old elp value to index*/
              }
          }
        u_lu[u+1] = u-l[u+1] ;

/* form (u+1)th discrepancy */
        if (u<nn-OPC_kk)    /* no discrepancy computed on last iteration */
          {
            if (s[u+1]!=-1)
                   d[u+1] = primitive_poly[0][s[u+1]] ;
            else
              d[u+1] = 0 ;
            for (i=1; i<=l[u+1]; i++)
              if ((s[u+1-i]!=-1) && (elp[u+1][i]!=0))
                d[u+1] ^= primitive_poly[0][(s[u+1-i]+index_of(elp[u+1][i]))%nn] ;
            d[u+1] = index_of(d[u+1]) ;    /* put d[u+1] into index form */
          }
      } while ((u<nn-OPC_kk) && (l[u+1]<=OPC_tt)) ;

      u++ ;

      ////////////////////////////////////////////////////
      // error correction start!!!!!!!!!!!!!!!!!!!!!!!!!
      ////////////////////////////////////////////////////

      // CE 'or' SDC cases
      if (l[u]<=OPC_tt)         /* can correct error */
      {
/* put elp into index form */
         for (i=0; i<=l[u]; i++)   elp[u][i] = index_of(elp[u][i]) ;

/* find roots of the error location polynomial -> finding error location */
        for (i=1; i<=l[u]; i++)
          reg[i] = elp[u][i];
        
        count = 0;
        for (i=1; i<=nn; i++){  
          q = 1 ;
          for (j=1; j<=l[u]; j++)
            if (reg[j]!=-1){ 
              reg[j] = (reg[j]+j)%nn;
              q ^= primitive_poly[0][reg[j]];
            }
          if (!q) {        /* store root and error location number indices */
            root[count] = i;
            loc[count] = nn-i; // -> error location
            count++;
          }
        }
        
        int CE_cases=1;
        //printf("error location check! (shortened RS code!)\n");
        for(int index=0; index<count; index++){
          if(loc[index]>=OPC_nn_short) // except for zero-padding part!!
            CE_cases=0;
        }
        
        if (l[u]>4){
            int reference_value = loc[0] / 4;
            for (int index = 0; index < count; index++) {
                int divided_value = loc[index] / 4;             
                
                if ((divided_value != reference_value) && ((divided_value + 10) != reference_value)) {
                    CE_cases = 0;
                }
            }
        }

         // CE 'or' SDC cases
         if (count==l[u] && CE_cases==1){    /* no. roots = degree of elp hence <= tt errors*/
          //printf("CE 'or' SDC cases\n");
          //printf("count : %d\n",count);
/* form polynomial z(x) */
          for (i=1; i<=l[u]; i++){        /* Z[0] = 1 always - do not need */
            if ((s[i]!=-1) && (elp[u][i]!=-1))
              z[i] = primitive_poly[0][s[i]] ^ primitive_poly[0][elp[u][i]];
            else if ((s[i]!=-1) && (elp[u][i]==-1))
              z[i] = primitive_poly[0][s[i]] ;
            else if ((s[i]==-1) && (elp[u][i]!=-1))
              z[i] = primitive_poly[0][elp[u][i]] ;
            else
              z[i] = 0 ;
            for (j=1; j<i; j++){
              if ((s[j]!=-1) && (elp[u][i-j]!=-1))
                z[i] ^= primitive_poly[0][(elp[u][i-j] + s[j])%nn];
            }
            z[i] = index_of(z[i]);         /* put into index form */
          }

  /* evaluate errors at locations given by error location numbers loc[i] */
          for (i=0; i<nn; i++){ 
            err[i] = 0;
          }

          for (i=0; i<l[u]; i++){    /* compute numerator of error term first */
            err[loc[i]] = 1;       /* accounts for z[0] */
            for (j=1; j<=l[u]; j++){
              if (z[j]!=-1)
                err[loc[i]] ^= primitive_poly[0][(z[j]+j*root[i])%nn];
            }
            if (err[loc[i]]!=0){
              err[loc[i]] = index_of(err[loc[i]]);
                q = 0;     /* form denominator of error term */
                for (j=0; j<l[u]; j++){
                  if (j!=i)
                    q += index_of(1^primitive_poly[0][(loc[j]+root[i])%nn]);
                }
                q = q % nn;
                err[loc[i]] = primitive_poly[0][(err[loc[i]]-q+nn)%nn];
                recd[loc[i]] ^= err[loc[i]];  /*recd[i] must be in polynomial form */   
            }
          }

            for (int symbol_index=0; symbol_index<80; symbol_index++) {
                if (recd[symbol_index] == 0)
                    for (int symbol_value_index=0; symbol_value_index<8; symbol_value_index++){
                        codeword_OPC[symbol_index*8+symbol_value_index] = 0;
                    }
            }


 
          return CE;
          /*
          printf("err (error values) : ");
          for(int index=0; index<nn; index++)
            printf("%d ",err[index]);
          printf("\n");
          */
        }
        // DUE cases
        else{    /* no. roots != degree of elp => >tt errors and cannot solve */
          //printf("DUE cases!!!\n");

          return DUE;
        }
      }
    // DUE cases
     else{         /* elp has degree has degree >tt hence cannot solve */
       //printf("DUE cases!\n");
       return DUE;
     }
  }
  // NE 'or' SDC cases
  else{       /* no non-zero syndromes => no errors: output received codeword */
    //printf("NE 'or' SDC cases!\n");
    return NE;
  }
}



/*------------------------------------------------------------------
                        QPC Correction
-------------------------------------------------------------------*/
int error_correction_QPC(unsigned int *codeword)
{
   register int i,j,u,q ;
   int elp[nn-QPC_kk+2][nn-QPC_kk], d[nn-QPC_kk+2], l[nn-QPC_kk+2], u_lu[nn-QPC_kk+2], s[nn-QPC_kk+1] ; 
   int count=0, syn_error=0, root[QPC_tt], loc[QPC_tt], z[QPC_tt+1], err[nn], reg[QPC_tt+1]; 
   unsigned int recd[QPC_nn_short] = {0};

  for (i=1; i<=2*QPC_tt; i++){
    s[i] = 0;

    for(int symbol_index=0; symbol_index<QPC_CW_SYMBOL_NUM; symbol_index++){ // 0~39
        unsigned exponent=255;
        unsigned symbol_value=0; // 0000_0000 ~ 1111_1111

        for(int symbol_value_index=0; symbol_value_index<SYMBOL_SIZE; symbol_value_index++){ // 8-bit symbol
            symbol_value^=(codeword[symbol_index*8+symbol_value_index] << (SYMBOL_SIZE-1-symbol_value_index)); // <<7, <<6, ... <<0
        }

        recd[symbol_index] = symbol_value;

        for(int prim_exponent=0; prim_exponent<255; prim_exponent++){
            if(symbol_value==primitive_poly[0][prim_exponent]){
                exponent=prim_exponent;
                break;
            }
        }
        
        if(exponent!=255) // s[i] = (a^exponent0) ^ (a^[exponent1+i*1]) ^ (a^[exponent2+i*2]) ... ^ (a^[exponent39+i*39])
            s[i]^=primitive_poly[0][(exponent+i*symbol_index)%255];
    }

    if (s[i]!=0)  syn_error=1;    
    s[i] = index_of(s[i]);
  }

 // Syndrome != 0 -> CE 'or' DUE 'or' SDC
  if (syn_error)       /* if errors, try and correct */
  {
/* initialise table entries */
      d[0] = 0 ;           /* index form */
      d[1] = s[1] ;        /* index form */
      elp[0][0] = 0 ;      /* index form */
      elp[1][0] = 1 ;      /* polynomial form */
      for (i=1; i<nn-QPC_kk; i++)
        { elp[0][i] = -1 ;   /* index form */
          elp[1][i] = 0 ;   /* polynomial form */
        }
      l[0] = 0 ;     
      l[1] = 0 ;     
      u_lu[0] = -1 ; /* index form */
      u_lu[1] = 0 ;  /* index form */
      u = 0 ; //step

      do
      {
        u++ ;
        if (d[u]==-1)  // s[i] == 0
          { l[u+1] = l[u];
            for (i=0; i<=l[u]; i++)
             {  elp[u+1][i] = elp[u][i] ;
                elp[u][i] = index_of(elp[u][i]) ;
             }
          }
        else
/* search for words with greatest u_lu[q] for which d[q]!=0 */
          { q = u-1 ;
            while ((d[q]==-1) && (q>0)) q-- ;
/* have found first non-zero d[q]  */
            if (q>0)
             { j=q ;
               do
               { j-- ;
                 if ((d[j]!=-1) && (u_lu[q]<u_lu[j]))
                   q = j ;
               }while (j>0) ;
             }

/* have now found q such that d[u]!=0 and u_lu[q] is maximum */
/* store degree of new elp polynomial */
            if (l[u]>l[q]+u-q)  l[u+1] = l[u] ;
            else  l[u+1] = l[q]+u-q ;

/* form new elp(x) */
            for (i=0; i<nn-QPC_kk; i++)    elp[u+1][i] = 0 ;
            for (i=0; i<=l[q]; i++)
              if (elp[q][i]!=-1)
                elp[u+1][i+u-q] = primitive_poly[0][(d[u]+nn-d[q]+elp[q][i])%nn] ;
            for (i=0; i<=l[u]; i++)
              { elp[u+1][i] ^= elp[u][i] ;
                elp[u][i] = index_of(elp[u][i]) ;  /*convert old elp value to index*/
              }
          }
        u_lu[u+1] = u-l[u+1] ;

/* form (u+1)th discrepancy */
        if (u<nn-QPC_kk)    /* no discrepancy computed on last iteration */
          {
            if (s[u+1]!=-1)
                   d[u+1] = primitive_poly[0][s[u+1]] ;
            else
              d[u+1] = 0 ;
            for (i=1; i<=l[u+1]; i++)
              if ((s[u+1-i]!=-1) && (elp[u+1][i]!=0))
                d[u+1] ^= primitive_poly[0][(s[u+1-i]+index_of(elp[u+1][i]))%nn] ;
            d[u+1] = index_of(d[u+1]) ;    /* put d[u+1] into index form */
          }
      } while ((u<nn-QPC_kk) && (l[u+1]<=QPC_tt)) ;

      u++ ;

      ////////////////////////////////////////////////////
      // error correction start!!!!!!!!!!!!!!!!!!!!!!!!!
      ////////////////////////////////////////////////////

      // CE 'or' SDC cases
      if (l[u]<=QPC_tt)         /* can correct error */
      {
/* put elp into index form */
         for (i=0; i<=l[u]; i++)   elp[u][i] = index_of(elp[u][i]) ;

/* find roots of the error location polynomial -> finding error location */
        for (i=1; i<=l[u]; i++)
          reg[i] = elp[u][i];
        
        count = 0;
        for (i=1; i<=nn; i++){  
          q = 1 ;
          for (j=1; j<=l[u]; j++)
            if (reg[j]!=-1){ 
              reg[j] = (reg[j]+j)%nn;
              q ^= primitive_poly[0][reg[j]];
            }
          if (!q) {        /* store root and error location number indices */
            root[count] = i;
            loc[count] = nn-i; // -> error location
            count++;
          }
        }
        
        int CE_cases=1;
        //printf("error location check! (shortened RS code!)\n");
        for(int index=0; index<count; index++){
          if(loc[index]>=QPC_nn_short) // except for zero-padding part!!
            CE_cases=0;
        }
        
        if (l[u]>2){
            for(int index=0; index<count; index++){
                if((loc[index] / 4) != (loc[0] / 4)) // except for zero-padding part!!
                    CE_cases=0;
            }
        }

         // CE 'or' SDC cases
         if (count==l[u] && CE_cases==1){    /* no. roots = degree of elp hence <= tt errors*/
          //printf("CE 'or' SDC cases\n");
          //printf("count : %d\n",count);
/* form polynomial z(x) */
          for (i=1; i<=l[u]; i++){        /* Z[0] = 1 always - do not need */
            if ((s[i]!=-1) && (elp[u][i]!=-1))
              z[i] = primitive_poly[0][s[i]] ^ primitive_poly[0][elp[u][i]];
            else if ((s[i]!=-1) && (elp[u][i]==-1))
              z[i] = primitive_poly[0][s[i]] ;
            else if ((s[i]==-1) && (elp[u][i]!=-1))
              z[i] = primitive_poly[0][elp[u][i]] ;
            else
              z[i] = 0 ;
            for (j=1; j<i; j++){ 
              if ((s[j]!=-1) && (elp[u][i-j]!=-1))
                z[i] ^= primitive_poly[0][(elp[u][i-j] + s[j])%nn];
            }
            z[i] = index_of(z[i]);         /* put into index form */
          }

  /* evaluate errors at locations given by error location numbers loc[i] */
          for (i=0; i<nn; i++){ 
            err[i] = 0;
          }

          for (i=0; i<l[u]; i++){    /* compute numerator of error term first */
            err[loc[i]] = 1;       /* accounts for z[0] */
            for (j=1; j<=l[u]; j++){
              if (z[j]!=-1)
                err[loc[i]] ^= primitive_poly[0][(z[j]+j*root[i])%nn];
            }
            if (err[loc[i]]!=0){
              err[loc[i]] = index_of(err[loc[i]]);
                q = 0;     /* form denominator of error term */
                for (j=0; j<l[u]; j++){
                  if (j!=i)
                    q += index_of(1^primitive_poly[0][(loc[j]+root[i])%nn]);
                }
                q = q % nn;
                err[loc[i]] = primitive_poly[0][(err[loc[i]]-q+nn)%nn];
                recd[loc[i]] ^= err[loc[i]];  /*recd[i] must be in polynomial form */   
            }
          }

            for (int symbol_index=0; symbol_index<40; symbol_index++) {
                if (recd[symbol_index] == 0)
                    for (int symbol_value_index=0; symbol_value_index<8; symbol_value_index++){
                        codeword[symbol_index*8+symbol_value_index] = 0;
                    }
            }


 
          return CE;
          /*
          printf("err (error values) : ");
          for(int index=0; index<nn; index++)
            printf("%d ",err[index]);
          printf("\n");
          */
        }
        // DUE cases
        else{    /* no. roots != degree of elp => >tt errors and cannot solve */
          //printf("DUE cases!!!\n");

          return DUE;
        }
      }
    // DUE cases
     else{         /* elp has degree has degree >tt hence cannot solve */
       //printf("DUE cases!\n");
       return DUE;
     }
  }
  // NE 'or' SDC cases
  else{       /* no non-zero syndromes => no errors: output received codeword */
    //printf("NE 'or' SDC cases!\n");
    return NE;
  }
}



/*------------------------------------------------------------------
                        AMD Correction
-------------------------------------------------------------------*/
int error_correction_AMDCHIPKILL(unsigned int *codeword, set<int> &error_chip_position)
{

    // Syndrome 
    // S0 = (a^exponent0) ^ (a^exponent1) ^ (a^exponent2) ... ^(a^exponent9)
    // S1 = (a^exponent0) ^ (a^[exponent1+1]) ^ (a^[exponent2+2]) ... ^ (a^[exponent9+9])
    // S0 
    unsigned int S0=0,S1=0;
    for(int symbol_index=0; symbol_index<AMDCHIPKILL_CW_SYMBOL_NUM; symbol_index++){ // 0~9
        unsigned exponent=255; 
        unsigned symbol_value=0; // 0000_0000 ~ 1111_1111

        for(int symbol_value_index=0; symbol_value_index<SYMBOL_SIZE; symbol_value_index++){ // 8-bit symbol
            symbol_value^=(codeword[symbol_index*8+symbol_value_index] << (SYMBOL_SIZE-1-symbol_value_index)); // <<7, <<6, ... <<0
        }
        for(int prim_exponent=0; prim_exponent<255; prim_exponent++){
            if(symbol_value==primitive_poly[0][prim_exponent]){
                exponent=prim_exponent;
                break;
            }
        }
        //printf("symbol_index : %d, symbol_value : %d\n",symbol_index, symbol_value);

        if(exponent!=255) // S0 = (a^exponent0) ^ (a^exponent1) ^ (a^exponent2) ... ^(a^exponent9)
            S0^=primitive_poly[0][exponent];
    }


    // S1 
    for(int symbol_index=0; symbol_index<AMDCHIPKILL_CW_SYMBOL_NUM; symbol_index++){ // 0~9
        unsigned exponent=255;
        unsigned symbol_value=0; // 0000_0000 ~ 1111_1111
        for(int symbol_value_index=0; symbol_value_index<SYMBOL_SIZE; symbol_value_index++){ // 8-bit symbol
            symbol_value^=(codeword[symbol_index*8+symbol_value_index] << (SYMBOL_SIZE-1-symbol_value_index)); // <<7, <<6, ... <<0
        }
        for(int prim_exponent=0; prim_exponent<255; prim_exponent++){
            if(symbol_value==primitive_poly[0][prim_exponent]){
                exponent=prim_exponent;
                break;
            }
        }
        
        if(exponent!=255) // S1 = (a^exponent0) ^ (a^[exponent1+1]) ^ (a^[exponent2+2]) ... ^ (a^[exponent9+9])
            S1^=primitive_poly[0][(exponent+symbol_index)%255];
    }

    // S0 = a^p, S1= a^q (a^0 ~ a^254)
    unsigned int p,q;
    for(int prim_exponent=0; prim_exponent<255; prim_exponent++){
        if(S0==primitive_poly[0][prim_exponent])
            p=prim_exponent;
        if(S1==primitive_poly[0][prim_exponent])
            q=prim_exponent;
    }
    //printf("S0 : %d(a^%d), S1 : %d(a^%d)\n",S0,p,S1,q);

    //printf("S0 : %d\n",S0);
    if(S0==0 && S1==0){ // NE (No Error)
        return NE;
    }
    
    // CE 'or' DUE
    // error chip position
    int error_symbol_position_recc;
    error_symbol_position_recc=(q+255-p)%255;

    // Table
    if(0<=error_symbol_position_recc && error_symbol_position_recc < CHIP_NUM){ // CE (error chip location : 0~9)
        // printf("CE case! error correction start!\n");
        //error correction
        for(int symbol_index=0; symbol_index<SYMBOL_SIZE; symbol_index++){ // 0~7
            codeword[error_symbol_position_recc*SYMBOL_SIZE+symbol_index]^=getAbit(S0, SYMBOL_SIZE-1-symbol_index); // S0 >> 7, S0 >> 6 ... S0 >> 0
            //Chip_array[error_symbol_position_recc][BL*4+symbol_index]^=getAbit(S0, SYMBOL_SIZE-1-symbol_index); // S0 >> 7, S0 >> 6 ... S0 >> 0
        }
        // printf("CE case! error correction done!\n");     
        error_chip_position.insert(error_symbol_position_recc);
        return CE;
    }
    // Table End!!!!!
    
    // DUE
    return DUE;
}




int SDC_check(int BL, unsigned int Chip_array[][OECC_CW_LEN], int recc_type)
{

    int error_check=0;

    if(recc_type==OPC){
        for(int Error_chip_pos=0; Error_chip_pos<OPC_CHIP_NUM; Error_chip_pos++){ 
            for(int Fault_pos=0; Fault_pos<OPC_OECC_CW_LEN; Fault_pos++){ 
                if(Chip_array[Error_chip_pos][Fault_pos]==1){
                    error_check++;
                    return error_check;
                }
            }
        }
    }
    else if(recc_type==QPC){
        for(int Error_chip_pos=0; Error_chip_pos<CHIP_NUM; Error_chip_pos++){
            for(int Fault_pos=32*BL; Fault_pos<32*BL+32; Fault_pos++){ 
                if(Chip_array[Error_chip_pos][Fault_pos]==1){
                    error_check++;
                    return error_check;
                }
            }
        }
    }
    else if(recc_type==AMDCHIPKILL){
       for(int Error_chip_pos=0; Error_chip_pos<CHIP_NUM; Error_chip_pos++){ 
            for(int Fault_pos=BL*4; Fault_pos<(BL+2)*4; Fault_pos++){ 
                if(Chip_array[Error_chip_pos][Fault_pos]==1){
                    error_check++;
                    return error_check;
                }
            }
        }
    }
    else if(recc_type==RECC_OFF){
        for(int Error_chip_pos=0; Error_chip_pos<CHIP_NUM; Error_chip_pos++){ 
            for(int Fault_pos=0; Fault_pos<OECC_DATA_LEN; Fault_pos++){
                if(Chip_array[Error_chip_pos][Fault_pos]==1){
                    error_check++;
                    return error_check;
                }
            }
        }
    }

    return error_check;
}


int main(int argc, char* argv[])
{
    ///////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////

    // 0. GF(2^8) primitive polynomial table 
    FILE *fp=fopen("GF_2^8__primitive_polynomial.txt","r");
    int primitive_count=0;
    while(1){
        char str_read[100];
        unsigned int primitive_value=0;
        fgets(str_read,100,fp);
        primitive_value=conversion_to_int_format(str_read, 8);

        generate_primitive_poly(primitive_value,8,primitive_count); 
        primitive_count++;

        if(feof(fp))
            break;
    }
    fclose(fp);

    ///////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////

    // 1. H_Matrix 
    // OECC (On-die ECC)
    FILE *fp1=fopen("H_Matrix_OECC.txt","r");
    while(1){
        unsigned int value;
        for(int row=0; row<OECC_REDUN_LEN; row++){
            for(int column=0; column<OECC_CW_LEN; column++){
                fscanf(fp1,"%d ",&value);
                H_Matrix_OECC[row][column]=value;
                //printf("%d ",H_Matrix_binary[row][column]);
            }
        }
        if(feof(fp1))
            break;
    }
    fclose(fp1);



    // 2. name of output files

    string OECC="X", RECC="X", FAULT="X"; 
    int oecc_type, recc_type, fault_type; // => on-die ECC, Rank-level ECC, fault_type 
    oecc_recc_fault_type_assignment(OECC, FAULT, RECC, &oecc_type, &fault_type, &recc_type, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    
    string Result_file_name = RECC + "_" + FAULT + ".S";
    FILE *fp3=fopen(Result_file_name.c_str(),"w"); 

    // 3. iteration
    unsigned int Chip_array[OPC_CHIP_NUM][OECC_CW_LEN]; // chip configuration


    int CE_cnt=0, DUE_cnt=0, SDC_cnt=0; // num of CE, DUE, SDC 
    srand((unsigned int)time(NULL)); 
    double error_scenario;
    for(int runtime=0; runtime<RUN_NUM; runtime++){
        if(runtime%1000000==0){
            fprintf(fp3,"\n===============\n");
            fprintf(fp3,"Runtime : %d/%d\n",runtime,RUN_NUM);
            fprintf(fp3,"CE : %d\n",CE_cnt);
            fprintf(fp3,"DUE : %d\n",DUE_cnt);
            fprintf(fp3,"SDC : %d\n",SDC_cnt);
            fprintf(fp3,"\n===============\n");
	    fflush(fp3);
        }

        // Linear block code
        for(int i=0; i<OPC_CHIP_NUM; i++)
            memset(Chip_array[i], 0, sizeof(unsigned int) * OECC_CW_LEN); 


        // 4-2. Error injection
        vector<int> Fault_Chip_position;
        while (Fault_Chip_position.size() < 4) {
            int random_pos = rand() % CHIP_NUM; // 0~39
            if (std::find(Fault_Chip_position.begin(), Fault_Chip_position.end(), random_pos) == Fault_Chip_position.end()) {

                Fault_Chip_position.push_back(random_pos);
            }
        }
     
        vector<int> Fault_Pin_position;
        while (Fault_Pin_position.size() < 4) {
            int random_position = rand() % 40; // 0~39
            if (std::find(Fault_Pin_position.begin(), Fault_Pin_position.end(), random_position) == Fault_Pin_position.end()) {
                Fault_Pin_position.push_back(random_position);
            }
        }


        switch (fault_type){
            case SBE: // 1bit
                error_injection_SE(Chip_array, recc_type);
                break;
            case PIN_1:
                error_injection_pin(Fault_Pin_position[0],Chip_array, recc_type);           
                break;

            case SCE: 
                error_injection_CHIPKILL(Fault_Chip_position[0],Chip_array, recc_type);
                break;
            case DBE: 
                error_injection_DBE(Chip_array, recc_type);
                break;
            case TBE:                 
                error_injection_TBE(Chip_array, recc_type);          
                break;
            case SCE_SBE:                   
                error_injection_CHIPKILL(Fault_Chip_position[0],Chip_array, recc_type); 
                error_injection_SE(Chip_array, recc_type); 
                break;
            case SCE_DBE:                
                error_injection_CHIPKILL(Fault_Chip_position[0],Chip_array, recc_type);                     
                error_injection_SE(Chip_array, recc_type); 
                error_injection_SE(Chip_array, recc_type);  
                break;

            case SCE_SCE: 
                error_injection_CHIPKILL(Fault_Chip_position[0],Chip_array, recc_type); 
                error_injection_CHIPKILL(Fault_Chip_position[1],Chip_array, recc_type); 
                break;
            case RANK:
                error_injection_rank(Chip_array, recc_type);        
                break;                                             
            default:
                break;
        }

        // 4-3. OECC
        switch(oecc_type){
            case OECC_OFF:
                break;
            case OECC_ON:
                break;
            default:
                break;
        }


        // 4-4. RECC
        set<int> error_chip_position;
        int result_type_recc; // NE, CE, DUE, SDC 
        int final_result, final_result_1=CE,final_result_2=CE;
        int Chip_idx=0;
        int isConservative=0;        
        switch(recc_type){
/*------------------------------------------------------------------
                        OPC Case
-------------------------------------------------------------------*/            
            case OPC:
                // 1st memory transfer block
                unsigned int codeword_OPC[OPC_CW_LEN];
                Chip_idx=0;
                while(Chip_idx<OPC_CHIP_NUM){
                    for(int Pin_num=0; Pin_num<CHIP_WIDTH; Pin_num++){
                        codeword_OPC[Chip_idx*32+Pin_num*8]=Chip_array[Chip_idx][Pin_num*8];
                        codeword_OPC[Chip_idx*32+Pin_num*8+1]=Chip_array[Chip_idx][Pin_num*8+1];
                        codeword_OPC[Chip_idx*32+Pin_num*8+2]=Chip_array[Chip_idx][Pin_num*8+2];
                        codeword_OPC[Chip_idx*32+Pin_num*8+3]=Chip_array[Chip_idx][Pin_num*8+3];
                        codeword_OPC[Chip_idx*32+Pin_num*8+4]=Chip_array[Chip_idx][Pin_num*8+4];
                        codeword_OPC[Chip_idx*32+Pin_num*8+5]=Chip_array[Chip_idx][Pin_num*8+5];
                        codeword_OPC[Chip_idx*32+Pin_num*8+6]=Chip_array[Chip_idx][Pin_num*8+6];
                        codeword_OPC[Chip_idx*32+Pin_num*8+7]=Chip_array[Chip_idx][Pin_num*8+7];
                    }
                    Chip_idx++;
                }


                // RECC implimentation
                result_type_recc = error_correction_OPC(codeword_OPC); 

                Chip_idx=0;
                while(Chip_idx<OPC_CHIP_NUM){
                    for(int Pin_num=0; Pin_num<CHIP_WIDTH; Pin_num++){                        
                        Chip_array[Chip_idx][Pin_num*8]=codeword_OPC[Chip_idx*32+Pin_num*8];
                        Chip_array[Chip_idx][Pin_num*8+1]=codeword_OPC[Chip_idx*32+Pin_num*8+1];
                        Chip_array[Chip_idx][Pin_num*8+2]=codeword_OPC[Chip_idx*32+Pin_num*8+2];
                        Chip_array[Chip_idx][Pin_num*8+3]=codeword_OPC[Chip_idx*32+Pin_num*8+3];
                        Chip_array[Chip_idx][Pin_num*8+4]=codeword_OPC[Chip_idx*32+Pin_num*8+4];
                        Chip_array[Chip_idx][Pin_num*8+5]=codeword_OPC[Chip_idx*32+Pin_num*8+5];
                        Chip_array[Chip_idx][Pin_num*8+6]=codeword_OPC[Chip_idx*32+Pin_num*8+6];
                        Chip_array[Chip_idx][Pin_num*8+7]=codeword_OPC[Chip_idx*32+Pin_num*8+7];
                    }
                    Chip_idx++;
                }


                // SDC check
                if(result_type_recc==CE || result_type_recc==NE){
                    int error_check=SDC_check(0, Chip_array, recc_type);
                    if(error_check){
                        final_result_1=SDC;
                    }
                }

                // DUE check
                if(result_type_recc==DUE)
                    final_result_1=DUE;
 

                // final result update
                final_result = final_result_1; 
                break;

 /*------------------------------------------------------------------
                        QPC Case
-------------------------------------------------------------------*/
               

            case QPC:
                // 1st memory transfer block

                unsigned int codeword[QPC_CW_LEN];
                Chip_idx=0;
                while(Chip_idx<CHIP_NUM){
                    for(int Pin_num=0; Pin_num<CHIP_WIDTH; Pin_num++){
                        codeword[Chip_idx*32+Pin_num*8]=Chip_array[Chip_idx][Pin_num*8];
                        codeword[Chip_idx*32+Pin_num*8+1]=Chip_array[Chip_idx][Pin_num*8+1];
                        codeword[Chip_idx*32+Pin_num*8+2]=Chip_array[Chip_idx][Pin_num*8+2];
                        codeword[Chip_idx*32+Pin_num*8+3]=Chip_array[Chip_idx][Pin_num*8+3];
                        codeword[Chip_idx*32+Pin_num*8+4]=Chip_array[Chip_idx][Pin_num*8+4];
                        codeword[Chip_idx*32+Pin_num*8+5]=Chip_array[Chip_idx][Pin_num*8+5];
                        codeword[Chip_idx*32+Pin_num*8+6]=Chip_array[Chip_idx][Pin_num*8+6];
                        codeword[Chip_idx*32+Pin_num*8+7]=Chip_array[Chip_idx][Pin_num*8+7];
                    }
                    Chip_idx++;
                }



                // RECC implimentation
                result_type_recc = error_correction_QPC(codeword); 

                Chip_idx=0;
                while(Chip_idx<CHIP_NUM){
                    for(int Pin_num=0; Pin_num<CHIP_WIDTH; Pin_num++){                        
                        Chip_array[Chip_idx][Pin_num*8]=codeword[Chip_idx*32+Pin_num*8];
                        Chip_array[Chip_idx][Pin_num*8+1]=codeword[Chip_idx*32+Pin_num*8+1];
                        Chip_array[Chip_idx][Pin_num*8+2]=codeword[Chip_idx*32+Pin_num*8+2];
                        Chip_array[Chip_idx][Pin_num*8+3]=codeword[Chip_idx*32+Pin_num*8+3];
                        Chip_array[Chip_idx][Pin_num*8+4]=codeword[Chip_idx*32+Pin_num*8+4];
                        Chip_array[Chip_idx][Pin_num*8+5]=codeword[Chip_idx*32+Pin_num*8+5];
                        Chip_array[Chip_idx][Pin_num*8+6]=codeword[Chip_idx*32+Pin_num*8+6];
                        Chip_array[Chip_idx][Pin_num*8+7]=codeword[Chip_idx*32+Pin_num*8+7];
                    }
                    Chip_idx++;
                }


                // SDC check
                if(result_type_recc==CE || result_type_recc==NE){
                    int error_check=SDC_check(0, Chip_array, recc_type);
                    if(error_check){
                        final_result_1=SDC;
                    }
                }
                // DUE check
                if(result_type_recc==DUE)
                    final_result_1=DUE;
            

                // 2nd memory transfer block
                Chip_idx=0;
                while(Chip_idx<CHIP_NUM){
                    for(int Pin_num=0; Pin_num<CHIP_WIDTH; Pin_num++){
                        codeword[Chip_idx*32+Pin_num*8]=Chip_array[Chip_idx][32+Pin_num*8];
                        codeword[Chip_idx*32+Pin_num*8+1]=Chip_array[Chip_idx][32+Pin_num*8+1];
                        codeword[Chip_idx*32+Pin_num*8+2]=Chip_array[Chip_idx][32+Pin_num*8+2];
                        codeword[Chip_idx*32+Pin_num*8+3]=Chip_array[Chip_idx][32+Pin_num*8+3];
                        codeword[Chip_idx*32+Pin_num*8+4]=Chip_array[Chip_idx][32+Pin_num*8+4];
                        codeword[Chip_idx*32+Pin_num*8+5]=Chip_array[Chip_idx][32+Pin_num*8+5];
                        codeword[Chip_idx*32+Pin_num*8+6]=Chip_array[Chip_idx][32+Pin_num*8+6];
                        codeword[Chip_idx*32+Pin_num*8+7]=Chip_array[Chip_idx][32+Pin_num*8+7];
                    }
                    Chip_idx++;
                }

                // RECC implimentation
                result_type_recc = error_correction_QPC(codeword); 

                Chip_idx=0;
                while(Chip_idx<CHIP_NUM){
                    for(int Pin_num=0; Pin_num<CHIP_WIDTH; Pin_num++){                        
                        Chip_array[Chip_idx][32+Pin_num*8]=codeword[Chip_idx*32+Pin_num*8];
                        Chip_array[Chip_idx][32+Pin_num*8+1]=codeword[Chip_idx*32+Pin_num*8+1];
                        Chip_array[Chip_idx][32+Pin_num*8+2]=codeword[Chip_idx*32+Pin_num*8+2];
                        Chip_array[Chip_idx][32+Pin_num*8+3]=codeword[Chip_idx*32+Pin_num*8+3];
                        Chip_array[Chip_idx][32+Pin_num*8+4]=codeword[Chip_idx*32+Pin_num*8+4];
                        Chip_array[Chip_idx][32+Pin_num*8+5]=codeword[Chip_idx*32+Pin_num*8+5];
                        Chip_array[Chip_idx][32+Pin_num*8+6]=codeword[Chip_idx*32+Pin_num*8+6];
                        Chip_array[Chip_idx][32+Pin_num*8+7]=codeword[Chip_idx*32+Pin_num*8+7];
                    }
                    Chip_idx++;
                }


                // SDC check
                if(result_type_recc==CE || result_type_recc==NE){
                    int error_check=SDC_check(1, Chip_array, recc_type);
                    if(error_check){
                        final_result_2=SDC;
                    }
                }
                // DUE check
                if(result_type_recc==DUE)
                    final_result_2=DUE;


                // final result update
                final_result = (final_result_1 > final_result_2) ? final_result_1 : final_result_2;

                break;


 /*------------------------------------------------------------------
                        AMD Case
-------------------------------------------------------------------*/


            case AMDCHIPKILL:
                // 1st memory transfer block
                for(int BL=0; BL<16; BL+=2){ // BL (Burst Length)<16 
                    unsigned int codeword_AMD[AMDCHIPKILL_CW_LEN];
                    Chip_idx=0;
                    while(Chip_idx<CHIP_NUM){
                        codeword_AMD[Chip_idx*8]=Chip_array[Chip_idx][BL*CHIP_WIDTH];
                        codeword_AMD[Chip_idx*8+1]=Chip_array[Chip_idx][BL*CHIP_WIDTH+1];
                        codeword_AMD[Chip_idx*8+2]=Chip_array[Chip_idx][BL*CHIP_WIDTH+2];
                        codeword_AMD[Chip_idx*8+3]=Chip_array[Chip_idx][BL*CHIP_WIDTH+3];
                        codeword_AMD[Chip_idx*8+4]=Chip_array[Chip_idx][BL*CHIP_WIDTH+4];
                        codeword_AMD[Chip_idx*8+5]=Chip_array[Chip_idx][BL*CHIP_WIDTH+5];
                        codeword_AMD[Chip_idx*8+6]=Chip_array[Chip_idx][BL*CHIP_WIDTH+6];
                        codeword_AMD[Chip_idx*8+7]=Chip_array[Chip_idx][BL*CHIP_WIDTH+7];
                        Chip_idx++;
                    }

                    // RECC implimentation
                    result_type_recc = error_correction_AMDCHIPKILL(codeword_AMD, error_chip_position); 

                    Chip_idx=0;
                    while(Chip_idx<CHIP_NUM){
                        Chip_array[Chip_idx][BL*CHIP_WIDTH]=codeword_AMD[Chip_idx*8];
                        Chip_array[Chip_idx][BL*CHIP_WIDTH+1]=codeword_AMD[Chip_idx*8+1];
                        Chip_array[Chip_idx][BL*CHIP_WIDTH+2]=codeword_AMD[Chip_idx*8+2];
                        Chip_array[Chip_idx][BL*CHIP_WIDTH+3]=codeword_AMD[Chip_idx*8+3];
                        Chip_array[Chip_idx][BL*CHIP_WIDTH+4]=codeword_AMD[Chip_idx*8+4];
                        Chip_array[Chip_idx][BL*CHIP_WIDTH+5]=codeword_AMD[Chip_idx*8+5];
                        Chip_array[Chip_idx][BL*CHIP_WIDTH+6]=codeword_AMD[Chip_idx*8+6];
                        Chip_array[Chip_idx][BL*CHIP_WIDTH+7]=codeword_AMD[Chip_idx*8+7];
                        Chip_idx++;
                    }

                    // SDC check
                    if(result_type_recc==CE || result_type_recc==NE){
                        int error_check=SDC_check(BL, Chip_array, recc_type);
                        if(error_check){
                            result_type_recc=SDC;
                        }
                    }
                    // DUE check
                    if(result_type_recc==DUE || final_result_1==DUE)
                        final_result_1=DUE;
                    else{ 
                        final_result_1 = (final_result_1>result_type_recc) ? final_result_1 : result_type_recc;
                    }

                    if(CONSERVATIVE_MODE)
                        isConservative = (error_chip_position.size()>1) ? 1 : isConservative;                    
                }
                
                if(final_result_1==NE || final_result_1==CE){
                    final_result_1 = (isConservative) ? DUE : CE;
                }
                
                // final result update
                final_result = final_result_1;
                break;



            case RECC_OFF:{
                int error_check;
                int BL=0; // trash value
                error_check = SDC_check(BL, Chip_array, recc_type);
                final_result = (error_check>0) ? SDC : CE;
                break;
            }
            default:
                break;
        }


        // 4-5. CE/DUE/SDC check

        CE_cnt   += (final_result==CE)  ? 1 : 0;
        DUE_cnt  += (final_result==DUE) ? 1 : 0;
        SDC_cnt  += (final_result==SDC) ? 1 : 0;

            
    }

    // final update
    fprintf(fp3,"\n===============\n");
    fprintf(fp3,"Runtime : %d\n",RUN_NUM);
    fprintf(fp3,"CE : %d\n",CE_cnt);
    fprintf(fp3,"DUE : %d\n",DUE_cnt);
    fprintf(fp3,"SDC : %d\n",SDC_cnt);
    fprintf(fp3,"\n===============\n");
    fflush(fp3);

    // final update 
    fprintf(fp3,"\n===============\n");
    fprintf(fp3,"Runtime : %d\n",RUN_NUM);
    fprintf(fp3,"CE : %.11f\n",(double)CE_cnt/(double)RUN_NUM);
    fprintf(fp3,"DUE : %.11f\n",(double)DUE_cnt/(double)RUN_NUM);
    fprintf(fp3,"SDC : %.11f\n",(double)SDC_cnt/(double)RUN_NUM);
    fprintf(fp3,"\n===============\n");
    fflush(fp3);

    fclose(fp3);


    return 0;
}
