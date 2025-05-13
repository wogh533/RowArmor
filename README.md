# RowArmor
All necessary code and dependencies are pre-installed in a Docker image.  
You can run the experiments without any manual setup.  
You can also download the compressed Docker image file directly from the following Google Drive link:

[Download Docker Image (rowarmor_image.tar.gz)](https://drive.google.com/uc?export=download&id=1-I07lHINtrVXqAi_Qrurnv20dJWff0k3)

---

## Quick Start (Using Docker)

To get started immediately:

```bash


```

## Running Experiments

We provide two scripts to automate the build and execution processes for our experiments.

### 1. Security Simulation

This simulation measures the attack success probability under various conditions.

To run:

    cd /root/RowArmor/sim
    .sim.sh <N> <BER>

- <N>: Number of attackers (≤ 128)
- <BER>: Bit error rate in percentage (e.g., 0.01, 0.1, 1, 2, 4, 10)

Example:

    ./sim.sh 64 0.1

The script will automatically:
- Build the source code in Security/src
- Navigate to Security/bin
- Execute the compiled binary with the provided parameters

### 2. Reliability Simulation

This simulation evaluates the reliability of the RowArmor scheme.

To run:

    cd /root/RowArmor/sim
    ./sim.sh

The script will automatically:
- Clean and build the source code in Reliability_simulation
- Run the run.py Python script

## Notes

- Ensure you are inside the /RowArmor directory when executing the scripts.
- If any additional Python packages are required when running run.py, they can be installed via:

    pip3 install <package_name>

- These experiments assume a basic Ubuntu environment with build-essential tools and Python 3 installed.

