#!/usr/bin/env python3

# pip3 install svg.path lxml

import sys
from lxml import etree

def get_rect_coordinates_and_label(element):
    factor = 2.95
    x = float(element.get("x", 0)) * factor + 2
    y = float(element.get("y", 0)) * factor
    width = float(element.get("width", 0)) * factor
    height = float(element.get("height", 0)) * factor
    label = element.get("{http://www.inkscape.org/namespaces/inkscape}label", "")
    return label, x, y, width, height

def get_circle_coordinates_and_label(element):
    factor = 2.95
    x = float(element.get("cx", 0)) * factor
    y = float(element.get("cy", 0)) * factor
    label = element.get("{http://www.inkscape.org/namespaces/inkscape}label", "")
    return label, x, y

def main():
    svg_file = "res/Beat707Simulation.svg"

    with open(svg_file, "r") as f:
        svg_tree = etree.parse(f)

    with open("src/gui.h", "w") as output_file:
        rects = svg_tree.findall(".//{http://www.w3.org/2000/svg}rect")
        for rect in rects:
            label, x, y, width, height = get_rect_coordinates_and_label(rect)
            if label != "":
                output_file.write(f"Rect {label} = {{{x}, {y}, {width}, {height}}};\n")
        circles = svg_tree.findall(".//{http://www.w3.org/2000/svg}circle")
        for circle in circles:
            label, x, y = get_circle_coordinates_and_label(circle)
            if label != "":
                output_file.write(f"Vec {label} = {{{x}, {y}}};\n")
    print("gui.h file generated.")

if __name__ == "__main__":
    main()
