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

#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <pthread.h>

#include "window.h"
#include "ncrok.h"

Window::Window(){

}

Window::~Window(){
	del_panel(panel);
	delwin(window);
}

void Window::init(){
	window = newwin(h, w, y, x);
	panel = new_panel(window);
	box(window,0,0);

	printTitle(title);
}

void Window::init(int _x, int _y, int _w, int _h){
	x = _x; y = _y; w = _w; h = _h;
	init();
}

void Window::initCenter(int _x, int _y, int _w, int _h){
	init(_x - _w / 2, _y - _h /2, _w, _h);
}

void Window::resize(int _x, int _y, int _w, int _h){
	x = _x; y = _y; w = _w; h = _h;

	mvwin(window, y, x);
	wresize(window, h, w);

	clear();

	replace_panel(panel, window);

	box(window, 0, 0);
}

void Window::resizeCenter(int _x, int _y, int _w, int _h){
	resize(_x - _w / 2, _y - _h / 2, _w, _h);
}

void Window::reBox(){
	box(window,0,0); //overwrite any old text
	printCentered(title, 0);
}

void Window::clear(){
	werase(window);
	reBox();
}

void Window::printTitle(const std::string &ntitle){
	title.assign(ntitle);

	reBox();
}

void Window::printCentered(const std::string &text, int y){
	mvwprintw(window, y, (w/2)-(text.length()/2), "%s", text.c_str());
}

void Window::hide(){
	hide_panel(panel);
}

void Window::show(){
	show_panel(panel);
}

void Window::touch(){
	touchwin(window);
}

void Window::display(int, int){
	reBox();

	show();
}

