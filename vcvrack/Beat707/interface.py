#!/usr/bin/env python3

# pip3 install svg.path lxml

import sys
from lxml import etree

def get_rect_coordinates_and_label(rect_element):
    factor = 2.95
    x = float(rect_element.get("x", 0)) * factor + 2
    y = float(rect_element.get("y", 0)) * factor
    width = float(rect_element.get("width", 0)) * factor
    height = float(rect_element.get("height", 0)) * factor

    label = rect_element.get("{http://www.inkscape.org/namespaces/inkscape}label", "")

    return label, x, y, width, height

def main():
    svg_file = "res/Beat707Simulation.svg"

    with open(svg_file, "r") as f:
        svg_tree = etree.parse(f)

    rects = svg_tree.findall(".//{http://www.w3.org/2000/svg}rect")

    with open("src/gui.h", "w") as output_file:
        for rect in rects:
            label, x, y, width, height = get_rect_coordinates_and_label(rect)
            if label != "":
                output_file.write(f"Rect {label} = {{{x}, {y}, {width}, {height}}};\n")
    print("gui.h file generated.")

if __name__ == "__main__":
    main()
