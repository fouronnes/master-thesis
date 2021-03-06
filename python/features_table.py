#!/usr/bin/env python3

import sys
import os
import os.path
import numpy as np

from features_common import match_angle, spatial_coverage

def make_features_table(root, subdirs, outlier_threshold, fraction_limit):
    """
    Make the features table for the given image pair directory
    and list of subdirs (from os.walk)
    """

    frac_lims = np.array([])
    first_coverages = np.array([])
    # For each algorithm directory
    for subdir in (os.path.join(root, s) for s in subdirs):
        # Load shape and match data
        shape = np.loadtxt(os.path.join(subdir, "shape.txt"))
        matches = np.loadtxt(os.path.join(subdir, "matches.txt"), comments="#")

        angles = match_angle(matches, shape)
        coverage = spatial_coverage(matches, shape)

        # Indice where outlier fraction > fraction_limit
        frac_lims = np.append(frac_lims, np.flatnonzero(np.cumsum(np.abs(angles) > outlier_threshold) / np.arange(1, angles.size+1) > fraction_limit)[0])
        # Spatial coverage at that indice
        first_coverages = np.append(first_coverages, coverage[frac_lims[-1]])

    # Sort by descending frac_lims
    sort_order = np.argsort(frac_lims)[::-1]
    subdirs = np.array(subdirs, dtype=str)

    return subdirs[sort_order], frac_lims[sort_order], first_coverages[sort_order]

def format_latex_table(algos, outs, covs):
    table = ""
    for algo, out, cov in zip(algos, outs, covs):
        table += "{0} & {1:0.0f} & {2:0.0f} \\\\\n".format(algo, out, 100*cov)
    return table

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: ./features_analysis.py path/to/results/features_analysis/")
        sys.exit(-1)

    # Find image pair directories
    path = sys.argv[1]
    for root, subdirs, files in os.walk(path):
        # For each image pair directory
        if len(subdirs) > 0 and "matches.txt" in os.listdir(os.path.join(root, subdirs[0])):
            # Produce the features table
            f = open(os.path.join(root, "features_table.tex"), "w")
            f.write(format_latex_table(*make_features_table(root, subdirs, 1.0, 0.05)))
