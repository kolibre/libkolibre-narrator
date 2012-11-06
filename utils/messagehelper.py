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

import re

def stripparamtype(text):
	"""
		strips the parameter types from a text string

		@param string text => a text string
		@return string
	"""
	param = re.compile('(\{\w+):([-\w]+|[-\w]+\(\w+\))(\})')
	return param.sub('\\1\\3', text)

def extractparams(text):
	"""
		extracts the parameters from a text string

		@param string text => a text string
		@return string
	"""
	param = re.compile('\{(\w+):([-\w]+|[-\w]+\(\w+\))\}')
	return param.findall(text)

def genfilename(text,basename,dirname,extension):
	"""
		generates a filename from a text string

		@param string text => text string from which to generate filename
		@param string basename => basename to be used in filename
		@param string dirname => dirname to be used in filename
		@param string extension => extension to be used in filename
		@return string
	"""

	# setup dirname
	if dirname == None: dirname = ''
	else: dirname = dirname + '/'

	# if a file name has been specified
	if basename != None:
		return dirname + basename + '.' + extension

	# generate file name from text string
	text = text.replace('å', 'a').replace('ä', 'a').replace('ö','o')
	text = text.replace('Å', 'A').replace('Ä', 'A').replace('Ö','O')
	text = text.replace('?', '')  # remove question marks
	text = text.replace('.', '')  # remove dots
	text = text.replace(',', '')  # remove commas
	text = text.replace(':', '')  # remove colons
	text = text.replace(' ', '_') # replace spaces with underscores
	return dirname + text.lower() + '.' + extension
