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

import sqlite3

class Parameter:
	"""
		Parameter class representing a row in table messageparameter in a database
	"""
	id = None
	key = None
	type = None
	types = dict({'number':1, 'number-en':1, 'digits':1, 'date(date)':1, 'date(year)':1, 'date(yearnum)':1, 'date(month)':1, 'date(dayname)':1, 'date(minute)':1, 'date(second)':1,'message':1})

	def __init__(self,key,type):
		"""
			constructor for class Parameter

			@param string key => key of parameter
			@param string type => type of parameter
			@return object Parameter

			the constructor will try to set values for all class member variables

		"""
		self.key = key
		self.type = type

	def validate(self):
		"""
			validates a Parameter instance

			@return boolan/string

			returns True if the instance is a valid Parameter object,
			otherwise returns a string containing information why the instance is not valid Parameter object
		"""
		if self.key == None:
			return 'Error: no key set for parameter'
		if self.type == None:
			return 'Error: no type set for parameter'
		if not self.type in self.types:
			return 'Error: invalid type value \'' + self.type + '\' for parameter'
		return True

	def insert(self,cursor,messageId):
		"""
			inserts the instance in a database if it does not already exist

			@param object cursor => a cursor instance
			@param interger messageId => the id of a Message instance
			@return boolean

			returns True and sets id to the value obtained from the database
		"""
		if not self.exists(cursor, messageId):
			t = (messageId, self.key, self.type)
			cursor.execute('INSERT INTO messageparameter VALUES (?, ?, ?)', t)
			self.id = cursor.lastrowid
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
		t = (messageId, self.key, self.type)
		cursor.execute('SELECT rowid FROM messageparameter WHERE message_id = ? AND key = ? AND type = ?', t)
		row = cursor.fetchone()
		if not row == None:
			self.id = row[0]
			return True
		return False
