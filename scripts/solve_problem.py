import sys
import argparse
import os
import json
import subprocess
from collections import defaultdict
sys.path.append('/sw/arc/centos7/gurobi/gurobi652/linux64/lib/python2.7')
sys.path.append('/sw/arc/centos7/gurobi/gurobi652/linux64/lib/python2.7/')
sys.path.append('/sw/arc/centos7/gurobi/gurobi652/linux64/lib/python2.7/gurobipy')

import gurobipy as grb

import make_mip

__author__ = "Byron Tasseff, Connor Riley"
__credits__ = ["Byron Tasseff", "Connor Riley"]
__license__ = "MIT"
__version__ = "0.0.4"
__maintainer__ = "Connor Riley"
__email__ = ""
__status__ = "Development"


def main(input_folder, output_folder, use_mixed_on_pure):
    # make sure the json files needed for input exist, if not write them
    write_inputs(use_mixed_on_pure)
    # initalize the files to store the results
    (result_files, output_path) = initialize_results_files(input_folder, output_folder, use_mixed_on_pure)
    results_store = {}
    results_store["num_solved"] = {}
    num_vars = 0
    results_store["num_cuts"] = defaultdict(list)
    #subdirs = get_immediate_subdirectories(input_folder)
    #for subdir in subdirs:
    #    full_subdir = input_folder + "/" + subdir
    onlyfiles = [f for f in os.listdir(input_folder) if os.path.isfile(os.path.join(input_folder, f))]
    for j, f in enumerate(onlyfiles):
        full_path = input_folder + "/" + f
        (objective, num_vars) = run_gurobi(full_path)
        instance_output_path = make_output_path(output_path, f)
        if not os.path.exists(instance_output_path):
            os.makedirs(instance_output_path)
        run_gomory(full_path, instance_output_path, use_mixed_on_pure)
        process_results(instance_output_path, result_files, objective,
            results_store, use_mixed_on_pure, j)
    write_results_store(results_store, output_path, num_vars)
    return 0

# TODO: finish this -- this runs, need to get average cuts
def write_results_store(results_store, folder, num_vars):
    for method in results_store["num_cuts"]:
        a = results_store["num_cuts"][method]
        avg = sum(a) / float(len(a))
        path = folder + "/" + "avg_cuts.csv"
        write_average_cuts(avg, path, method)
    num_solved_store = results_store["num_solved"]
    for solve_type in num_solved_store:
        path = get_bar_graph_path(solve_type, folder)
        write_bar_graph_data(path, num_solved_store, solve_type, num_vars)
    return 0


def write_average_cuts(avg, path, method):
    data_to_write = method + "," + str(avg) + "\n"
    if not os.path.exists(path):
        f = open(path, 'w')
        f.write('type,avg\n')
        f.close()
    f = open(path, "a")
    f.write(data_to_write)
    f.close()


def process_results(output_path, result_files, actual_objective, results_store, 
    use_mixed_on_pure, j):
    methods = ["naive", "lex", "rounds", "purging", "rounds_purging", 
    "lex_rounds", "lex_purging", "lex_rounds_purging"]
    last_lines = []
    if use_mixed_on_pure:
        methods.extend(["naive_mixed", "lex_mixed", "rounds_mixed", 
            "purging_mixed", "rounds_purging_mixed", "lex_rounds_mixed", 
            "lex_purging_mixed", "lex_rounds_purging_mixed"])
    for i, method in enumerate(methods):
        stats = get_stats(output_path + "/" + method + ".txt", actual_objective)
        num_cuts = int(stats[0])
        if method not in results_store["num_solved"].keys():
            results_store["num_solved"][method] = 0
        if stats[-1] == True : results_store["num_solved"][method] += 1
        if num_cuts < 10000:
            results_store["num_cuts"][method].append(num_cuts)
        last_lines.append(stats)
    for i, path in enumerate(result_files):
        write_data_line(path, last_lines[i], j)
    return 0


def make_output_path(output_path, file_name):
    split = file_name.split(".")
    return output_path + "/" + split[0]


def run_gurobi(file_path):
    rc = subprocess.check_call(["gurobi_cl", "ResultFile=" + "gurobi_solution.sol", file_path])
    with open("gurobi_solution.sol") as f:
        content = f.readlines()
    obj = content[0].split("=")[1]
    num_vars = len(content) - 1
    return (float(obj), num_vars)


def run_gomory(input_path, output_path, use_mixed_on_pure):
    rc = subprocess.check_call(["./RUN.sh", input_path, output_path])
    if use_mixed_on_pure:
        rc = subprocess.check_call(["./RUN_MIXED.sh", input_path, output_path])
    return 0


def get_stats(filepath, actual_objective):
    with open(filepath, "r") as f:
        lines = f.read().splitlines()
    max_det = 0
    for i,line in enumerate(lines):
        if i == 0:
            continue
        split_line = line.split(",")
        det = split_line[2]
        if abs(float(det)) > max_det:
            max_det = abs(float(det)) 
    last_line_split = lines[-1].split(",")
    obj = last_line_split[3]
    num_cuts = last_line_split[0]
    num_constr = last_line_split[1]
    achieved_solution = test_for_solution(num_cuts, obj, actual_objective)
    return(num_cuts, num_constr, max_det, obj, achieved_solution) 


def test_for_solution(num_cuts, obj, actual_objective):
    test = abs(float(obj) - float(actual_objective))
    if test < .0000001 and int(num_cuts) < 2500:
        return True
    return False


def write_bar_graph_data(bar_path, results_store, solve_method, num_vars):
    data_to_write = str(num_vars) + "," + str(results_store[solve_method]) + "\n"
    if not os.path.exists(bar_path):
        f = open(bar_path, 'w')
        f.write('num_starting_vars,num_finished\n')
        f.close()
    f = open(bar_path, "a")
    f.write(data_to_write)
    f.close()


def get_bar_graph_path(solve_method, folder):
    return folder + "/" + "bar_graph_" + solve_method + ".csv"


def initialize_results_files(input_folder, output_folder, use_mixed_on_pure):
    output_path = output_folder + "/" + input_folder.split("/")[-1]
    if not os.path.exists(output_path):
            os.makedirs(output_path)
    results_file_path_naive = output_path + "/results_naive.csv"
    results_file_path_lex = output_path + "/results_lex.csv"
    results_file_path_rounds = output_path + "/results_rounds.csv"
    results_file_path_purging = output_path + "/results_purging.csv"
    results_file_path_rounds_purging = output_path + "/results_rounds_purging.csv"
    results_file_path_lex_rounds = output_path + "/results_lex_rounds.csv"
    results_file_path_lex_purging = output_path + "/results_lex_purging.csv"
    results_file_path_lex_rounds_purging = output_path + "/results_lex_rounds_purging.csv" 
    filepaths = [results_file_path_naive, results_file_path_lex, 
        results_file_path_rounds, results_file_path_purging, 
        results_file_path_rounds_purging, results_file_path_lex_rounds,
        results_file_path_lex_purging, results_file_path_lex_rounds_purging]
    if use_mixed_on_pure:
        path1 = output_path + "/results_naive_mixed.csv"
        path2 = output_path + "/results_lex_mixed.csv"
        path3 = output_path + "/results_rounds_mixed.csv"
        path4 = output_path + "/results_purging_mixed.csv"
        path5 = output_path + "/results_rounds_purging_mixed.csv"
        path6 = output_path + "/results_lex_rounds.csv"
        path7 = output_path + "/results_lex_purging_mixed.csv"
        path8 = output_path + "/results_lex_rounds_purging_mixed.csv" 
        filepaths.extend([path1, path2, path3, path4, path5, path6, path7, path8])
    create_results_files(filepaths)
    return (filepaths, output_path)


def create_results_files(file_array):
    for fn in file_array:
        f = open(fn, 'w')
        f.write('problem_num, num_cuts,num_constr,det,obj,solved\n')
        f.close()
    return 0


def write_data_line(filepath, line, j):
    line_to_write = str(str(j) + "," + line[0]) + ","  + str(line[1]) + "," + str(
            line[2]) + "," + str(line[3] + "," + str(line[4]))+ "\n"
    f = open(filepath, 'a')
    f.write(line_to_write)
    f.close()


def write_inputs(use_mixed_on_pure):
    write_input(folder, False, False, False, False, "naive")
    write_input(folder, True, False, False, False, "rounds")
    write_input(folder, False, True, False, False, "lex")
    write_input(folder, False, False, True, False, "purging")
    write_input(folder, True, True, False, False, "lex_rounds")
    write_input(folder, False, True, True, False, "lex_purging")
    write_input(folder, True, False, True, False, "rounds_purging")
    write_input(folder, True, True, True, False, "lex_rounds_purging")
    if use_mixed_on_pure:
        write_input(folder, False, False, False, True, "naive_mixed")
        write_input(folder, True, False, False, True, "rounds_mixed")
        write_input(folder, False, True, False, True, "lex_mixed")
        write_input(folder, False, False, True, True, "purging_mixed")
        write_input(folder, True, True, False, True, "lex_rounds_mixed")
        write_input(folder, False, True, True, True, "lex_purging_mixed")
        write_input(folder, True, False, True, True, "rounds_purging_mixed")
        write_input(folder, True, True, True, True, "lex_rounds_purging_mixed")


def write_input(folder, rounds, lex, purging, mixed, name):
    d = {}
    d["parameters"] = {}
    d["parameters"]["maxCuts"] = 2500
    d["parameters"]["awayEpsilon"] = .01
    d["parameters"]["purgeEpsilon"] = 1.0e-9
    d["parameters"]["useRounds"] = rounds
    d["parameters"]["useLexicographic"] = lex
    d["parameters"]["useMixedCut"] = mixed
    d["parameters"]["usePurging"] = purging
    with open(name + '.json', 'w') as outfile:
        json.dump(d, outfile)
    return name


def read_last_line(filepath):
    with open(filepath, 'r') as f:
        lines = f.read().splitlines()
        last_line = lines[-1]
    return last_line


def get_immediate_subdirectories(a_dir):
    return [name for name in os.listdir(a_dir)
            if os.path.isdir(os.path.join(a_dir, name))]


if __name__ == "__main__":
    description = 'Generate random feasible problems, solve them and output results.'
    parser = argparse.ArgumentParser(description = description)
    parser.add_argument('input', type=str, nargs=1,
                        metavar = 'input',
                        help = 'input folder')
    parser.add_argument('folder', type=str, nargs=1,
                        metavar = 'folder',
                        help = 'folder to put statistics in')
    parser.add_argument('-m', '--m', action="store_true", default=False,
			help = 'use this option to use mixed cuts on pure problems')

    args = parser.parse_args()
    input_folder = args.input[0]
    folder = args.folder[0]
    use_mixed_on_pure = args.m
    main(input_folder, folder, use_mixed_on_pure)
