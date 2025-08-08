#!/usr/bin/env python3
##
# convert_config_section.py
#
# Vega Strike - Space Simulation, Combat and Trading
# Copyright (C) 2001-2025 The Vega Strike Contributors:
# Project creator: Daniel Horn
# Original development team: As listed in the AUTHORS file
# Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
#
#
# https://github.com/vegastrike/Vega-Strike-Engine-Source
#
# This file is part of Vega Strike.
#
# Vega Strike is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vega Strike is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
#

import argparse
import sys
from io import TextIOWrapper
import re
from pathlib import Path
from re import RegexFlag


class VegaSetting:
    name: str
    type: str
    replacement_expression: str


class LineReplacement:
    line_number: int
    starting_text: str|None
    ending_text: str|None


class ParseKey:
    substitution: str
    pattern: re.Pattern
    leave_as_is: str


options_h_path: Path = Path('libraries/root_generic/options.h').resolve()
options_cpp_path: Path = Path('libraries/root_generic/options.cpp').resolve()

regex_flags_single_line: int = RegexFlag.UNICODE
regex_flags_single_line_case_insensitive: int = RegexFlag.UNICODE + RegexFlag.IGNORECASE
regex_flags_multiline: int = RegexFlag.MULTILINE + RegexFlag.UNICODE

parse_keys: list[ParseKey] = []
options_section_header_parse_key_1 = ParseKey()
options_section_header_parse_key_1.pattern = re.compile(r'^\s*/\*\s*([A-Za-z0-9 ]*?)\s*Options\s+\*/\s*$', regex_flags_single_line_case_insensitive)
options_section_header_parse_key_1.substitution = ''
options_section_header_parse_key_1.leave_as_is = r'\0'
parse_keys.append(options_section_header_parse_key_1)
options_section_header_parse_key_2 = ParseKey()
options_section_header_parse_key_2.pattern = re.compile(r'^\s*//\s*([A-Za-z0-9 ]*?)\s*Options\s*//\s*$', regex_flags_single_line_case_insensitive)
options_section_header_parse_key_2.substitution = ''
options_section_header_parse_key_2.leave_as_is = r'\0'
parse_keys.append(options_section_header_parse_key_2)

comment_parse_key_1 = ParseKey()
comment_parse_key_1.pattern = re.compile(r'^\s*/\*.*?\*/ *\r?\n', regex_flags_multiline)
comment_parse_key_1.substitution = ''
comment_parse_key_1.leave_as_is = r'\0\n'
parse_keys.append(comment_parse_key_1)
comment_parse_key_2 = ParseKey()
comment_parse_key_2.pattern = re.compile(r'^\s*/{2}[^/]*/{2} *$', regex_flags_single_line)
comment_parse_key_2.substitution = ''
comment_parse_key_2.leave_as_is = r'\0'
parse_keys.append(comment_parse_key_2)

trailing_comment_parse_key_1 = ParseKey()
trailing_comment_parse_key_1.pattern = re.compile(r'^\s*((?:\S+\s+)*)//(.*)$', regex_flags_single_line)
trailing_comment_parse_key_1.substitution = r'\1'
trailing_comment_parse_key_1.leave_as_is = r'\2'
# parse_keys.append(trailing_comment_parse_key_1)
trailing_comment_parse_key_2 = ParseKey()
trailing_comment_parse_key_2.pattern = re.compile(r'^\s*((?:\S+\s+)*)/\*(.*?)\*/$', regex_flags_single_line)
trailing_comment_parse_key_2.substitution = r'\1'
trailing_comment_parse_key_2.leave_as_is = r'\2'
# parse_keys.append(trailing_comment_parse_key_2)

setting_declaration_namespaced_parse_key = ParseKey()
setting_declaration_namespaced_parse_key.pattern = re.compile(r'^(?P<initial_space> {4})(?P<static_or_const>static|const)\s*('
                                                              r'?P<type>std::string)\s+(?P<name>[a-z0-9_]+)(?P<init_braces>\{'
                                                              r'})?\s*; *\r?\n', regex_flags_multiline)
setting_declaration_namespaced_parse_key.substitution = r'\g<initial_space>\g<static_or_const> \g<type> \g<name>\g<init_braces>;\n'
setting_declaration_namespaced_parse_key.leave_as_is = ''
setting_declaration_namespaced_parse_key_1a = ParseKey()
setting_declaration_namespaced_parse_key_1a.pattern = re.compile(r'^(?P<initial_space> {4})(?P<type>std::string)\s+('
                                                                 r'?P<name>[a-z0-9_]+)(?P<init_braces>\{})?\s*; *\r?\n',
                                                                 regex_flags_multiline)
setting_declaration_namespaced_parse_key_1a.substitution = r'\g<initial_space>\g<type> \g<name>\g<init_braces>;\n'
setting_declaration_namespaced_parse_key_1a.leave_as_is = ''
setting_declaration_namespaced_parse_key_1b = ParseKey()
setting_declaration_namespaced_parse_key_1b.pattern = re.compile(r'^(?P<initial_space> {4})(?P<type>std::string)\s+('
                                                                 r'?P<name>[a-z0-9_]+)\s*=\s*\"(?P<default_value>w+)\"\s*; *\r?\n',
                                                                 regex_flags_multiline)
setting_declaration_namespaced_parse_key_1b.substitution = r'\g<initial_space>\g<type> \g<name> = \g<default_value>;\n'
setting_declaration_namespaced_parse_key_1b.leave_as_is = ''
setting_declaration_namespaced_parse_key_1c = ParseKey()
setting_declaration_namespaced_parse_key_1c.pattern = re.compile(r'^(?P<initial_space> {4})(?P<static_or_const>static|const)\s*(?P<type>std::string)\s+(?P<name>[a-z0-9_]+)\s*=\s*\"(?P<default_value>w+)\"\s*; *\r?\n', regex_flags_multiline)
setting_declaration_namespaced_parse_key_1c.substitution = 'r\g<initial_space>\g<static_or_const> \g<type> \g<name> = \g<default_value>;\n'
setting_declaration_namespaced_parse_key_1c.leave_as_is = ''
setting_declaration_simple_parse_key = ParseKey()
setting_declaration_simple_parse_key.pattern = re.compile(r'^(?P<initial_space> {4})(?P<static_or_const>static|const)\s*('
                                                          r'?P<type>bool|double|float|int)\s+(?P<name>[a-z0-9_]+)(?P<init_braces>\{})?\s*; '
                                                          r'*\r?\n', regex_flags_multiline)
setting_declaration_simple_parse_key.substitution = r'\g<initial_space>\g<static_or_const> \g<type> \g<name>\g<init_braces>;\n'
setting_declaration_simple_parse_key.leave_as_is = ''
setting_declaration_simple_parse_key_1a = ParseKey()
setting_declaration_simple_parse_key_1a.pattern = re.compile(r'^(?P<initial_space> {4})('
                                                             r'?P<type>bool|double|float|int)\s+(?P<name>[a-z0-9_]+)('
                                                             r'?P<init_braces>\{})?\s*; *\r?\n', regex_flags_multiline)
setting_declaration_simple_parse_key_1a.substitution = r'\g<initial_space>\g<type> \g<name>\g<init_braces>;\n'
setting_declaration_simple_parse_key_1a.leave_as_is = ''
setting_declaration_simple_parse_key_1b = ParseKey()
setting_declaration_simple_parse_key_1b.pattern = re.compile(r'^((?P<initial_space> {4})(?P<type>bool|double|float|int)\s+(?P<name>[a-z0-9_]+))\s*=\s*\"(?P<default_value>w+)\"\s*; *\r?\n', regex_flags_multiline)
setting_declaration_simple_parse_key_1b.substitution = r'^\g<initial_space>\g<type> \g<name> = \g<default_value>;\n'
setting_declaration_simple_parse_key_1b.leave_as_is = ''
setting_declaration_simple_parse_key_1c = ParseKey()
setting_declaration_simple_parse_key_1c.pattern = re.compile(r'^(?P<initial_space> {4})(?P<static_or_const>static|const)\s*('
                                                             r'?P<type>bool|double|float|int)\s+(?P<name>[a-z0-9_]+)\s*=\s*\"(?P<default_value>w+)\"\s*; *\r?\n')
setting_declaration_simple_parse_key_1c.substitution = r'^\g<initial_space>\g<static_or_const> \g<type> \g<name> = \g<default_value>;\n'
setting_declaration_simple_parse_key_1c.leave_as_is = ''

setting_declaration_parse_keys: list[ParseKey] = [setting_declaration_namespaced_parse_key,
                                                  setting_declaration_namespaced_parse_key_1a,
                                                  setting_declaration_namespaced_parse_key_1b,
                                                  setting_declaration_namespaced_parse_key_1c,
                                                  setting_declaration_simple_parse_key,
                                                  setting_declaration_simple_parse_key_1a,
                                                  setting_declaration_simple_parse_key_1b,
                                                  setting_declaration_simple_parse_key_1c]

equals_configuration_parse_key_1 = ParseKey()
equals_configuration_parse_key_1.pattern = re.compile(r'^(?P<initial_space> *)(?P<name>\w+)\s*=\s*('
                                                      r'?P<config_expr>configuration\(\)->\w+(?:\.\w+)+); *\r?\n',
                                                      regex_flags_multiline)
equals_configuration_parse_key_1.substitution = r'\g<initial_space>\g<name> = \g<config_expr>;\n'
equals_configuration_parse_key_1.leave_as_is = ''
parse_keys.append(equals_configuration_parse_key_1)
equals_configuration_parse_key_2 = ParseKey()
equals_configuration_parse_key_2.pattern = re.compile(r'^(?P<initial_space> *)(?P<name>\w+)\s*=\s*(?P<config_expr>configuration\(\)->\w+(?:\.\w+)+);$',
                                                      regex_flags_single_line)
equals_configuration_parse_key_2.substitution = r'\g<initial_space>\g<name> = \g<config_expr>;'
equals_configuration_parse_key_2.leave_as_is = ''
parse_keys.append(equals_configuration_parse_key_2)

equals_configuration_parse_keys: list[ParseKey] = [equals_configuration_parse_key_1, equals_configuration_parse_key_2]
equals_configuration_parse_keys_single_line: list[ParseKey] = [equals_configuration_parse_key_2]
equals_configuration_parse_keys_multi_line: list[ParseKey] = [equals_configuration_parse_key_1]


def build_parse_keys(section_name: str) -> list[ParseKey]:
    parse_key_1 = ParseKey()
    parse_key_1.pattern = re.compile(
        r'^(?P<initial_space> {4})static\s+(?P<type>bool|double|float|int|std::string)\s+(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<subsection>[a-z0-9_]+)"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_1.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<subsection>.\g<name2>;\n'
    parse_key_1.leave_as_is = ''
    parse_keys.append(parse_key_1)
    parse_key_1a = ParseKey()
    parse_key_1a.pattern = re.compile(
        r'^(?P<initial_space> {4})static\s+(?P<type>bool|double|float|int|std::string)\s+(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<subsection>[a-z0-9_]+)"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_1a.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<subsection>.\g<name2>'
    parse_key_1a.leave_as_is = ''
    parse_keys.append(parse_key_1a)
    parse_key_2 = ParseKey()
    parse_key_2.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<type>bool|double|float|int|std::string)\s+=(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<subsection>[a-z0-9_]+)"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_2.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<subsection>.\g<name2>;\n'
    parse_key_2.leave_as_is = ''
    parse_keys.append(parse_key_2)
    parse_key_2a = ParseKey()
    parse_key_2a.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<type>bool|double|float|int|std::string)\s+=(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<subsection>[a-z0-9_]+)"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_2a.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<subsection>.\g<name2>'
    parse_key_2a.leave_as_is = ''
    parse_keys.append(parse_key_2a)
    parse_key_3 = ParseKey()
    parse_key_3.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<name>[a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\('
        r'\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<subsection>w+)"s*,s*"(?P<name2>w+)"\s*,'
                                                               r'\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_3.substitution = r'\g<initial_space>\g<name> = configuration()->' + section_name + r'.\g<subsection>.\g<name2>;\n'
    parse_key_3.leave_as_is = ''
    parse_keys.append(parse_key_3)
    parse_key_3a = ParseKey()
    parse_key_3a.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<name>[a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\('
        r'\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<subsection>w+)"s*,s*"(?P<name2>w+)"\s*,'
                                                               r'\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_3a.substitution = r'\g<initial_space>\g<name> = configuration()->' + section_name + r'.\g<subsection>.\g<name2>'
    parse_key_3a.leave_as_is = ''
    parse_keys.append(parse_key_3a)
    parse_key_4 = ParseKey()
    parse_key_4.pattern = re.compile(
        r'\bXMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"('
                                                                                                  r'?P<subsection>w+)"s*,s*"(?P<name2>w+)"\s*,\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_4.substitution = r'configuration()->' + section_name + r'.\g<subsection>.\g<name2>;\n'
    parse_key_4.leave_as_is = ''
    parse_keys.append(parse_key_4)
    parse_key_4a = ParseKey()
    parse_key_4a.pattern = re.compile(
        r'\bXMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"('
                                                                                                  r'?P<subsection>w+)"s*,s*"(?P<name2>w+)"\s*,\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_4a.substitution = r'configuration()->' + section_name + r'.\g<subsection>.\g<name2>'
    parse_key_4a.leave_as_is = ''
    parse_keys.append(parse_key_4a)
    parse_key_5 = ParseKey()
    parse_key_5.pattern = re.compile(
        r'\bparse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<subsection>['
                                                                                      r'a-z0-9_]+)"\s*,'
                                                                                      r'\s*"(?P<name2>['
                                                                                      r'a-z0-9_]+)"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_5.substitution = r'configuration()->' + section_name + r'.\g<subsection>.\g<name2>;\n'
    parse_key_5.leave_as_is = ''
    parse_keys.append(parse_key_5)
    parse_key_5a = ParseKey()
    parse_key_5a.pattern = re.compile(
        r'\bparse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<subsection>['
                                                                                      r'a-z0-9_]+)"\s*,'
                                                                                      r'\s*"(?P<name2>['
                                                                                      r'a-z0-9_]+)"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_5a.substitution = r'configuration()->' + section_name + r'.\g<subsection>.\g<name2>;'
    parse_key_5a.leave_as_is = ''
    parse_keys.append(parse_key_5a)
    parse_key_6 = ParseKey()
    parse_key_6.pattern = re.compile(
        r'\bvs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<subsection>w+)"s*,s*"(?P<name2>w+)"\s*,'
                                                              r'\s*"[^"]*"\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_6.substitution = r'configuration()->' + section_name + r'.\g<subsection>.\g<name2>;\n'
    parse_key_6.leave_as_is = ''
    parse_keys.append(parse_key_6)
    parse_key_6a = ParseKey()
    parse_key_6a.pattern = re.compile(
        r'\bvs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<subsection>w+)"s*,s*"(?P<name2>w+)"\s*,'
                                                              r'\s*"[^"]*"\s*\)',
        regex_flags_multiline)
    parse_key_6a.substitution = r'configuration()->' + section_name + r'.\g<subsection>.\g<name2>'
    parse_key_6a.leave_as_is = ''
    parse_keys.append(parse_key_6a)
    parse_key_7 = ParseKey()
    parse_key_7.pattern = re.compile(
        r'^(?P<initial_space> *)static\s+(?P<type>bool|double|float|int|std::string)\s+(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_7.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<name2>;\n'
    parse_key_7.leave_as_is = ''
    parse_keys.append(parse_key_7)
    parse_key_7a = ParseKey()
    parse_key_7a.pattern = re.compile(
        r'^(?P<initial_space> *)static\s+(?P<type>bool|double|float|int|std::string)\s+(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_7a.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<name2>'
    parse_key_7a.leave_as_is = ''
    parse_keys.append(parse_key_7a)
    parse_key_8 = ParseKey()
    parse_key_8.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<type>bool|double|float|int|std::string)\s+=(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_8.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<name2>;\n'
    parse_key_8.leave_as_is = ''
    parse_keys.append(parse_key_8)
    parse_key_8a = ParseKey()
    parse_key_8a.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<type>bool|double|float|int|std::string)\s+=(?P<name>['
        r'a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name +
        r'"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_8a.substitution = r'\g<initial_space>const \g<type> \g<name> = configuration()->' + section_name + r'.\g<name2>'
    parse_key_8a.leave_as_is = ''
    parse_keys.append(parse_key_8a)
    parse_key_9 = ParseKey()
    parse_key_9.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<name>[a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\('
        r'\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,'
                                                               r'\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_9.substitution = r'\g<initial_space>\g<name> = configuration()->' + section_name + r'\g<name2>;\n'
    parse_key_9.leave_as_is = ''
    parse_keys.append(parse_key_9)
    parse_key_9a = ParseKey()
    parse_key_9a.pattern = re.compile(
        r'^(?P<initial_space> *)(?P<name>[a-z0-9_]+)\s+=\s*XMLSupport::parse_[a-z0-9_]+\s*\('
        r'\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,'
                                                               r'\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_9a.substitution = r'\g<initial_space>\g<name> = configuration()->' + section_name + r'\g<name2>'
    parse_key_9a.leave_as_is = ''
    parse_keys.append(parse_key_9a)
    parse_key_10 = ParseKey()
    parse_key_10.pattern = re.compile(
        r'\bXMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"('
                                                                                                  r'?P<name2>['
                                                                                                  r'a-z0-9_]+)"\s*,'
                                                                                                  r'\s*"['
                                                                                                  r'^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_10.substitution = r'configuration()->' + section_name + r'.\g<name2>;\n'
    parse_key_10.leave_as_is = ''
    parse_keys.append(parse_key_10)
    parse_key_10a = ParseKey()
    parse_key_10a.pattern = re.compile(
        r'\bXMLSupport::parse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"('
                                                                                                  r'?P<name2>['
                                                                                                  r'a-z0-9_]+)"\s*,'
                                                                                                  r'\s*"['
                                                                                                  r'^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_10a.substitution = r'configuration()->' + section_name + r'.\g<name2>'
    parse_key_10a.leave_as_is = ''
    parse_keys.append(parse_key_10a)
    parse_key_11 = ParseKey()
    parse_key_11.pattern = re.compile(
        r'\bparse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<name2>['
                                                                                      r'a-z0-9_]+)"\s*,'
                                                                                      r'\s*"[^"]*"\s*\)\s*\)\s*;\r?\n',
        regex_flags_multiline)
    parse_key_11.substitution = r'configuration()->' + section_name + r'.\g<name2>;\n'
    parse_key_11.leave_as_is = ''
    parse_keys.append(parse_key_11)
    parse_key_11a = ParseKey()
    parse_key_11a.pattern = re.compile(
        r'\bparse_[a-z0-9_]+\s*\(\s*vs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<name2>['
                                                                                      r'a-z0-9_]+)"\s*,'
                                                                                      r'\s*"[^"]*"\s*\)\s*\)',
        regex_flags_multiline)
    parse_key_11a.substitution = r'configuration()->' + section_name + r'.\g<name2>'
    parse_key_11a.leave_as_is = ''
    parse_keys.append(parse_key_11a)
    parse_key_12 = ParseKey()
    parse_key_12.pattern = re.compile(
        r'\bvs_config->getVariable\s*\(\s*"' + section_name + r'"\s*,\s*"(?P<name2>[a-z0-9_]+)"\s*,\s*"[^"]*"\s*\)',
        regex_flags_multiline)
    parse_key_12.substitution = r'configuration()->' + section_name + r'.\g<name2>;'
    parse_key_12.leave_as_is = ''
    parse_keys.append(parse_key_12)
    return parse_keys


def build_game_options_parse_keys(parse_keys_list: list[ParseKey], config_items: dict[str, VegaSetting]) -> list[ParseKey]:
    for each_key in config_items.keys():
        this_setting: VegaSetting = config_items[each_key]
        if this_setting:
            new_parse_key = ParseKey()
            new_parse_key.pattern = re.compile(r'\bgame_options\(\)->' + this_setting.name + r'\b', regex_flags_multiline)
            new_parse_key.substitution = this_setting.replacement_expression
            new_parse_key.leave_as_is = ''
            if new_parse_key.pattern and new_parse_key.substitution:
                parse_keys_list.append(new_parse_key)

    return parse_keys


def is_comment(line_of_input: str) -> bool:
    if comment_parse_key_1.pattern.fullmatch(line_of_input):
        return True
    elif comment_parse_key_2.pattern.fullmatch(line_of_input):
        return True
    else:
        return False


def is_options_section_header_comment(line_of_input: str) -> bool:
    if options_section_header_parse_key_1.pattern.fullmatch(line_of_input):
        return True
    elif options_section_header_parse_key_2.pattern.fullmatch(line_of_input):
        return True
    else:
        return False


def parse_cmdline_arguments() -> str:
    parser = argparse.ArgumentParser()
    parser.add_argument('section', help='which config section to convert. Case insensitive')
    args = parser.parse_args()
    return args.section


def skip_to_section_header(file_reader: TextIOWrapper, section_name: str) -> bool:
    preamble: str = ''
    section_name_upper = section_name.upper()
    for line in file_reader:
        match1: re.Match|None = options_section_header_parse_key_1.pattern.fullmatch(line)
        if match1 and match1.group(1).upper() == section_name_upper:
            return True
        match2: re.Match|None = options_section_header_parse_key_2.pattern.fullmatch(line)
        if match2 and match2.group(1).upper() == section_name_upper:
            return True

        preamble += line
        preamble += '\n'

    return False

def read_options_h(section_name: str) -> dict[str, VegaSetting]:
    settings: dict[str, VegaSetting] = dict()

    print('* Starting to read options.h file')

    with options_h_path.open(mode='r', encoding='utf-8', newline=None) as options_h_reader:
        found_section = skip_to_section_header(options_h_reader, section_name)
        if found_section:
            for line in options_h_reader:
                if line.isspace():
                    pass
                elif is_options_section_header_comment(line):
                    print("Done reading relevant section from options.h")
                    return settings
                elif comment_parse_key_1.pattern.fullmatch(line):
                    pass
                elif comment_parse_key_2.pattern.fullmatch(line):
                    pass
                elif trailing_comment_parse_key_1.pattern.fullmatch(line):
                    line_tmp: str = trailing_comment_parse_key_1.pattern.fullmatch(line).group(1)
                    for each_parse_key in setting_declaration_parse_keys:
                        attempted_match: re.Match|None = each_parse_key.pattern.fullmatch(line_tmp)
                        if attempted_match:
                            setting: VegaSetting = VegaSetting()
                            setting.type = attempted_match.group('type')
                            setting.name = attempted_match.group('name')
                            setting.replacement_expression = ''
                            settings[setting.name] = setting
                            break
                elif trailing_comment_parse_key_2.pattern.fullmatch(line):
                    line_tmp: str = trailing_comment_parse_key_2.pattern.fullmatch(line).group(1)
                    for each_parse_key in setting_declaration_parse_keys:
                        attempted_match: re.Match|None = each_parse_key.pattern.fullmatch(line_tmp)
                        if attempted_match:
                            setting: VegaSetting = VegaSetting()
                            setting.type = attempted_match.group('type')
                            setting.name = attempted_match.group('name')
                            setting.replacement_expression = ''
                            settings[setting.name] = setting
                            break
                else:
                    line_tmp: str = line
                    for each_parse_key in setting_declaration_parse_keys:
                        attempted_match: re.Match|None = each_parse_key.pattern.fullmatch(line_tmp)
                        if attempted_match:
                            setting: VegaSetting = VegaSetting()
                            setting.type = attempted_match.group('type')
                            setting.name = attempted_match.group('name')
                            setting.replacement_expression = ''
                            settings[setting.name] = setting
                            break
                    if not attempted_match:
                        print("Don't know what to do with this options.h line:")
                        print(line)
            return settings
        else:
            print("Didn't find section " + section_name + " in options.h")
            return settings


def get_newline_positions(file_contents: str) -> list[int]:
    ret_val: list[int] = [match.start() for match in re.finditer('\n', file_contents)]
    return ret_val


def process_a_transform_pass(starting_file_contents: str, parse_key: ParseKey) -> str:
    if parse_key and parse_key.substitution:
        return re.sub(parse_key.pattern, parse_key.substitution, starting_file_contents)
    return starting_file_contents


def read_options_cpp(config_items: dict[str, VegaSetting]) -> None:
    print('** Starting to read options.cpp file')

    for each_parse_key in equals_configuration_parse_keys_multi_line:
        with options_cpp_path.open(mode='r', encoding='utf-8', newline=None) as options_cpp_reader:
            options_cpp_content: str = options_cpp_reader.read()
            each_pattern: re.Pattern = each_parse_key.pattern
            current_match: re.Match|None = each_pattern.search(options_cpp_content)
            while current_match:
                setting_name: str = current_match.group('name')
                replacement_expression: str = current_match.group('config_expr')
                if setting_name in config_items.keys():
                    setting: VegaSetting | None = config_items[setting_name]
                    if setting:
                        setting.replacement_expression = replacement_expression
                current_match = each_parse_key.pattern.search(options_cpp_content, current_match.end())

    for each_parse_key in equals_configuration_parse_keys_single_line:
        with options_cpp_path.open(mode='r', encoding='utf-8', newline=None) as options_cpp_reader:
            for each_line in options_cpp_reader:
                current_match: re.Match|None = each_parse_key.pattern.fullmatch(each_line)
                if current_match:
                    setting_name: str = current_match.group('name')
                    replacement_expression: str = current_match.group('config_expr')
                    if setting_name in config_items.keys():
                        setting: VegaSetting | None = config_items[setting_name]
                        if setting:
                            setting.replacement_expression = replacement_expression


def read_parse_write_misc_file(file_path: Path, my_parse_keys: list[ParseKey]) -> None:
    print('*** Starting to process file ' + file_path.__fspath__())
    this_file_as_read: str
    with file_path.open(mode='r', encoding='utf-8', newline=None) as file_read:
        this_file_as_read = file_read.read()
    this_file_rewritten: str = this_file_as_read
    for each_parse_key in my_parse_keys:
        this_file_rewritten = process_a_transform_pass(this_file_rewritten, each_parse_key)
    with file_path.open(mode='w', encoding='utf-8', newline=None) as file_write:
        file_write.write(this_file_rewritten)

    return None


def list_files_recursive() -> list[Path]:
    ret_val: list[Path] = []

    for file in Path('./engine/src').rglob('*.h'):
        if not str(file).endswith('options.h'):
            ret_val.append(file)
    for file in Path('./engine/src').rglob('*.h.in'):
        ret_val.append(file)
    for file in Path('./engine/src').rglob('*.c'):
        ret_val.append(file)
    for file in Path('./engine/src').rglob('*.cpp'):
        ret_val.append(file)

    for file in Path('./libraries').rglob('*.h'):
        if not str(file).endswith('options.h'):
            ret_val.append(file)
    for file in Path('./libraries').rglob('*.h.in'):
        ret_val.append(file)
    for file in Path('./libraries').rglob('*.c'):
        ret_val.append(file)
    for file in Path('./libraries').rglob('*.cpp'):
        ret_val.append(file)

    return ret_val


def build_gl_options_parse_keys(parse_keys_list: list[ParseKey], config_items: dict[str, VegaSetting]) -> list[ParseKey]:
    for each_key in config_items.keys():
        this_setting: VegaSetting = config_items[each_key]
        if this_setting:
            new_parse_key = ParseKey()
            new_parse_key.pattern = re.compile(r'\bgl_options\.' + this_setting.name + r'\b', regex_flags_multiline)
            new_parse_key.substitution = this_setting.replacement_expression
            new_parse_key.leave_as_is = ''
            if new_parse_key.pattern and new_parse_key.substitution:
                parse_keys_list.append(new_parse_key)

    return parse_keys


def main() -> int:
    section_name_parsed: str = parse_cmdline_arguments()
    section_name_parsed = section_name_parsed.lower()
    my_parse_keys: list[ParseKey] = build_parse_keys(section_name_parsed)
    configuration_items: dict[str, VegaSetting]
    configuration_items = read_options_h(section_name_parsed)

    read_options_cpp(configuration_items)

    build_game_options_parse_keys(my_parse_keys, configuration_items)

    build_gl_options_parse_keys(my_parse_keys, configuration_items)

    file_list: list[Path] = list_files_recursive()
    for file in file_list:
        read_parse_write_misc_file(file, my_parse_keys)

    return 0


if __name__ == "__main__":
    sys.exit(main())

