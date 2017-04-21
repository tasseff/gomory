import sys


def main(folder, mixed):
    if int(mixed) == 1:
        methods = ["rounds_mixed", "purging_mixed", "lex_mixed", "rounds_purging_mixed",
			"lex_rounds_mixed", "lex_purging_mixed", "lex_rounds_purging_mixed"]
    else:
    	methods = ["rounds", "purging", "lex", "rounds_purging", 
	    "lex_rounds", "lex_purging", "lex_rounds_purging"]
    avgs = []
    for method in methods:
        avgs.append(get_avg_for_method(folder, method, mixed))
    print(avgs)


def get_avg_for_method(parent, method, mixed):
    print(method)
    fname = "/results_" + method + ".csv"
    total_avg = 0
    total_num_problems = 0
    for i in range(2,11):
	folder = parent + "/"  + "n_" + str(i)
        (avg_gap, num_problems) = get_avg_gap(folder, fname, mixed)
        total_num_problems += num_problems
        total_avg += avg_gap * num_problems
        print(avg_gap, num_problems)
    return(total_avg / total_num_problems)


def get_avg_gap(folder, fname, mixed):
    naive_file = folder + "/results_naive.csv"
    if int(mixed) == 1:
        naive_file = folder + "/results_naive_mixed.csv"
    with open(naive_file) as f:
        content = f.readlines()
    (avg_naive_gap, problems) = get_gap(content)
    (naive_det_list, _) = get_det(content)
    lex_file = folder + fname
    with open(lex_file) as f:
        content = f.readlines()
    (avg_lex_gap, problems) = get_gap(content, problems, True)
    (lex_det_list, _) = get_det(content, problems, True)
    better = 0
    assert(len(naive_det_list) == len(lex_det_list))
    for i, el in enumerate(naive_det_list):
	if el > lex_det_list[i]:
	    better += 1
    return (avg_naive_gap - avg_lex_gap, len(naive_det_list))


def get_det(content, p = list(range(0,100)), b = False):
    det = []
    new_problems = []
    for i, line in enumerate(content):
        line = line.split(",")
        if (line[5] == "False" or b) and i in p:
	    new_problems.append(i)
	    det.append(abs(float(line[3])))
    return (det, new_problems)


def get_gap(content, p = list(range(0, 100)), b = False):
    gap = 0
    new_problems = []
    for i, line in enumerate(content):
        line = line.split(",")
        if (line[5] == "False" or b) and i in p:
            new_problems.append(i)
            gap += float(line[6])/ (- float(line[6]) + float(line[4]))
    avg_gap = gap / len(new_problems)
    return (avg_gap, new_problems)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
