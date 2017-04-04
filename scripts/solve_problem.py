import sys
import argparse
import os
import json
import subprocess

import gurobipy as grb

import make_mip


__author__ = "Byron Tasseff, Connor Riley"
__credits__ = ["Byron Tasseff", "Connor Riley"]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = "Connor Riley"
__email__ = ""
__status__ = "Development"


def main(folder):
    if not os.path.exists(folder):
        os.makedirs(folder)
    results_store = {}
    pure = True
    for i in range(2,13):
        j = 0
        results_file_path_naive = folder + "/results_naive_" + str(i) + ".csv"
        results_file_path_lex = folder + "/results_lex_" + str(i) + ".csv"
        results_file_path_rounds = folder + "/results_rounds_" + str(i) + ".csv"
        results_file_path_lex_rounds = folder + "/results_lexrounds_" + str(i) + ".csv"
        create_results_files([results_file_path_naive, results_file_path_rounds, 
            results_file_path_lex, results_file_path_lex_rounds])
        while(j < 100):
            new_folder_path = folder + "/ex_" + str(i) + "_" + str(j)
            if not os.path.exists(new_folder_path):
                os.makedirs(new_folder_path)
            (feasible, obj) = make_mip.make_mip(i, i, pure, 
                new_folder_path + "/generated_problem.lp")
            if feasible:
                j += 1
            write_temp_input(new_folder_path)
            run_gomory(new_folder_path)
            output_results(new_folder_path, results_file_path_naive,
                results_file_path_rounds, results_file_path_lex,
                results_file_path_lex_rounds)


def create_results_files(file_array):
    for fn in file_array:
        write_first_line(fn)
    return 0


def write_first_line(fn):
    file = open(fn, 'w')
    file.write('problem_num, num_cuts, det, obj, num_const\n')
    file.close()

def output_results(new_folder_path, results_file_path_naive, results_file_path_lex,
    results_file_path_rounds, results_file_path_lex_rounds):
    lex_last = get_stats(new_folder_path + "/lex.txt")
    round_lex_last = get_stats(new_folder_path + "/rounds_lex.txt")
    naive_last = get_stats(new_folder_path + "/naive.txt")
    rounds_last = get_stats(new_folder_path + "/rounds.txt")
    write_data_line(results_file_path_naive, naive_last)
    write_data_line(results_file_path_naive, naive_last)
    write_data_line(results_file_path_naive, naive_last)
    write_data_line(results_file_path_naive, naive_last)
    return 0


def write_data_line(filepath, line):
    line_to_write = line[0] + ","  + line[1] + "," + line[2] + "," + line[3] + "\n"
    file = open(filepath, 'a')
    f.write(line_to_write)
    f.close()


def get_stats(filepath):
    with open(filepath, "r") as f:
        lines = f.read().splitlines()
    max_det = 0
    for line in lines:
        split_line = line.split("\t")
        det = split_line[1]
        if abs(det) > max_det:
            max_det = abs(det)
    obj = lines[-1].split("\t")[2]
    num_cuts = lines[-1].split("-t")[0]
    num_constr = lines[-1].split("-t")[3]
    return(num_cuts, max_det, obj, num_constr) 


def read_last_line(filepath):
    with open(filepath, 'r') as f:
        lines = f.read().splitlines()
        last_line = lines[-1]
    return last_line


def run_gomory(folder):
    rc = subprocess.check_call(["./RUN.sh", folder])
    return 0


def write_temp_input(folder):
    d = {}
    d["parameters"] = {}
    d["parameters"]["model"] = "generated_problem.lp"
    d["parameters"]["solution"] = "solution.sol"
    with open(folder + '/temp.json', 'w') as outfile:
        json.dump(d, outfile)
    return 0


def run_gurobi(folder):
    rc = subprocess.check_call(["gurobi_cl", "ResultFile=" + folder + "/gurobi_solution.sol", folder+"/generated_problem.lp"])
    with open(folder + "/gurobi_solution.sol") as f:
        content = f.readlines()
    obj = content[0].split("=")[1]
    return float(obj)


if __name__ == "__main__":
    description = 'Generate random feasible mixed-integer programs.'
    parser = argparse.ArgumentParser(description = description)
    parser.add_argument('-f', '--folder', type=str, nargs=1,
                        metavar= 'folder',
                        help = 'output folder path')
   
    args = parser.parse_args()

    folder = 'temp'
    if args.folder:
        folder = args.folder[0]
    main(folder)