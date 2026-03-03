"""
Parser for LSystem files (L2D/L3D).
Will provide the following members after initialization with a valid file:
- alphabet (set of strings (length 1))
- draw (map of strings (length 1) to booleans)
- rules (map of strings (length 1) to strings (any length))
- initiator (string (any length))
- angle (float (degrees))
- startingAngle (float (degrees))
- iterations (integer)
"""


class LParser:
    def __init__(self, filename):
        with open(filename, "r") as f:
            drawing = False
            rules = False
            alphabet = False
            for element in f:
                element = element.strip().replace("{", ",")
                line_split = element.split(",")
                for line in line_split:
                    line = line.strip()
                    if line == "":
                        continue
                    if drawing:
                        if line[-1].strip() == "}":
                            drawing = False
                            line = line.strip(" }")
                        if len(line.split("->")) != 2:
                            drawing = False
                            continue
                        splits = line.split("->")
                        self.draw[splits[0].strip()] = True if splits[1].strip() == "1" else False
                        continue
                    if rules:
                        if line[-1].strip() == "}":
                            rules = False
                            line = line.strip(" }")
                        if len(line.split("->")) != 2:
                            rules = False
                            continue
                        splits = line.split("->")
                        self.rules[splits[0].strip()] = splits[1].strip().split('"')[1]
                        continue
                    if alphabet:
                        if line[-1].strip() == "}":
                            alphabet = False
                            self.alphabet.add(line.strip("{ }"))
                            continue
                        self.alphabet.add(line.strip("{ }"))
                        continue
                    splits = line.split("=")
                    if len(splits) != 2:
                        continue
                    if splits[0].strip() == "Alphabet":
                        alphabet = True
                        self.alphabet = set()
                    elif splits[0].strip() == "Draw":
                        drawing = True
                        self.draw = {}
                    elif splits[0].strip() == "Rules":
                        rules = True
                        self.rules = {}
                    elif splits[0].strip() == "Initiator":
                        self.initiator = splits[1].split('"')[1]
                    elif splits[0].strip() == "Angle":
                        self.angle = float(splits[1].strip())
                    elif splits[0].strip() == "StartingAngle":
                        self.startingAngle = float(splits[1].strip())
                    elif splits[0].strip() == "Iterations":
                        self.iterations = int(splits[1].strip())