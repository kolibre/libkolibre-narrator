#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Copyright (C) 2012 Kolibre

This file is part of kolibre-narrator.

Kolibre-narrator is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Kolibre-narrator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with kolibre-narrator. If not, see <http://www.gnu.org/licenses/>.
"""

import csv
from os import path

def parse(file):
	"""
		parse a comma separated values (csv) file
		@param string file => input file to parsea

		@return -1 on failure
		@return string content on success
	"""
	if not path.exists(file):
		return -1
	lines = csv.reader(open(file, 'rb'), delimiter=';', quotechar='"')
	data = []
	for line in lines:
		if len(line) == 0:
			continue
		if line[0][0] == '#':
			continue
		data.append(line)
	return data
