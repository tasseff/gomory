import sys


def main(folder):
    naive_file = folder + "/results_naive.csv"
    with open(naive_file) as f:
        content = f.readlines()
    (avg_naive_gap, problems) = get_gap(content)
    lex_file = folder + "/results_lex.csv"
    with open(lex_file) as f:
        content = f.readlines()
    (avg_lex_gap, problems) = get_gap(content, problems)
    print(avg_naive_gap)
    print(avg_lex_gap)
    print(avg_naive_gap - avg_lex_gap)


def get_gap(content, problems = list(range(0, 100))):
    gap = 0
    new_problems = []
    for i, line in enumerate(content):
        line = line.split(",")
        print(line[5])
        print(type(line[5]))
        if line[5] == "False" and i in problems:
            new_problems.append(i)
            gap += float(line[6])
    avg_gap = gap / len(problems)
    return (avg_gap, new_problems)


if __name__ == "__main__":
    main(sys.argv[1])