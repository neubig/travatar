#!/usr/bin/python3

import sys
import argparse
from collections import defaultdict
from xml.sax.saxutils import escape

def fmt_error(fname, linenum, msg):
	print("invalid trace format: %s, line %d" % (fname, linenum), file=sys.stderr)
	print(msg, file=sys.stderr)


def split_skip_quot(string):
	r = []
	brk = False
	buff = ""
	for c in string:
		if c == " " and not brk:
			r.append(buff)
			buff = ""
			continue
		elif c == "\"":
			brk = not brk
		buff += c
	if buff != "":
		r.append(buff)
	return r


class Node():
	def __init__(self, id, desc):
		self.id = id
		self.desc = desc
		self.items = []


class TraceParser():
	def __init__(self):
		self.rules = []

	def addRule(self, start, end, tree, string):
		self.rules.append((start, end, tree, string))

	def __parse(self, i):
		start = self.rules[i][0]
		end = self.rules[i][1]
		tree = self.rules[i][2]
		string = self.rules[i][3]
		gentree = Node(i, tree)
		genstr = Node(i, string)
		vals = {}
		brk = 0
		j = i

		if tree == "UNK":
			if string == "\"<unk>\"":
				gentree.items = self.src[start:end]
				tree = ""
			else:
				tree = " ".join("%s:UNK" % s for s in split_skip_quot(string))

		for item in split_skip_quot(tree):
			if item == "(":
				brk += 1
			elif item == ")":
				brk -= 1
			elif item[0] != "\"" and item[-1] != "\"":
				spl = item.split(":")
				if len(spl) == 2:
					k = spl[0]
					j += 1
					result = self.__parse(j)
					if not result:
						return None
					j, t, s = result
					gentree.items.append(t)
					vals[k] = s
			elif len(item) >= 2 and item[0] == item[-1] == "\"":
				gentree.items.append(item[1:-1])
			else:
				print("syntax error at rule %d" % i, file=sys.stderr)
				return None

		if string == "\"<unk>\"":
			genstr.items = gentree.items[:]
			string = ""

		for item in split_skip_quot(string):
			if item[0] != "\"" and item[-1] != "\"":
				if item not in vals:
					print("undefined variable: %s, at rule %d" % (item, i), file=sys.stderr)
					return None
				genstr.items.append(vals[item])
			elif len(item) >= 2 and item[0] == item[-1] == "\"":
				genstr.items.append(item[1:-1])
			else:
				print("syntax error at rule %d" % i, file=sys.stderr)
				return None

		if brk != 0:
			print("syntax error at rule %d" % i, file=sys.stderr)
			return None

		return j, gentree, genstr

	def parse(self, src=None):
		if src:
			self.src = src.split()
		else:
			self.src = []
		self.rules.sort(key=lambda x: -x[1])
		self.rules.sort(key=lambda x: x[0])
		result = self.__parse(1)
		if result:
			i, gentree, genstr = result
		else:
			return None
		return gentree, genstr


class HTMLBuilder():
	def __init__(self):
		self.srclst = []
		self.trglst = []
		self.reflst = []
		self.html_header = """<!DOCTYPE html>
<html>
	<head>
		<meta charset="utf-8" />
		<title>Travatar analysis sheet</title>
		<link rel="stylesheet" type="text/css" href="./includes/analysis_sheet.css" />
		<script src="./includes/jquery-2.1.0.min.js"></script>
		<script src="./includes/highlight.js"></script>
		<script><!--
var desc = new Object();
var parentref = new Object();
		//--></script>
	</head>
	<body>
		<h1>Travatar analysis sheet</h1>
		<div class="rules"></div>"""
		self.html_footer = """	</body>
</html>"""
		self.html_script_header = """		<script><!--"""
		self.html_descset_content = """desc["%s"] = "%s";"""
		self.html_parentref_content = """parentref["%s"] = "%s";"""
		self.html_script_footer = """		//--></script>"""

		self.html_table1 = """		<table>
			<tr>
				<td>Sentence ID:</td>
				<td>%d</td>
			</tr>
			<tr>
				<td>Source:</td>
				<td>%s</td>
			</tr>"""
		self.html_table2 = """			<tr>
				<td>Target Ref:</td>
				<td>%s</td>
			</tr>"""
		self.html_table3 = """			<tr>
				<td>Target MT:</td>
				<td>%s</td>
			</tr>
		</table>"""

	def add_sentence(self, src, trg, ref=None):
		self.srclst.append(src)
		self.trglst.append(trg)
		self.reflst.append(ref)

	def __gen(self, sentid, structure, postfix, descset, parentref):
		descset["%d-%d-%s" % (sentid, structure.id, postfix)] = structure.desc
		html = "<span id='range-%d-%d-%s'>" % (sentid, structure.id, postfix)
		for i, item in enumerate(structure.items):
			if isinstance(item, str):
				html += "<span id='text-%d-%d-%d-%s' class='text'> %s </span>" % (sentid, structure.id, i, postfix, item)
			elif isinstance(item, Node):
				parentref["%d-%d" % (sentid, item.id)] = "%d-%d" % (sentid, structure.id)
				html += self.__gen(sentid, item, postfix, descset, parentref)
		html += "</span>"
		return html

	def output(self, fileprefix, size):
		if not size > 1:
			return

		htmlset = []
		for i, (src, trg) in enumerate(zip(self.srclst, self.trglst)):
			descset = {}
			parentref = {}
			srchtml = self.__gen(i, src, "src", descset, parentref)
			trghtml = self.__gen(i, trg, "trg", descset, parentref)
			htmlset.append((i, srchtml, trghtml, descset, parentref))

		fp = open("%s-0.html" % fileprefix, "w")
		print(self.html_header, file=fp)
		for (i, src, trg, descset, parentref), ref in zip(htmlset, self.reflst):
			print(self.html_script_header, file=fp)
			for k, v in descset.items():
				print(self.html_descset_content % (k, escape(v).replace("\\", "\\\\").replace("\"", "\\\"")), file=fp)
			for k, v in parentref.items():
				print(self.html_parentref_content % (k, v), file=fp)
			print(self.html_script_footer, file=fp)
			print(self.html_table1 % (i, src), file=fp)

			if ref:
				print(self.html_table2 % ref.strip(), file=fp)

			print(self.html_table3 % trg, file=fp)
			if (i+1) % size == 0:
				print(self.html_footer, file=fp)
				fp.close()
				fp = open("%s-%d.html" % (fileprefix, i+1), "w")
				print(self.html_header, file=fp)
		print(self.html_footer, file=fp)
		fp.close()


def main():

	parser = argparse.ArgumentParser()

	parser.add_argument("trace", help="Trace file generated from travatar")
	parser.add_argument("src", help="Sources of translations")
	parser.add_argument("out_prefix", help="Prefix of generated HTML file(s)")
	parser.add_argument("--ref", "-r", dest="ref", default=None, help="References of translations")
	parser.add_argument("--limit", "-l", dest="limit", help="Number of sentences contained in each HTML", default=0, type=int)

	args = parser.parse_args()

	tracefile = open(args.trace, "r")
	srcfile = open(args.src, "r")
	if args.ref:
		reffile = open(args.ref, "r")
	else:
		reffile = None

	trans_set = defaultdict(lambda : TraceParser())
	for i, line in enumerate(tracefile):
		spl = line.strip().split(" ||| ")
		if len(spl) < 5:
			fmt_error(args.trace, i+1, "each lines must contain 5 columns")
			return
		if not spl[0].strip().isdigit():
			fmt_error(args.trace, i+1, "1st column must be a number")
			return
		transid = int(spl[0])
		if len(spl[1]) == 0 or spl[1][0] != "[" or spl[1][-1] != "]":
			fmt_error(args.trace, i+1, "2nd column must be enclosed using square brackets: [ ]")
			return
		rangestr = spl[1][1:-1].split(",")
		if len(rangestr) != 2:
			fmt_error(args.trace, i+1, "2nd column must contain 2 items")
			return
		if not spl[0].strip().isdigit() or spl[1].strip().isdigit():
			fmt_error(args.trace, i+1, "both items of 2nd column must be a number")
			return
		start = int(rangestr[0])
		end = int(rangestr[1])
		trans_set[transid].addRule(start, end, spl[2], spl[3])

	tracefile.close()

	htmlbuilder = HTMLBuilder()
	for (i, trans), srcline in zip(sorted(trans_set.items(), key=lambda x: x[0]), srcfile):
		result = trans.parse(srcline)
		if not result:
			print("Error at sentence id %d" % i, file=sys.stderr)
			return
		if reffile:
			refline = reffile.readline()
		else:
			refline = None
		htmlbuilder.add_sentence(result[0], result[1], refline)

	if reffile:
		reffile.close()
	htmlbuilder.output(args.out_prefix, args.limit)

	return


if __name__ == "__main__":
	main()
