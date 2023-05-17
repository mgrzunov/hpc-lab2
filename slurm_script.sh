#!/bin/bash
#SBATCH --job-name=lab2
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=4G
#SBATCH -o lab2.out
#SBATCH --partition=computes_thin

echo "Sequential"
./seq rgb_video.yuv

#SBATCH --cpus-per-task=2
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=3
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=4
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=6
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=8
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=12
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=16
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=20
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=24
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=32
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=40
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=48
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=56
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=64
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=96
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv

#SBATCH --cpus-per-task=128
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "Parallel on $SLURM_CPUS_PER_TASK CPUs"
./par rgb_video.yuv
