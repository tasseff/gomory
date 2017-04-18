import sys

if __name__ == "__main__":
    main(sys.argv[1:])

def main(folder):
    file_list = ["bar_graph_naive.csv",                  
        "bar_graph_rounds.csv",                 
        "bar_graph_purging.csv",
        "bar_graph_lex.csv",                      
        "bar_graph_rounds_purging.csv",
        "bar_graph_lex_rounds.csv",             
        "bar_graph_lex_purging.csv",     
        "bar_graph_lex_rounds_purging.csv"]      
    solved_list = []
    for fname in file_list:
        with open(fname) as f:
            content = f.readlines()
        solved_list.append(content[1].split(",")[1])
    print(solved_list)
    return 0

