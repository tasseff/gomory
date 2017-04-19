import sys

def main(folder, mixed):
    types = ["naive", "rounds", "purging", "lex", "rounds_purging",
        "lex_rounds", "lex_purging", "lex_rounds_purging"]
    file_list = ["bar_graph_naive.csv",                  
        "bar_graph_rounds.csv",                 
        "bar_graph_purging.csv",
        "bar_graph_lex.csv",                      
        "bar_graph_rounds_purging.csv",
        "bar_graph_lex_rounds.csv",             
        "bar_graph_lex_purging.csv",     
        "bar_graph_lex_rounds_purging.csv"]
    pure_list = get_lines(folder, file_list)
    
    if mixed:
        file_list = ["bar_graph_naive_mixed.csv",                  
        "bar_graph_rounds_mixed.csv",                 
        "bar_graph_purging_mixed.csv",
        "bar_graph_lex_mixed.csv",                      
        "bar_graph_rounds_purging_mixed.csv",
        "bar_graph_lex_rounds_mixed.csv",             
        "bar_graph_lex_purging_mixed.csv",     
        "bar_graph_lex_rounds_purging_mixed.csv"]
        mixed_list = get_lines(folder, file_list)
    
    cuts_file = folder + "/" + "avg_cuts.csv"
    with open(cuts_file) as f:
        content = f.readlines()
    d = {}
    for i, line in enumerate(content):
        if i == 0:
            continue
        linesplit = line.split(",")
        d[linesplit[0]] = float(linesplit[1].rstrip())
    pure_cuts_list = get_avg_cuts(d, types)
    
    if mixed:
        types = ["naive_mixed", "rounds_mixed", "purging_mixed", "lex_mixed", 
        "rounds_purging_mixed", "lex_rounds_mixed", "lex_purging_mixed", 
        "lex_rounds_purging_mixed"]
        mixed_cuts_list = get_avg_cuts(d, types)
    print_pure = []
    for i, el in enumerate(pure_list):
        print_pure.append(el)
        print_pure.append(pure_cuts_list[i])
    print(print_pure)
    if mixed:
        mixed_list = []
        for i, el in enumerate(mixed_list):
            print_mixed.append(el)
            print_mixed.append(pure_cuts_list[i])
        print(print_mixed)
    return 0


def get_avg_cuts(d, types):
    avg_cuts = []
    for t in types:
        avg_cuts.append(d[t])
    return avg_cuts)


def get_lines(folder, file_list):     
    solved_list = []
    for name in file_list:
	fname = str(folder) + "/" + str(name)
        with open(fname) as f:
            content = f.readlines()
        num = content[1].split(",")[1]
	num = num.rstrip()
	num = int(num) + 1
	solved_list.append(num)
    return solved_list


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])

