#!/usr/bin/python3


import argparse


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="Rascal Value File Creator",
        description="Generate the skeleton of a module for a C file implementing a Rascal type."
    )

    parser.add_argument('name')
    parser.add_argument('-o', '--object',    action='store_true')
    parser.add_argument('-s', '--sequence',  action='store_true')
    parser.add_argument('-t', '--traceable', action='store_true')
    parser.add_argument('-f', '--freeable',  action='store_true')
    parser.add_argument('-c', '--cloneable', action='store_true')
    parser.add_argument('-p', '--printable', action='store_true')

    args = parser.parse_args()
