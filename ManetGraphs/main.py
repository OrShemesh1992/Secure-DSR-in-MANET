# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
from random import random


def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    print_hi('PyCharm')

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
def openfile():
    x = []
    y = []
    file = open("wpbc.data", "r")
    file = list(file)
    random.shuffle(file)
    for lines in file:
        line = lines.split(',')
        if line[len(line) - 1] == '?\n':
           continue
        del line[2]
        del line[0]
        if line[0] == 'R':##append R 3 times to duplicate the data
            y.append(1)
            y.append(1)
            y.append(1)
            del line[0]
            x.append(list(map(float, line)))
            x.append(list(map(float, line)))
            x.append(list(map(float, line)))
        else:
            y.append(0)
            del line[0]
            x.append(list(map(float, line)))
    return x, y