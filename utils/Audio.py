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

import messagehelper

import sqlite3
import re
import hashlib
from os import path
from os import popen
from os import remove
from os import system
from os import path

class Audio:
	"""
		Audio class representing a row in table messageaudio in a database
	"""
	id = None
	tagid = None
	text = None
	size = None
	length = None
	encoding = None
	data = None
	md5 = None

	def __init__(self,tagid,text,basename,audiodir):
		"""
			constructor for class Audio

			@param integer tagid => tag id for the audio
			@param string text => text representation of the audio
			@param string basename => basename to be used in filename generation
			@param string audiodir => full path to the directory where the audio file is located
			@return object Audio

			the constructor will only set values for class member variables tagid and text
			values for the remaing class member variables are set during generation of ogg audio
		"""
		self.tagid = tagid
		self.text = text
		# generate filenames for wav audio and ogg audio
		self.wavfile = messagehelper.genfilename(text, basename, audiodir, 'wav')
		self.oggfile = messagehelper.genfilename(text, basename, audiodir, 'ogg')
		self.encoding = 'ogg'

	def validate(self):
		"""
			validates an Audio instance

			@return boolan/string

			returns True if the instance is a valid Audio object,
			otherwise returns a string containing information why the instance is not valid Audio object
		"""
		if self.tagid == None:
			return 'Error: no tagid set for audio'
		if self.text == None:
			return 'Error: no text set for audio'
		if self.size == None:
			return 'Error: no size set for audio'
		if self.length == None:
			return 'Error: no length set for audio'
		if self.encoding == None:
			return 'Error: no encoding set for audio'
		if self.data == None:
			return 'Error: no data set for audio'
		if self.md5 == None:
			return 'Error: no md5 set for audio'
		return True

	def insert(self,cursor,translationId):
		"""
			inserts the instance in a database if it does not already exist

			@param object cursor => a cursor instance
			@param interger translationId => the id of a Translation instance
			@return boolean

			returns True and sets id to the value obtained from the database
		"""
		if not self.exists(cursor, translationId):
			text = unicode(self.text,'utf-8)').encode('utf-8')
			t = (translationId, self.tagid, text, self.size, self.length, self.encoding, self.data, self.md5)
			cursor.execute('INSERT INTO messageaudio VALUES (?, ?, ?, ?, ?, ?, ?, ?)', t)
			self.id = cursor.lastrowid
		return True

	def exists(self,cursor,translationId):
		"""
			checks if instance already exist in database

			@param object cursor => a cursor instance
			@param integer translationId => the id of a Translation instance
			@return boolean

			returns True if the instance already exist in the database and sets id to the value obtained from the database,
			otherwise returns False
		"""
		text = unicode(self.text,'utf-8)').encode('utf-8')
		t = (translationId, self.tagid, text)
		cursor.execute('SELECT rowid FROM messageaudio WHERE translation_id = ? AND tagid = ? AND text = ?', t)
		row = cursor.fetchone()
		if not row == None:
			self.id = row[0]
			return True
		return False

	def oggcreate(self,voice):
		"""
			encodes wav to ogg and sets values for reminaing class member variables

			@param boolean/string voice => generate missing audio files using this espeak voice
			@return boolan/string

			returns True if ogg file was created or up-to-date
			otherwise returns a string containing information why creation failed
		"""
		if not path.exists(self.wavfile):
			# Try searching for file in language independent folder
			cwd = path.dirname(path.abspath(__file__))
			basenamewav = path.basename(self.wavfile)
			basenameogg = path.basename(self.oggfile)
			wavfile = cwd + '/special/' + basenamewav
			oggfile = cwd + '/special/' + basenameogg
			if path.exists(wavfile):
				self.wavfile = wavfile
			if path.exists(oggfile):
				self.oggfile = oggfile
		if voice and not path.exists(self.wavfile):
			cwd = path.dirname(path.abspath(__file__))
			command = 'espeak -v ' + voice + ' -w ' + self.wavfile + ' \"' + self.text + '\"'
			print 'Info: wav file ' + self.wavfile + ' not found, generating audio with espeak'
			system(command)
		retval = self.oggenc(self.wavfile, self.oggfile)
		if retval != 0:
		 	return retval
		if not path.exists(self.oggfile):
			return 'Error: ogg file ' + self.oggfile + ' does not exist\n'
		self.size = self.oggsize(self.oggfile)
		self.length = self.ogglength(self.oggfile)
		if not isinstance(self.length, int):
			return self.length # self.length is now an error string
		self.data = self.oggdata(self.oggfile)
		self.md5 = self.oggmd5(self.oggfile)
		return True

	def oggenc(self,wavfile,oggfile):
		"""
			encodes wav to ogg if the ogg file does not exist or if the wav file is newer than the ogg file
                        1. sox trims silence from beginning and end of audio
                        2. sox pads audio with 10ms silence before and 50ms silence after audio
                        3. oggenc encodes trimmed and padded audio to ogg

			@param string wavfile => full path to the input wav file
			@param string oggfile => full path to the output ogg file
			@return integer/string

			returns 0 if the convertion was successfull,
			otherwise returns a string containing the command used to convert audio
		"""
		trimmed_wav_file = path.split(wavfile)[0] + '/trimmed.wav'
		trimcommand = 'sox -q -t wav ' + wavfile + ' ' + trimmed_wav_file + ' silence 1 1 0.05% reverse silence 1 1 0.05% reverse pad 0.010 0.050'
		encodecommand = 'oggenc -Q --resample 44100 ' + trimmed_wav_file + ' -o ' + oggfile

		# ogg file does not exist
		if not path.exists(self.oggfile):
			print 'Info: ogg file ' + oggfile + ' does not exist, encoding wav to ogg'

			child = popen(trimcommand)
			err = child.close()
			if err != None:
				return 'Error: ' + wavfile + ' could not trim audio using using command: ' + trimcommand + '\n'

			child = popen(encodecommand)
			err = child.close()
			if err != None:
				return 'Error: ' + wavfile + ' could not be encoded to ogg using command: ' + encodecommand + '\n'
			return 0

		# wav file is newer than ogg file
		if path.getctime(wavfile) > path.getctime(oggfile):
			remove(oggfile)
			print 'Info: wav file ' + wavfile + ' is updated, re-encoding wav to ogg'

			child = popen(trimcommand)
			err = child.close()
			if err != None:
				return 'Error: ' + wavfile + ' could not trim audio using using command: ' + trimcommand + '\n'

			child = popen(encodecommand)
			err = child.close()
			if err != None:
				return 'Error: ' + wavfile + ' could not be encoded to ogg using command: ' + encodecommand + '\n'
			return 0
		return 0

	def oggsize(self,file):
		"""
			retrieves the file size in bytes for the ogg file

			@param string file => full path to the ogg file
			@return integer
		"""
		return path.getsize(file)

	def ogglength(self,file):
		"""
			retrives the playback length in milliseconds for the ogg file

			@param string file => full path to the ogg file
			@return integer/string

			returns an integer if the parsing was successful,
			otherwise returns a string containing information why parsing was unsuccessful
		"""
		command = 'ogginfo ' + file
		child = popen(command)
		data = child.read()
		err = child.close()
		if err != None:
			return 'Error: could not get info for file ' + file + '\n'
		pattern = re.compile('Playback length: (\d+)m:(\d+).(\d+)s')
		matches = pattern.findall(data)
		if len(matches) != 1:
			return 'Error: could not find playback length for file ' + file + '\n'
		if len(matches[0]) != 3:
			return 'Error: could not parse playback length for file ' + file + '\n'
		minutes = int(matches[0][0])
		seconds = int(matches[0][1])
		mseconds = int(matches[0][2])
		return minutes*60*1000 + seconds*1000 + mseconds

	def oggdata(self,file):
		"""
			retrives the binary data for the ogg file

			@param string file => full path to the ogg file
			@return binary data

		"""
		return sqlite3.Binary(open(file, 'rb').read())

	def oggmd5(self,file):
		"""
			retrives the md5 sum for the ogg file

			@param string file => full path to the ogg file
			@return string
		"""
		h = hashlib.md5(open(file, 'rb').read())
		return h.hexdigest()

	def resample(self,filename,voice):
		"""
			resmaples wav file and writes it to the file specified by filename

			@param filename => full path to where the wav file shall be stored
			@param boolean/string voice => generate missing audio files using this espeak voice
		"""
		if voice and not path.exists(self.wavfile):
			cwd = path.dirname(path.abspath(__file__))
			command = 'espeak -v ' + voice + ' -w ' + self.wavfile + ' \"' + self.text + '\"'
			system(command)
		if not path.exists(self.wavfile):
			return 'Error: wav file ' + self.wavfile + ' does not exist\n'

		if not path.exists(filename):
			command = 'sox ' + self.wavfile + ' -r 8000 -b 8 ' + filename
			print 'Info: wav file ' + filename + ' does not exist, resampling ' + self.wavfile
			system(command)

		return True
