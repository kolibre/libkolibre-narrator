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

# Import custom modules
import Message
import Translation
import csvparser

# Import python modules
import sqlite3
import getopt
import sys
import shutil
import tempfile
from os import path
from os import system
from os import remove

def usage():
	"""
		prints usage information for this script
	"""
	print ''
	print 'usage: python ' + sys.argv[0] + ' -p <file> -m <file> -t <file> -l <language code> -o <file>'
	print
	print 'required arguments:'
	print ' -p, --prompts <file>\t\tcsv file listing prompts to include in build'
	print ' -m, --messages <file>\t\tcsv file listing prompt types and identifier'
	print ' -t, --translation <file>\tcsv file listing prompt translations'
	print ' -l, --language-code <lang>\ttwo letter code for the language to build (ISO-639)'
	print ' -o, --output <file>\t\toutput file'
	print
	print 'optional arguments:'
	print ' -a, --append\t\t\tappend prompts to existing database'
	print ' -i, --indir\t\t\tuse and save narrator audio in this directory, default is /tmp'
	print ' -u, --unarrator\t\tcreate prompts for unarrator'
	print ' -h, --help\t\t\tshow this help message and exit'

def listHasDuplicates(identifiers, filename):
	hasDuplicates = False
	uniqueIdentifiers = set(identifiers)
	for identifier in uniqueIdentifiers:
		count = identifiers.count(identifier)
		if count > 1:
			sys.stderr.write('Error: identifier \'' + str(identifier) + '\' occurs ' + str(count) + ' times in file ' + filename + '\n')
			hasDuplicates = True
	return hasDuplicates

# main starts here
if __name__ == '__main__':

	# check that oggenc command exists
	retval = system('which oggenc >> /dev/null')
	if retval != 0:
		sys.stderr.write('Error: oggenc command not found on system, make sure that package \'vorbis-tools\' is installed\n')
		sys.exit(1)

	# check that ogginfo command exists
	retval = system('which ogginfo >> /dev/null')
	if retval != 0:
		sys.stderr.write('Error: ogginfo command not found on system, make sure that package \'vorbis-tools\' is installed\n')
		sys.exit(1)

	# check that espeak command exists
	retval = system('which espeak >> /dev/null')
	if retval != 0:
		sys.stderr.write('Error: espeak command not found on system, make sure that package \'espeak\' is installed\n')
		sys.exit(1)

	# check that sox command exists
	retval = system('which sox >> /dev/null')
	if retval != 0:
		sys.stderr.write('Error: sox command not found on system, make sure that package \'sox\' is installed\n')
		sys.exit(1)

	# parse command line options
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'haup:m:t:l:o:i:', ['help', 'prompts', 'messages', 'translation', 'language-code', 'output', 'append', 'indir', 'unarrator'])
	except getopt.GetoptError, err:
		sys.stderr.write(str(err)) # will print something like "option -a not recognized"
		usage()
		sys.exit(2)

	promptFile = None
	messageFile = None
	translationFile = None
	langCode = None
	outputFile = None
	append = False
	audioDir = '/tmp'
	buildDB = True
	for opt, value in opts:
		if opt in ('-h', '--help'):
			usage()
			sys.exit()
		elif opt in ('-p', '--prompts'):
			promptFile = value
		elif opt in ('-m', '--messages'):
			messageFile = value
		elif opt in ('-t', '--translation'):
			translationFile = value
		elif opt in ('-l', '--language-code'):
			langCode = value
		elif opt in ('-o', '--output'):
			outputFile = value
		elif opt in ('-a', '--append'):
			append = True
		elif opt in ('-i', '--indir'):
			audioDir = value
		elif opt in ('-u', '--unarrator'):
			buildDB = False

	if promptFile == None or messageFile == None or translationFile == None or langCode == None or outputFile == None:
		usage()
		sys.exit(1)

	# try finding narrator prompts, types and translations
	narratorDataPath = None

	# search in install directory
	cwd = path.dirname(path.abspath(__file__))
	startPos = cwd.find('python'+sys.version[0:3])
	if startPos != -1:
		narratorDataPath = cwd[0:startPos-5] + '/share/libkolibre/narrator'

	# search in parent directory
	if narratorDataPath == None:
		tmpFile = cwd + '/../prompts/narrator.csv'
		if path.exists(tmpFile):
			narratorDataPath = path.dirname(path.abspath(tmpFile))

	# setup input files and paths
	narratorPromptFile = narratorDataPath + '/narrator.csv'
	narratorTypeFile = narratorDataPath + '/types.csv'
	narratorTranslationFile = narratorDataPath + '/' + langCode + '_translations.csv'
	unarratorDir = audioDir + '/' + 'unarrator' + '/' + langCode

	# verify that path exists
	if not path.exists(narratorPromptFile):
		narratorPromptFile = None
	if not path.exists(narratorTranslationFile):
		narratorTranslationFile = None
	if not path.exists(narratorTypeFile):
		narratorTypeFile = None
	if not path.exists(messageFile):
		sys.stderr.write('message file ' + messageFile + ' does not exist\n')
		sys.exit(1)
	if not path.exists(translationFile):
		sys.stderr.write('translation file ' + translationFile + ' does not exist\n')
		sys.exit(1)
	if not path.exists(promptFile):
		sys.stderr.write('prompt file ' + promptFile + ' does not exist\n')
		sys.exit(1)
	if not path.exists(audioDir):
		sys.stderr.write('directory ' + audioDir + ' does not exist\n')
		sys.exit(1)

	# delete outputFile if it exists and we don't want to append to it
	if not append and path.exists(outputFile):
		remove(outputFile)

	# parse message, translation and prompt files
	msgs = csvparser.parse(messageFile)
	if msgs == -1:
		sys.stderr.write('Error: could not parse message file ' + messageFile + '\n')
		sys.exit(1)
	trls = csvparser.parse(translationFile)
	if trls == -1:
		sys.stderr.write('Error: could not parse translation file ' + translationFile + '\n')
		sys.exit(1)
	prompts = csvparser.parse(promptFile)
	if prompts == -1:
		sys.stderr.write('Error: could not parse prompt file ' + promptFile + '\n')
		sys.exit(1)
	narratorPrompts = []
	if narratorPromptFile:
		narratorPrompts = csvparser.parse(narratorPromptFile)
		if narratorPrompts == -1:
			sys.stderr.write('Error: could not parse prompt file ' + narratorPromptFile + '\n')
			sys.exit(1)
	narratorTypes = []
	if narratorTypeFile:
		narratorTypes = csvparser.parse(narratorTypeFile)
		if narratorTypeFile == -1:
			sys.stderr.write('Error: could not parse type file ' + narratorTypeFile + '\n')
			sys.exit(1)
	narratorTranslations = []
	if narratorTranslationFile:
		narratorTranslations = csvparser.parse(narratorTranslationFile)
		if narratorTranslations == -1:
			sys.stderr.write('Error: could not parse translation file ' + narratorTranslationFile + '\n')
			sys.exit(1)

	# list duplicates in lists
	messageDuplicates = listHasDuplicates([ x[2] for x in msgs if len(x) >= 3 ], messageFile)
	messageIdDuplicates = listHasDuplicates([ x[0] for x in msgs ], messageFile)
	translationDuplicates = listHasDuplicates( [x[0] for x in trls ], translationFile)
	promptDuplicates = listHasDuplicates([ x[0] for x in prompts ], promptFile)
	narratorPromptDuplicates = listHasDuplicates([ x[0] for x in narratorPrompts ], narratorPromptFile)
	narratorTypeDuplicates = listHasDuplicates([ x[2] for x in narratorTypes if len(x) >= 3 ], narratorTypeFile)
	narratorTypeIdDuplicates = listHasDuplicates([ x[0] for x in narratorTypes ], narratorTypeFile)
	narratorTranslationDuplicates = listHasDuplicates([ x[0] for x in narratorTranslations ], narratorTranslationFile)

	# exit if duplicates occurs
	if messageDuplicates or messageIdDuplicates or translationDuplicates or promptDuplicates or narratorPromptDuplicates or narratorTypeDuplicates or narratorTypeIdDuplicates or narratorTranslationDuplicates:
		sys.exit(2)

	# combine narratorPrompts and prompts, remove duplicates
	for prompt in prompts:
		index = [i for i,x in enumerate(narratorPrompts) if x[0] == prompt[0]]
		if len(index) == 0:
			narratorPrompts.append(prompt)
	prompts = narratorPrompts

	# combine narratorTypes and messages, override existing narrator types with user's messages
	for msg in msgs:
		if len(msg) >= 3:
			index = [i for i,x in enumerate(narratorTypes) if x[1] == msg[1] and x[2] == msg[2]]
			if len(index) == 1:
				narratorTypes[index[0]] = msg
			elif len(index) == 0:
				narratorTypes.append(msg)
	msgs = narratorTypes

	# combine narratorTranslations and translations, override existing narrator translations with user's translations
	for trl in trls:
		if len(trl) >= 2:
			index = [i for i,x in enumerate(narratorTranslations) if x[0] == trl[0]]
			if len(index) == 1:
				narratorTranslations[index[0]] = trl
			elif len(index) == 0:
				narratorTranslations.append(trl)
	trls = narratorTranslations

	messages = []         # list of Message instances
	translations = []     # list of Translation instances
	promptmessages = []   # list of Messages to include in build
	try:
		tmpAudioDir = tempfile.mkdtemp()
		print tmpAudioDir
		# generate a list of translations
		for trl in trls:
			if len(trl) >= 2:
				basenames = trl[2:len(trl)]
				translations.append(Translation.Translation(langCode, trl[0], trl[1], basenames, tmpAudioDir))
	
		# generate a list of messages
		for msg in msgs:
			if len(msg) >= 3:
				translation = [translation for translation in translations if translation.key == msg[2]]
				if len(translation) == 1:
					messages.append(Message.Message(msg[2], msg[1], msg[0], translation[0]))
				else:
					sys.stderr.write('Error: no \'' + langCode + '\' translation found for message \'' + msg[2] + '\'\n')
					sys.stderr.write('Tip: You probably just have to add it to ' + translationFile + '\n')
					sys.exit(2)
	
		# generate a list of message to include in build
		for prompt in prompts:
			message = [message for message in messages if message.key == prompt[0]]
			if len(message) == 1:
				promptmessages.append(message[0])
			else:
				sys.stderr.write('Error: no message with identifier \'' + prompt[0] + '\' found in ' + messageFile + '\n')
				sys.stderr.write('Tip: You probably just have to add it to ' + messageFile + '\n')
				sys.exit(2)
	
		# create ogg audio for messages to include in build
		if buildDB:
			oggcreateFailed = False
			for message in promptmessages:
				retval = message.oggcreate(langCode)
				if retval != True:
					oggcreateFailed = True
			if oggcreateFailed:
				sys.exit(2)
	
			# validate messages to include in build
			for message in promptmessages:
				valid = message.validate()
				if valid != True:
					print valid
					sys.exit(2)
	
		if buildDB:
			# connect to sql database
			sql = sqlite3.connect(outputFile)
			sql.text_factory = str
			cursor = sql.cursor()
	
			# create table message
			cursor.execute('CREATE TABLE IF NOT EXISTS message (string TEXT, class TEXT, id INT, UNIQUE(class, id))')
			# create table messageparameter
			cursor.execute('CREATE TABLE IF NOT EXISTS messageparameter (message_id INT, key TEXT, type TEXT)')
			# create table messagetranslation
			cursor.execute('CREATE TABLE IF NOT EXISTS messagetranslation (message_id INT, translation TEXT, language TEXT, audiotags TEXT)')
			# create table messageaudio
			cursor.execute('CREATE TABLE IF NOT EXISTS messageaudio (translation_id INT, tagid INT, text TEXT, size INT, length INT, data BLOB, md5 TEXT)')
	
			# insert data in db
			for message in promptmessages:
				message.insert(cursor)
	
			# save (commit) the changes
			sql.commit()
	
			# close cursor
			cursor.close()
		else:
			for message in promptmessages:
				message.resample(unarratorDir, langCode)
	finally:
		print "Removing temp folder: " + tmpAudioDir
		shutil.rmtree(tmpAudioDir)
