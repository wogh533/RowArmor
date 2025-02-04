# GoM_Error_Simulation

# Overview
We provide reliability experiment code for QPC, OPC, and AMD in GoM.

# DIMM configuration (per-sub channel)
- DDR5 ECC-DIMM
- Num of rank: 1
- Beat length: 40 bit
- Burst length: 16
- Num of data chips: 8
- Num of parity chips: 2
- Num of DQ: 4 (x4 chip)

# ECC configuration
- OD-ECC (2 options): No OD-ECC, (136, 128) Hamming SEC code **[1]**
- RL-ECC (3 options): No RL-ECC, [10, 8] Chipkill (using Reed-Solomon code), QPC, OPC

# Error Scenario configuration
- SE(SBE): per-chip Single Bit Error
- DE(DBE): per-chip Double Bit Error
- CHIPKILL(SCE): Single Chip Error (All Random)
- Combination of Error Patterns

# Getting Started
- $ make clean
- $ make
- $ python run.py


# Additional Information
- NE: no error
- CE: detected and corrected error
- DUE: detected but uncorrected error
- SDC: Silent Data Corruption
- The codeword is in the default all-zero state (No-error state). In other words, the original message is all-zero
- Thus, there's no need to encode
- Reason: Because it's a Linear code, the same syndrome appears regardless of 1->0 or 0->1 error at the same location

# References
- **[1]** Hamming, Richard W. "Error detecting and error correcting codes." The Bell system technical journal 29.2 (1950): 147-160.
