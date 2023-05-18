#!/bin/bash
#SBATCH --job-name=lab2
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=3000MB
#SBATCH -o lab2-%j.out
#SBATCH --partition=computes_thin

echo "Sequential"
./seq rgb_video.yuv
