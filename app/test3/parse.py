#!/usr/bin/python

from __future__ import annotations
import os
import subprocess
import argparse
import pandas as pd
import matplotlib.pyplot as plt


def run_and_parse_program(num_runs: int) -> int:
    command = f"make NUM_RUNS={num_runs} run | grep -P 'INT-[0-9]+'"
    result = subprocess.run(command, stdout=subprocess.PIPE, text=True, shell=True, check=True)
    output = result.stdout

    # Extract the values after "ONE-INT: " and "NO-INT: "
    data = 0
    for line in output.splitlines():
        if f"INT-{num_runs}: " in line:
            data = int(line.split(f"INT-{num_runs}: ")[1].strip())

    return data


def draw(data: pd.DataFrame, bins: int):
    num_columns = len(data.columns) - 1
    _, axes = plt.subplots(nrows=num_columns, figsize=(24, 6*num_columns))
    for i, ax in enumerate(axes):
        ax: plt.Axes
        column = data.columns[i + 1]
        ax.hist(data[column], bins=bins)
        ax.set_title(f'Histogram for {column}')
        ax.set_xlabel(column)
        ax.set_ylabel('Frequency')
        ax.set_xlim(150000, 500000)
        ax.grid(True)
    plt.tight_layout()
    plt.savefig('histograms.png')


def main(iterations: int):
    csv_file = "parsed_data.csv"
    if os.path.isfile(csv_file):
        df = pd.read_csv(csv_file)
    else:
        df = pd.DataFrame()
        for num_runs in range(10):
            results: list[int] = []
            subprocess.run("make clean", stdout=subprocess.PIPE, text=True, shell=True, check=True)
            for _ in range(iterations):
                print(f"INT-{num_runs}: {_}/{iterations}", end='\r')
                data = run_and_parse_program(num_runs)
                results.append(data)

            df[f"INT-{num_runs}"] = results
        df.to_csv(csv_file)
    draw(df, iterations//10)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run and parse program output.")
    parser.add_argument("iterations", type=int, help="Number of times to run the program")
    args = parser.parse_args()

    main(args.iterations)
