import os
import re
from dataclasses import dataclass
from datetime import datetime
from glob import glob
from os import mkdir
from pathlib import Path
from unittest import case

comment_parser = re.compile(r'//.*?\n')
system_include_parser = re.compile(r'\s*#\s*include\s<(.*)>.*')
local_include_parser = re.compile(r'\s*#\s*include\s\"(.*)\".*')
header_guard_parser = re.compile(r'\s*#\s*ifndef\s(.*)\s*.*')
header_guard_parser2 = re.compile(r'\s*#\s*define\s(.*)\s*.*')
header_guard_parser3 = re.compile(r'\s*#\s*endif\s//(.*)\s*.*')

@dataclass
class LineInfo:
    line_no: int
    original_line: str
    stripped_line: str
    is_comment: bool
    is_blank: bool

    @property
    def is_blank_or_comment(self):
        return self.is_blank or self.is_comment


def read_file(filename):
    with open(filename, mode='r', encoding='utf-8') as f:
        for i, line in enumerate(f):
            stripped_line = line.strip()
            yield LineInfo(i, line, stripped_line, stripped_line.startswith('//'), stripped_line == "")


def skip_blanks_and_comments(reader, line = None):
    if line is None:
        line = next(reader)
    while True:
        if not line.is_blank_or_comment:
            return line
        line = next(reader)


class Packer:
    def __init__(self, filename: str):
        self.filename = filename

        self.system_includes = []
        self.local_includes = []

        self.main_content_starts_at = None
        self.header_guard = None

    def __str__(self):
        return f"{self.filename} ({len(self.system_includes)} system include(s) and {len(self.local_includes)}) local include(s))"

    def read_prefix(self, is_header: bool):
        reader = read_file(self.filename)

        # Skip comment header block
        line = skip_blanks_and_comments(reader)

        if is_header:
            if m := header_guard_parser.match(line.stripped_line):
                self.header_guard = m.group(1)
                line = next(reader)
                if not header_guard_parser2.match(line.stripped_line):
                    raise Exception(f"Expected header guard ({self.header_guard}) at {self.filename}:{line.line_no}" )
                line = skip_blanks_and_comments(reader)

        try:
            while True:
                if not line.is_blank_or_comment:
                    if m := system_include_parser.match(line.stripped_line):
                        self.system_includes.append(m.group(1))
                    elif m := local_include_parser.match(line.stripped_line):
                        self.local_includes.append(m.group(1))
                    else:
                        break
                line = next(reader)

            self.main_content_starts_at = line.line_no

        except StopIteration:
            pass

        reader.close()

    def copy_to(self, f):
        if self.main_content_starts_at is None:
            # Don't copy files that only include their headers
            return

        f.write(f"\n// Packed from {self.filename}\n")
        reader = read_file(self.filename)

        # Skip to start (!TBD: should seek)
        for i in range(0, self.main_content_starts_at):
            next(reader)

        # line = next(reader) # !DBG
        guard_seen = False

        for line in reader:
            if self.header_guard:
                if m := header_guard_parser3.match(line.stripped_line):
                    guard_name = m.group(1)
                    if guard_name.strip() == self.header_guard:
                        if not guard_name.startswith(" "):
                            print(f"* Closing guard name is missing a space in {self.filename}:{line.line_no}")
                        guard_seen = True
                        break
            f.write(line.original_line)

        if self.header_guard and not guard_seen:
            raise Exception(f"Expected closing header guard ({self.header_guard}) in {self.filename}")

class LibPacker:
    def __init__(self):
        self.src_system_includes = set()
        self.header_system_includes = set()
        self.local_includes_seen = set()
        self.src_packers = []
        self.include_packers = []

    def parse_prefixes(self, from_dir: str):
        cpp_files = sorted(glob(os.path.join(from_dir, '**/*.cpp'), recursive=True))
        for cpp_file in cpp_files:
            packer = Packer(cpp_file)
            self.src_packers.append(packer)
            self._parse_prefix(packer, False)

    def _parse_prefix(self, packer: Packer, is_header: bool):
        packer.read_prefix(is_header)
        if is_header:
            self.header_system_includes = self.header_system_includes.union(set(packer.system_includes))
        else:
            self.src_system_includes = self.src_system_includes.union(set(packer.system_includes))
        for include_spec in packer.local_includes:
            match include_spec.split('/'):
                case [local_header]:
                    include_filename = os.path.join(os.path.dirname(packer.filename), local_header)
                case [project, local_filename]:
                    include_filename = os.path.join(project, "include", project, local_filename)
                case _:
                    include_filename = include_spec


            if include_filename not in self.local_includes_seen:
                self.local_includes_seen.add(include_filename)
                include_packer = Packer(include_filename)
                self._parse_prefix(include_packer, True)
                self.include_packers.append(include_packer)

    def write_packed_header(self, to_filename: str, guard_name: str, lib_name: str):
        if dirname := os.path.dirname(to_filename):
            Path(dirname).mkdir(parents=True, exist_ok=True)

        with open("LICENSE") as lf:
            license_lines = lf.readlines()

        with open(to_filename, "w") as f:
            f.write("/*\n")
            f.write(f" * {lib_name}\n")
            f.write(f" * Single header - Generated: {datetime.now()}\n")
            f.write(" * ----------------------------------------------------------\n")
            f.write(" * This file has been generated by merging together multiple source files.\n")
            f.write(" * Please don't edit it directly.\n")
            f.write(" *\n")
            f.write(" * Distributed under the MIT license. See text reproduced, below:\n")
            f.write(" *\n")
            for line in license_lines:
                f.write(f" * {line}")
            f.write(" */\n")

            f.write(f"#ifndef {guard_name}\n")
            f.write(f"#define {guard_name}\n\n")
            f.write("// Packed header\n\n")
            f.write("// System includes (for headers):\n")
            for include in self.header_system_includes:
                f.write(f"#include <{include}>\n")
            f.write("\n")

            f.write("// From header files:\n")
            for packer in self.include_packers:
                packer.copy_to(f)

            f.write("// From source files:\n")
            f.write("#ifdef CATCHKIT_IMPL\n\n")

            f.write("// System includes (for impl):\n")
            for include in self.header_system_includes:
                f.write(f"#include <{include}>\n")
            f.write("\n")

            for packer in self.src_packers:
                packer.copy_to(f)

            f.write("#endif // CATCHKIT_IMPL\n")

            f.write(f"\n#endif // {guard_name}\n")

        print(f"Written: {to_filename}")

lib_packer = LibPacker()
lib_packer.parse_prefixes(from_dir="catchkit/src")
lib_packer.write_packed_header(to_filename="catchkit/single_include/catchkit.h", guard_name="CATCHKIT_PACKED_H_INCLUDED", lib_name="CatchKit")
lib_packer.parse_prefixes(from_dir="catch23/src")
lib_packer.write_packed_header(to_filename="catch23/single_include/catch23.h", guard_name="CATCH23_PACKED_H_INCLUDED", lib_name="Catch23")
