#!/bin/bash
#SBATCH --job-name=lab2
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --mem-per-cpu=3000MB
#SBATCH -o lab2-%j.out
#SBATCH --partition=computes_thin

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv $SLURM_CPUS_PER_TASK
