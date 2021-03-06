/***************************************************************************
 * mgl2png.cpp is part of Math Graphic Library
 * Copyright (C) 2007 Alexey Balakin <balakin@appl.sci-nnov.ru>            *
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
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "mgl/mgl_zb.h"
#include "mgl/mgl_parse.h"
//-----------------------------------------------------------------------------
int main(int narg, char **arg)
{
	mglGraphZB gr;
	mglParse p(true);

	char str[8192], buf[2048];
	long i,j=-1,k=-1;
	for(i=1;i<narg;i++)	// add arguments for the script
	{
		if(arg[i][0]=='-' && arg[i][1]>='0' && arg[i][1]<='9')
			p.AddParam(arg[i][1]-'0',arg[i]+2);
		if(arg[i][0]=='-' && arg[i][1]=='L')
			setlocale(LC_CTYPE, arg[i]+2);
		if(arg[i][0]=='-' && (arg[i][1]=='h' || (arg[i][1]=='-' && arg[i][2]=='h')))
		{
			printf("mgl2png convert mgl script to bitmap png file.\n");
			printf("Current version is 1.%g\n",MGL_VERSION);
			printf("Usage:\tmgl2png scriptfile [outputfile parameter(s)]\n");
			printf("\tParameters have format \"-Nval\".\n");
			printf("\tHere N=0,1...9 is parameter ID and val is its value.\n");
			printf("\tOption -Lval set locale to val.\n");
		}
		if(arg[i][0]!='-' && j<0)	j=i;
		if(arg[i][0]!='-' && j>0)	k=i;
	}
	gr.Message = buf;	*buf=0;
	FILE *fp = j>0?fopen(arg[j],"r"):stdin;
	p.Execute(&gr,fp,true);
	if(j>0)	fclose(fp);
	if(k>0)	strcpy(str,arg[k]);
	else
	{
		strcpy(str,j>0?arg[j]:"out");	strcat(str,".png");
		printf("Write output to %s\n",str);
	}
	gr.WritePNG(str);
	return 0;
}
//-----------------------------------------------------------------------------
