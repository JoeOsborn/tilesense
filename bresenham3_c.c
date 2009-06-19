/*
* libtcod 1.4.1
* Copyright (c) 2008,2009 J.C.Wilk
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of J.C.Wilk may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY J.C.WILK ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL J.C.WILK BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "libtcod.h"
#include "bresenham3.h"

static TCOD_bresenham3_data_t bresenham3_data;

/* ********** bresenham3 line drawing ********** */
void TCOD_line3_init_mt(int xFrom, int yFrom, int zFrom, int xTo, int yTo, int zTo, TCOD_bresenham3_data_t *data) {
	int x=data->origx=xFrom,y=data->origy=yFrom,z=data->origz=zFrom;
	data->destx=xTo;
	data->desty=yTo;
  data->destz=zTo;
	data->deltax=data->destx-x;
	data->deltay=data->desty-y;
  data->deltaz=data->destz-z;
	if ( data->deltax > 0 ) {
		data->stepx=1;
	} else if ( data->deltax < 0 ){
		data->stepx=-1;
	} else data->stepx=0;
	if ( data->deltay > 0 ) {
		data->stepy=1;
	} else if ( data->deltay < 0 ){
		data->stepy=-1;
	} else data->stepy = 0;
	if ( data->deltaz > 0 ) {
		data->stepz=1;
	} else if ( data->deltaz < 0 ){
		data->stepz=-1;
	} else data->stepz = 0;
	
	if ( data->stepx*data->deltax > data->stepy*data->deltay ) {
		data->e1 = data->stepx*data->deltax;
		data->deltax *= 2;
		data->deltay *= 2;
	} else {
		data->e1 = data->stepy*data->deltay;
		data->deltax *= 2;
		data->deltay *= 2;
	}
  float l = data->stepx*data->deltax;
  float m = data->stepy*data->deltay;
  float n = data->stepz*data->deltaz;
	if ( l >= m && l >= n ) {
    data->e1 = m;
    data->e2 = n;
	} else if (m >= l && l >= n) {
		data->e1 = l;
    data->e2 = n;
	} else {
    data->e1 = l;
    data->e2 = m;
	}
	data->deltax *= 2;
	data->deltay *= 2;
	data->deltaz *= 2;
}

bool TCOD_line3_step_mt(int *xCur, int *yCur, int *zCur, TCOD_bresenham3_data_t *data) {
  float l = data->stepx*data->deltax;
  float m = data->stepy*data->deltay;
  float n = data->stepz*data->deltaz;
	if ( l >= m && l >= n ) {
		if ( data->origx == data->destx ) return true;
		data->origx+=data->stepx;
		data->e1 -= m;
		data->e2 -= n;
		if ( data->e1 < 0) {
			data->origy+=data->stepy;
			data->e1+=l;
		}
		if ( data->e2 < 0) {
      data->origz+=data->stepz;
      data->e2+=l;
		}
	} else if ( m >= l && m >= n ) {
		if ( data->origy == data->desty ) return true;
		data->origy+=data->stepy;
		data->e1 -= l;
		data->e2 -= n;
		if ( data->e1 < 0) {
			data->origx+=data->stepx;
			data->e1+=m;
		}
		if ( data->e2 < 0) {
			data->origz+=data->stepz;
			data->e2+=m;
		}
	} else {
	  if ( data->origz == data->destz ) return true;
		data->origz+=data->stepz;
		data->e1 -= l;
		data->e2 -= m;
		if ( data->e1 < 0) {
			data->origx+=data->stepx;
			data->e1+=n;
		}
		if ( data->e2 < 0) {
			data->origy+=data->stepy;
			data->e2+=n;
		}
	}
	*xCur=data->origx;
	*yCur=data->origy;
  *zCur=data->origz;
	return false;
}

bool TCOD_line3_mt(int xo, int yo, int zo, int xd, int yd, int zd, TCOD_line3_listener_t listener, TCOD_bresenham3_data_t *data) {
	TCOD_line3_init_mt(xo,yo,zo,xd,yd,zd,data);
	do {
		if (! listener(xo,yo,zo)) return false;
	} while (! TCOD_line3_step_mt(&xo,&yo,&zo,data));
	return true;
}

void TCOD_line3_init(int xFrom, int yFrom, int zFrom, int xTo, int yTo, int zTo) {
	TCOD_line3_init_mt(xFrom,yFrom,zFrom,xTo,yTo,zTo,&bresenham3_data);
}

bool TCOD_line3_step(int *xCur, int *yCur, int *zCur) {
	return TCOD_line3_step_mt(xCur,yCur,zCur,&bresenham3_data);
}

bool TCOD_line3(int xo, int yo, int zo, int xd, int yd, int zd, TCOD_line3_listener_t listener) {
	return TCOD_line3_mt(xo,yo,zo,xd,yd,zd,listener,&bresenham3_data);
}

