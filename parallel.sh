#!/bin/bash
#SBATCH --job-name=lab2
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --mem-per-cpu=4G
#SBATCH -o lab2-%j.out
#SBATCH --partition=computes_thin

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv
