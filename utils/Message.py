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

import Translation

import sqlite3

class Message:
	"""
		Message class representing a row in table message in a database

		A message have a Translation child
	"""
	id = None
	key = None
	type = None
	types = dict({'prompt':1, 'announcement':1, 'number':1, 'date':1})
	translation = None

	def __init__(self,key,type,translation):
		"""
			constructor for class Message

			@param string key => original text
			@param string type => type of message
			@param object translation => a Translation instance
			@return object Message

			the constructor will try to set values for all class member variables
		"""
		self.key = key
		self.type = type
		self.translation = translation

	def validate(self):
		"""
			validates a Message instance and its child

			@return boolan/string

			returns True if the instance is a valid Message object,
			otherwise returns a string containing information why the instance is not valid Message object
		"""
		if self.key == None:
			return 'Error: no key set for message'
		if self.type == None:
			return 'Error: no type set for message \'' + self.key + '\''
		if not self.type in self.types:
			return 'Error: invalid type value \'' + self.type + '\' for message \'' + self.key + '\''
		if self.translation == None:
			return 'Error: no translation set for message \'' + self.key + '\''
		valid = self.translation.validate()
		if not valid == True:
			return valid + ' in message \'' + self.key + '\''
		return True

	def insert(self,cursor):
		"""
			inserts the instance and its child in a database if it does not already exist

			@param object cursor => a cursor instance
			@return boolean

			returns True and sets id to the value obtained from the database
		"""
		if not self.exists(cursor):
			t = (self.key, self.type)
			cursor.execute('INSERT INTO message values (?,?)', t)
			self.id = cursor.lastrowid
		return self.translation.insert(cursor, self.id);

	def exists(self,cursor):
		"""
			checks if instance already exist in database

			@param object cursor => a cursor instance
			@return boolean

			returns True if the instance already exist in the database and sets id to the value obtained from the database,
			otherwise returns False
		"""
		t = (self.key, self.type)
		cursor.execute('SELECT rowid FROM message WHERE string = ? AND class = ?', t)
		row = cursor.fetchone()
		if not row == None:
			self.id = row[0]
			return True
		return False

	def oggcreate(self,voice):
		"""
			invokes operation oggcreate() for its translation
			and returns the value returned by oggcreate()

			@param boolean/string voice => generate missing audio files with this espeak voice
		"""
		return self.translation.oggcreate(voice);

	def resample(self,destination,voice):
		"""
			invokes operation resample() for its translation
			and returns the value returned by resample()

			@param destination => path to destination folder
			@param boolean/string voice => generate missing audio files with this espeak voice
		"""
		return self.translation.resample(destination,voice)
