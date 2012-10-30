/***************************************************************************
 *   Copyright (C) 2008-2012 by Ben Nahill                                 *
 *   bnahill@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <tag.h>
#include <fileref.h>
#include <menu.h>
#include <string>
#include <string.h>
#include <regex.h>

#include "audio.h"
#include "window.h"
#include "tune.h"

#include <fstream>

static bool compare_i(char a, char b);
static void cleanString(std::string &in, int maxlen);

Tune::Tune(const std::string &name) :
	filename(name)
{
	track = 0;
	year = 0;
	parseFile();
	genDisplay();
	queue_index = -1;
	stopafter = 0;
}

void Tune::genDisplay(){
	char buffer[1024];
	size_t last_slash;

	displayName.clear();
	if(title.length() > 0){
		sprintf(buffer, "%s - %s [%02d] - %s", artist.c_str(), album.c_str(), track, title.c_str());
		displayName.assign(buffer);
	}
	else{
		//Find filename start index
		last_slash = filename.find_last_of('/');
		if(artist.length() > 0){
			sprintf(buffer, "%s - %s", artist.c_str(), filename.c_str() + last_slash + 1);
			displayName.assign(buffer);
		} else {
			displayName.assign(filename);
		}
	}
}

Tune::Tune(struct tune_block &block) :
	filename(block.filename)
{
	artist.assign(block.artist);
	album.assign(block.album);
	title.assign(block.title);

	track = block.track;
	year = block.year;

	queue_index = -1;
	stopafter = 0;
	genDisplay();
}


void Tune::getBlock(struct tune_block &block) const{
	strncpy(block.filename, filename.c_str(), TUNE_LEN_FNAME);
	strncpy(block.artist, artist.c_str(), TUNE_LEN_ARTIST);
	strncpy(block.album, album.c_str(), TUNE_LEN_ALBUM);
	strncpy(block.title, title.c_str(), TUNE_LEN_TITLE);

	block.track = track;
	block.year = year;
}


Tune::~Tune(){

}


const std::string &Tune::getMenuText() const{
	return displayName;
}

ITEM *Tune::getItem(){
	ITEM * tmpitem;
	uint_fast16_t i;
	// To be freed by ncurses, which uses this space instead of copying it
	char *item_text = (char *)malloc(TUNE_LEN_ITEM);
	sprintf(item_text, "   ");
	if(queue_index != -1){
		if(queue_index > 8)
			sprintf(item_text,"%d %s",queue_index+1, displayName.c_str());
		else sprintf(item_text,"%d  %s",queue_index+1, displayName.c_str());
	} else {
		strcpy(item_text + 3, displayName.c_str());
	}
	if(stopafter) item_text[2] = '*';
	item_text[TUNE_LEN_ITEM - 1] = 0;

	tmpitem = new_item(item_text,NULL);
	
	// If new_item finds an unprintable character, go find it and replace it
	if(!tmpitem){
		for(i = strlen(item_text) - 1; i; i--){
			if(!isprint(item_text[i])){
				item_text[i] = '_';
			}
		}
		
		// Then create a new item that it will actually accept
		tmpitem = new_item(item_text,NULL);
	}
	
	return tmpitem;
}

void Tune::updateItem(ITEM *item){
	char* item_text = (char*)malloc(TUNE_LEN_ITEM);
	sprintf(item_text, "   ");
	if(queue_index != -1){
		if(queue_index > 8)//needs 2 digits
			sprintf(item_text,"%d %s",queue_index+1, displayName.c_str());
		else sprintf(item_text,"%d  %s",queue_index+1, displayName.c_str());
	}
	else sprintf(item_text,"   %s",displayName.c_str());
	if(stopafter) item_text[2] = '*';
	free((void*)item->name.str);
	item->name.str = item_text;
}

bool Tune::startsWith(char* str) const{
	int len = strlen(str);
	char i;
	for(i = 0; i < strlen(str); i++){
		if(compare_i(str[i],displayName[i]) == 0) return false;
	}
	return true;
}

bool Tune::query(regex_t **terms) const{
	char i;
	for(i = 0; terms[i] != NULL; i++){
		if(regexec(terms[i], artist.c_str(), 0, NULL, 0) != 0 &&
		   regexec(terms[i], title.c_str(), 0, NULL, 0) != 0 &&
		   regexec(terms[i], album.c_str(), 0, NULL, 0) != 0)
		return false;
	}
	return true;
}

void Tune::parseFile(){
	TagLib::FileRef f(filename.c_str());

	if(f.tag()->isEmpty()) {
		guessFile();
		return;
	}
	
	artist.assign(f.tag()->artist().to8Bit());
	cleanString(artist, TUNE_LEN_ARTIST);

	if(artist.length() != 0){ //Do the rest
		album.assign(f.tag()->album().to8Bit());
		cleanString(album, TUNE_LEN_ALBUM);

		title.assign(f.tag()->title().to8Bit());
		cleanString(title, TUNE_LEN_TITLE);

		track = f.tag()->track();
		year = f.tag()->year();
	} else guessFile();
}

void Tune::guessFile(){
	//char buffer[512];
	size_t last_slash = filename.find_last_of('/');
	if(last_slash != std::string::npos){
		//strcpy(buffer, filename.c_str() + last_slash + 1);
		
		title.assign(filename.substr(last_slash + 1));
		//cleanString(title, TUNE_LEN_TITLE);
	}
}

void Tune::play() const{
	std::string path("file://");
	path.append(filename);
	audio.playPath(path);
}

const std::string &Tune::getAlbum() const { return album; }
const std::string &Tune::getArtist() const { return artist; }
const std::string &Tune::getTitle() const { return title; }

uint32_t Tune::getTrack() const{ return track; }
uint32_t Tune::getYear() const{ return year; }

bool Tune::tune_compare(const Tune &a, const Tune &b){
	int result;
	result = a.artist.compare(b.artist);
	if(result != 0)
		return (result < 0);
	result = a.album.compare(b.album);
	if(result != 0)
		return (result < 0);
	result = a.track - b.track;
	if(result != 0)
		return (result < 0);
	result = a.title.compare(b.title);
	return (result < 0);
}

static bool compare_i(char a, char b){
	if(a == b) return true;
	if(a + 32 == b) return true;
	if(b + 32 == a) return true;
	return false;
}

static void cleanString(std::string &s, int maxlen){
	if(s.size() > maxlen){
		s.resize(maxlen);
	}
}
