#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Copyright (C) 2012  The Kolibre Foundation

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

import Parameter
import Audio
import messagehelper

import sqlite3
import re
import sys

class Translation:
	"""
		Translation class representing a row in table messagetranslation in a database

		A translation may have Parameter and Audio childs
	"""
	id = None
	language = None
	key = None
	text = None
	parameters = None
	audioclips = None

	def __init__(self,language,key,text,basenames,audiodir):
		"""
			constructor for class Translation

			@param string language => short langauge code for the translation
			@param string key => original text
			@param string text => translation of the original text
			@param list basenames => list of basenames to be used in filename generation
			@param string audiodir => full path to the directory where the audio file is located
			@return object Translation

			the constructor will try to set values for all class member variables

		"""
		self.language = language
		self.key = key
		self.text = text
		self.parameters = self.params(text)
		self.audioclips = self.aclips(text, basenames, audiodir)

	def validate(self):
		"""
			validates a Translation instance and its childs

			@return boolan/string

			returns True if the instance is a valid Translation object,
			otherwise returns a string containing information why the instance is not valid Translation object
		"""
		if self.language == None:
			return 'Error: no language set for translation'
		if self.text == None:
			return 'Error: no text set for translation'
		for parameter in self.parameters:
			valid = parameter.validate()
			if not valid == True:
				return valid
		for audioclip in self.audioclips:
			valid = audioclip.validate()
			if not valid == True:
				return valid
		return True

	def insert(self,cursor,messageId):
		"""
			inserts the instance and its childs in a database if it does not already exist

			@param object cursor => a cursor instance
			@param interger messageId => the id of a Message instance
			@return boolean

			returns True and sets id to the value obtained from the database
		"""
		if not self.exists(cursor, messageId):
			text = unicode(messagehelper.stripparamtype(self.text),'utf-8)').encode('latin1')
			t = (messageId, text, self.language, self.tags(self.text))
			cursor.execute('INSERT INTO messagetranslation VALUES (?, ?, ?, ?)', t)
			self.id = cursor.lastrowid
		for parameter in self.parameters:
			parameter.insert(cursor, messageId)
		for audioclip in self.audioclips:
			audioclip.insert(cursor, self.id)
		return True

	def exists(self,cursor,messageId):
		"""
			checks if instance already exist in database

			@param object cursor => a cursor instance
			@param integer messageId => the id of a Message instance
			@return boolean

			returns True if the instance already exist in the database and sets id to the value obtained from the database,
			otherwise returns False
		"""
		text = unicode(messagehelper.stripparamtype(self.text),'utf-8)').encode('latin1')
		t = (messageId, text, self.language)
		cursor.execute('SELECT rowid FROM messagetranslation WHERE message_id = ? AND translation = ? AND language = ?', t)
		row = cursor.fetchone()
		if not row == None:
			self.id = row[0]
			return True
		return False

	def tags(self,translation):
		"""
			generates a tag representation from a translation

			@param string translation => a translation string
			@return string
		"""
		i = 0
		tags = []
		translation = messagehelper.stripparamtype(translation)
		param = re.compile('\{\w+\}')
		m = param.search(translation)
		while m != None:
			if m.start() != 0:
				tags.append('[' + str(i) + ']')
				i += 1
			tags.append(translation[m.start():m.end()])
			translation = translation[m.end():].lstrip(' ').rstrip(' ')
			m = param.search(translation)
		if len(translation) > 0: tags.append('[' + str(i) + ']')
		return ' '.join(str(tag) for tag in tags)

	def params(self,translation):
		"""
			creates Parameter childs from a translation

			@param string translation => a translation string
			@return list of Parameters instances
		"""
		pl = []
		matches = messagehelper.extractparams(translation)
		for match in matches:
			pl.append(Parameter.Parameter(match[0], match[1]))
		return pl

	def aclips(self,translation,basenames,audiodir):
		"""
			creates Audio childs from a translation

			@param string translation => translation of the original text
			@param list basenames => list of basenames to be used in filename generation
			@param string audiodir => full path to the directory where the audio file is located
			@return list of Audio instances
		"""
		i = 0
		tags = []
		translation = messagehelper.stripparamtype(translation)
		param = re.compile('\{\w+\}')
		m = param.search(translation)
		while m != None:
			if m.start != 0:
				text = translation[0:m.start()].lstrip(' ').rstrip(' ')
				if len(text) > 0:
					tags.append((i, text))
					i += 1
			translation = translation[m.end():].lstrip(' ').rstrip(' ')
			m = param.search(translation)
		if len(translation) > 0: tags.append((i, translation))
		al = []
		for tag in tags:
			if len(basenames) > 0:
				al.append(Audio.Audio(tag[0], tag[1], basenames.pop(0), audiodir))
			else:
				al.append(Audio.Audio(tag[0], tag[1], None, audiodir))
		return al

	def oggcreate(self,voice):
		"""
			invokes operation oggcreate() for all audioclips
			and returns False if operation failed, otherwise True

			@param boolean/string voice => generate missing audio files using this espeak voice
		"""
		result = True
		for audioclip in self.audioclips:
			retval = audioclip.oggcreate(voice)
		 	if retval != True:
		 		sys.stderr.write(str(retval))
		 		result = False
		return result

	def resample(self,destination,voice):
		"""
			invokes operation resample() for its audioclip
			and returns False if operation failed, otherwise True

			@param destination => path to destination folder
			@param boolean/string voice => generate missing audio files using this espeak voice
		"""

		# create tags from key string
		i = 0
		tags = []
		key = messagehelper.stripparamtype(self.key)
		param = re.compile('\{\w+\}')
		m = param.search(key)
		while m != None:
			if m.start != 0:
				text = key[0:m.start()].lstrip(' ').rstrip(' ')
				if len(text) > 0:
					tags.append((i, text))
					i += 1
			key = key[m.end():].lstrip(' ').rstrip(' ')
			m = param.search(key)
		if len(key) > 0: tags.append((i, key))

		if len(tags) > 1 and len(self.audioclips) > 1:
			sys.stderr.write('Error: key or translation can contain ONLY one tag\n')
			sys.stderr.write('       key is: \"' + self.key + '\"\n')
			sys.stderr.write('       translation is: \"' + self.text + '\"\n')
			return False

		result = True
		tag = tags[0];
		audioclip = self.audioclips[0]
		filename = messagehelper.genfilename(tag[1], None, destination, 'wav')
		retval = audioclip.resample(filename,voice)
		if retval != True:
			sys.stderr.write(str(retval))
			result = False
		return result
