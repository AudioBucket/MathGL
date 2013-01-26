/***************************************************************************
 * surf.cpp is part of Math Graphic Library
 * Copyright (C) 2007-2012 Alexey Balakin <mathgl.abalakin@gmail.ru>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "mgl2/define.h"
#include "mgl2/surf.h"
#include "mgl2/data.h"
#include "mgl2/eval.h"
//-----------------------------------------------------------------------------
void MGL_NO_EXPORT mgl_mesh_plot(mglBase *gr, long *pos, long n, long m, int how)
{
	int d = gr->MeshNum>0 ? gr->MeshNum+1 : n*m, dx = n>d?n/d:1, dy = m>d?m/d:1;
	register long i,j,s;
	// NOTE: number of lines in each direction can be reduced too
	if(how&1)	for(j=0;j<m;j+=dy)
	{
		for(s=i=0;i<n-1;i++)	if(pos[n*j+i]>=0 && pos[n*j+i+1]>=0)	s++;
		d = gr->FaceNum>0 ? gr->FaceNum+1 : n;	s = s>d?s/d:1;
		for(i=0;i<n-s;i+=s)
			gr->line_plot(pos[n*j+i],pos[n*j+i+s]);

	}
	if(how&2)	for(i=0;i<n;i+=dx)
	{
		for(s=j=0;j<m-1;j++)	if(pos[n*j+i]>=0 && pos[n*j+i+n]>=0)	s++;
		d = gr->FaceNum>0 ? gr->FaceNum+1 : n;	s = s>d?s/d:1;
		for(j=0;j<m-s;j+=s)
			gr->line_plot(pos[n*j+i],pos[n*j+i+s*n]);
	}
}
//-----------------------------------------------------------------------------
void MGL_NO_EXPORT mgl_surf_plot(mglBase *gr, long *pos, long n, long m)
{
	register long i,j,s=0;
	for(j=0;j<m-1;j++)	for(i=0;i<n-1;i++)
		if(pos[n*j+i]>=0 && pos[n*j+i+1]>=0 && pos[n*j+i+n]>=0 && pos[n*j+i+n+1]>=0)
			s++;
	long dx=1,dy=1;
	if(gr->FaceNum && s>gr->FaceNum*gr->FaceNum)
	{
		int d = gr->FaceNum+1,ns=n*s/((n-1)*(m-1)),ms=m*s/((n-1)*(m-1));
		dx = ns>d?ns/d:1;		dy = ms>d?ms/d:1;
	}
	for(j=0;j<m-dy;j+=dy)	for(i=0;i<n-dx;i+=dx)
		gr->quad_plot(pos[n*j+i],pos[n*j+i+dx],pos[n*j+i+n*dy],pos[n*j+i+n*dy+dx]);
}
//-----------------------------------------------------------------------------
//
//	Plot by formulas series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fsurf(HMGL gr, const char *eqZ, const char *sch, const char *opt)
{	// TODO: Add strong function variation analysis ???
	if(eqZ==0 || eqZ[0]==0)	return;		// nothing to plot
	mreal r = gr->SaveState(opt);
	long n = (mgl_isnan(r) || r<=0) ? 100:long(r+0.5);
	mglData z(n,n);
	mglFormula *eq = new mglFormula(eqZ);
	register int i,j;
	mreal dx = (gr->Max.x - gr->Min.x)/(n-1.), dy = (gr->Max.y - gr->Min.y)/(n-1.);
	for(j=0;j<n;j++)	for(i=0;i<n;i++)
	{
		if(gr->Stop)	{	delete eq;	return;	}
		z.a[i+n*j] = eq->Calc(gr->Min.x+i*dx, gr->Min.y+j*dy);
	}
	mgl_surf(gr, &z, sch,0);
	delete eq;
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fsurf_xyz(HMGL gr, const char *eqX, const char *eqY, const char *eqZ, const char *sch, const char *opt)
{	// TODO: Add strong function variation analisys ???
	if(eqZ==0 || eqZ[0]==0)	return;		// nothing to plot
	mreal r = gr->SaveState(opt);
	long n = (mgl_isnan(r) || r<=0) ? 100:long(r+0.5);
	mglData x(n,n), y(n,n), z(n,n);
	if(n<=0)	n=100;
	mglFormula *ex, *ey, *ez;
	ex = new mglFormula(eqX ? eqX : "u");
	ey = new mglFormula(eqY ? eqY : "v");
	ez = new mglFormula(eqZ);
	register int i,j;
	register mreal u,v;
	for(j=0;j<n;j++)	for(i=0;i<n;i++)
	{
		if(gr->Stop)	{	delete ex;	delete ey;	delete ez;	return;	}
		v = i/(n-1.);	u = j/(n-1.);
		x.a[i+n*j] = ex->Calc(0,v,0,u);
		y.a[i+n*j] = ey->Calc(0,v,0,u);
		z.a[i+n*j] = ez->Calc(0,v,0,u);
	}
	mgl_surf_xy(gr,&x,&y,&z,sch,0);
	delete ex;	delete ey;	delete ez;
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fsurf_(uintptr_t *gr, const char *fy, const char *stl, const char *opt, int ly, int ls, int lo)
{	char *s=new char[ly+1];	memcpy(s,fy,ly);	s[ly]=0;
	char *p=new char[ls+1];	memcpy(p,stl,ls);	p[ls]=0;
	char *o=new char[lo+1];	memcpy(o,opt,lo);	o[lo]=0;
	mgl_fsurf(_GR_, s, p, o);	delete []o;	delete []s;	delete []p;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fsurf_xyz_(uintptr_t *gr, const char *fx, const char *fy, const char *fz, const char *stl, const char *opt, int lx, int ly, int lz, int ls, int lo)
{
	char *sx=new char[lx+1];	memcpy(sx,fx,lx);	sx[lx]=0;
	char *sy=new char[ly+1];	memcpy(sy,fy,ly);	sy[ly]=0;
	char *sz=new char[lz+1];	memcpy(sz,fz,lz);	sz[lz]=0;
	char *p=new char[ls+1];		memcpy(p,stl,ls);	p[ls]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_fsurf_xyz(_GR_, sx, sy, sz, p, o);	delete []o;
	delete []sx;	delete []sy;	delete []sz;	delete []p;
}
//-----------------------------------------------------------------------------
//
//	Mesh series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_mesh_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Mesh"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Mesh",cgid++);
	gr->SetPenPal("-");
	long ss = gr->AddTexture(sch);
	long *pos = new long[n*m];
	gr->Reserve(n*m*z->GetNz());

	mglPoint p;
	mreal c;
	for(k=0;k<z->GetNz();k++)
	{
		for(j=0;j<m;j++)	for(i=0;i<n;i++)
		{
			if(gr->Stop)	{	delete []pos;	return;	}
			p = mglPoint(GetX(x,i,j,k).x, GetY(y,i,j,k).x, z->v(i,j,k));
			c = gr->GetC(ss,p.z);		pos[i+n*j] = gr->AddPnt(p,c);
		}
		mgl_mesh_plot(gr,pos,n,m,3);
	}
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_mesh(HMGL gr, HCDT z, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_mesh_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_mesh_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_mesh_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_mesh_(uintptr_t *gr, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_mesh(_GR_, _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Fall series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fall_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Fall"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Fall",cgid++);
	gr->SetPenPal("-");
	long ss = gr->AddTexture(sch);
	long *pos = new long[n*m];
	gr->Reserve(n*m*z->GetNz());

	mglPoint p;
	mreal c;
	for(k=0;k<z->GetNz();k++)
	{
		for(j=0;j<m;j++)	for(i=0;i<n;i++)
		{
			if(gr->Stop)	{	delete []pos;	return;	}
			p = mglPoint(GetX(x,i,j,k).x, GetY(y,i,j,k).x, z->v(i,j,k));
			c = gr->GetC(ss,p.z);	pos[i+n*j] = gr->AddPnt(p,c);
		}
		mgl_mesh_plot(gr,pos,n,m, (mglchr(sch,'x')) ? 2:1);
	}
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fall(HMGL gr, HCDT z, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_fall_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fall_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_fall_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_fall_(uintptr_t *gr, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_fall(_GR_, _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Grid series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_grid_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Grid"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Grid",cgid++);
	mreal	zVal = gr->Min.z;
	gr->SetPenPal(sch?sch:"k-");
	long *pos = new long[n*m];
	gr->Reserve(n*m*z->GetNz());

	mglPoint p;
	for(k=0;k<z->GetNz();k++)
	{
		if(z->GetNz()>1)	zVal = gr->Min.z+(gr->Max.z-gr->Min.z)*mreal(k)/(z->GetNz()-1);
		for(j=0;j<m;j++)	for(i=0;i<n;i++)
		{
			if(gr->Stop)	{	delete []pos;	return;	}
			p = mglPoint(GetX(x,i,j,k).x, GetY(y,i,j,k).x, zVal);
			pos[i+n*j] = gr->AddPnt(p,gr->CDef);
		}
		mgl_mesh_plot(gr,pos,n,m,3);
	}
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_grid(HMGL gr, HCDT z,const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_grid_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_grid_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_grid_xy(_GR_,_DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_grid_(uintptr_t *gr, uintptr_t *a,const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_grid(_GR_, _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Surf series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surf_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Surf"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Surf",cgid++);
	long ss = gr->AddTexture(sch);
	long *pos = new long[n*m];
	bool wire = (mglchr(sch,'#'));
	gr->Reserve(n*m*z->GetNz()*(wire?2:1));

	mglPoint p,q,s,xx,yy;
	mreal c;
	for(k=0;k<z->GetNz();k++)
	{
		for(j=0;j<m;j++)	for(i=0;i<n;i++)
		{
			if(gr->Stop)	{	delete []pos;	return;	}
			xx = GetX(x,i,j,k);		yy = GetY(y,i,j,k);
			p = mglPoint(xx.x, yy.x, z->v(i,j,k));
			q = mglPoint(xx.y, yy.y, z->dvx(i,j,k));
			s = mglPoint(xx.z, yy.z, z->dvy(i,j,k));
			c = gr->GetC(ss,p.z);
			pos[i+n*j] = gr->AddPnt(p,c,q^s);
		}
		if(sch && mglchr(sch,'.'))	for(i=0;i<n*m;i++)	gr->mark_plot(pos[i],'.');
		else	mgl_surf_plot(gr,pos,n,m);
		if(wire)
		{
			gr->SetPenPal("k-");
			for(i=0;i<n*m;i++)	pos[i] = gr->CopyNtoC(pos[i],gr->CDef);
			mgl_mesh_plot(gr,pos,n,m,3);
		}
	}
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surf(HMGL gr, HCDT z, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_surf_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surf_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_surf_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surf_(uintptr_t *gr, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_surf(_GR_, _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Belt series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_belt_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Belt"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Belt",cgid++);
	int d = gr->MeshNum>0 ? gr->MeshNum+1 : n*m, dx = n>d?n/d:1, dy = m>d?m/d:1;
	long ss = gr->AddTexture(sch);
	long *pos = new long[2*(n>m?n:m)];
	gr->Reserve(2*n*m*z->GetNz());
	bool how = !mglchr(sch,'x');

	mglPoint p1,p2,q,s,xx,yy;
	mreal c;
	for(k=0;k<z->GetNz();k++)
	{
		if(how)	for(i=0;i<n-dx;i+=dx)
		{
			for(j=0;j<m;j++)
			{
				if(gr->Stop)	{	delete []pos;	return;	}
				xx = GetX(x,i,j,k);		yy = GetY(y,i,j,k);
				p1 = mglPoint(xx.x, yy.x, z->v(i,j,k));
				s = mglPoint(xx.z, yy.z, z->dvy(i,j,k));
				q = mglPoint(xx.y, yy.y, 0);	s = q^s;
				c = gr->GetC(ss,p1.z);
				p2 = mglPoint(GetX(x,i+dx,j,k).x,GetY(y,i+dx,j,k).x,p1.z);
				pos[2*j] = gr->AddPnt(p1,c,s);
				pos[2*j+1]=gr->AddPnt(p2,c,s);
			}
			mgl_surf_plot(gr,pos,2,m);
		}
		else	for(j=0;j<m-dy;j+=dy)
		{
			for(i=0;i<n;i++)	// ñîçäàåì ìàññèâ òî÷åê
			{
				if(gr->Stop)	{	delete []pos;	return;	}
				xx = GetX(x,i,j,k);		yy = GetY(y,i,j,k);
				p1 = mglPoint(xx.x, yy.x, z->v(i,j,k));
				q = mglPoint(xx.y, yy.y, z->dvx(i,j,k));
				s = mglPoint(xx.z, yy.z, 0);	s = q^s;
				c = gr->GetC(ss,p1.z);
				p2 = mglPoint(GetX(x,i,j+dy,k).x,GetY(y,i,j+dy,k).x,p1.z);
				pos[2*i] = gr->AddPnt(p1,c,s);
				pos[2*i+1]=gr->AddPnt(p2,c,s);
			}
			mgl_surf_plot(gr,pos,2,n);
		}
	}
	delete []pos; gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_belt(HMGL gr, HCDT z, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_belt_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_belt_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_belt_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_belt_(uintptr_t *gr, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];	memcpy(o,opt,lo);	o[lo]=0;
	mgl_belt(_GR_, _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Dens series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_dens_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Dens"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Dens",cgid++);
	mreal	zVal = gr->Min.z;

	long ss = gr->AddTexture(sch);
	long *pos = new long[n*m];
	gr->Reserve(n*m*z->GetNz());

	mglPoint p,s=mglPoint(0,0,1);
	mreal zz, c;
	for(k=0;k<z->GetNz();k++)
	{
		if(z->GetNz()>1)
			zVal = gr->Min.z+(gr->Max.z-gr->Min.z)*mreal(k)/(z->GetNz()-1);
		for(j=0;j<m;j++)	for(i=0;i<n;i++)	// ñîçäàåì ìàññèâ òî÷åê
		{
			if(gr->Stop)	{	delete []pos;	return;	}
			p = mglPoint(GetX(x,i,j,k).x, GetY(y,i,j,k).x, zVal);
			zz = z->v(i,j,k);	c = gr->GetC(ss,zz);
			if(mgl_isnan(zz))	p.x = NAN;
			pos[i+n*j] = gr->AddPnt(p,c,s);
		}
		if(sch && mglchr(sch,'.'))	for(i=0;i<n*m;i++)	gr->mark_plot(pos[i],'.');
		else	mgl_surf_plot(gr,pos,n,m);
		if(mglchr(sch,'#'))
		{
			gr->Reserve(n*m);	gr->SetPenPal("k-");
			for(i=0;i<n*m;i++)
				pos[i] = gr->CopyNtoC(pos[i],gr->CDef);
			mgl_mesh_plot(gr,pos,n,m,3);
		}
	}
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_dens(HMGL gr, HCDT z, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x, gr->Max.x);
	y.Fill(gr->Min.y, gr->Max.y);
	mgl_dens_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_dens_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_dens_xy(_GR_,_DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_dens_(uintptr_t *gr, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_dens(_GR_,_DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	STFA series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_stfa_xy(HMGL gr, HCDT x, HCDT y, HCDT re, HCDT im, int dn, const char *sch, const char *opt)
{	mglData tmp(mglSTFA(*re,*im,dn,'x'));	mgl_dens_xy(gr,x,y,&tmp,sch,opt);	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_stfa(HMGL gr, HCDT re, HCDT im, int dn, const char *sch, const char *opt)
{	mglData tmp(mglSTFA(*re,*im,dn,'x'));	mgl_dens(gr,&tmp,sch,opt);	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_stfa_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *re, uintptr_t *im, int *dn, const char *sch, const char *opt, int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_stfa_xy(_GR_,_DA_(x), _DA_(y), _DA_(re), _DA_(im), *dn, s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_stfa_(uintptr_t *gr, uintptr_t *re, uintptr_t *im, int *dn, const char *sch, const char *opt, int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_stfa(_GR_,_DA_(re), _DA_(im), *dn, s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	SurfC series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfc_xy(HMGL gr, HCDT x, HCDT y, HCDT z, HCDT c, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,c,"SurfC"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("SurfC",cgid++);
	long ss = gr->AddTexture(sch);
	long *pos = new long[n*m];
	gr->Reserve(n*m*z->GetNz());
	mreal col;

	mglPoint p,q,s,xx,yy;
	for(k=0;k<z->GetNz();k++)
	{
		for(j=0;j<m;j++)	for(i=0;i<n;i++)
		{
			if(gr->Stop)	{	delete []pos;	return;	}
			xx = GetX(x,i,j,k);		yy = GetY(y,i,j,k);
			p = mglPoint(xx.x, yy.x, z->v(i,j,k));
			q = mglPoint(xx.y, yy.y, z->dvx(i,j,k));
			s = mglPoint(xx.z, yy.z, z->dvy(i,j,k));
			col = gr->GetC(ss,c->v(i,j,k));
			pos[i+n*j] = gr->AddPnt(p,col,q^s);
		}
		if(sch && mglchr(sch,'.'))	for(i=0;i<n*m;i++)	gr->mark_plot(pos[i],'.');
		else	mgl_surf_plot(gr,pos,n,m);
		if(mglchr(sch,'#'))
		{
			gr->Reserve(n*m);	gr->SetPenPal("k-");
			for(i=0;i<n*m;i++)	pos[i] = gr->CopyNtoC(pos[i],gr->CDef);
			mgl_mesh_plot(gr,pos,n,m,3);
		}
	}
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfc(HMGL gr, HCDT z, HCDT c, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_surfc_xy(gr,&x,&y,z,c,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfc_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *z, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_surfc_xy(_GR_, _DA_(x), _DA_(y), _DA_(z), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfc_(uintptr_t *gr, uintptr_t *z, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_surfc(_GR_, _DA_(z), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	SurfA series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfa_xy(HMGL gr, HCDT x, HCDT y, HCDT z, HCDT c, const char *sch, const char *opt)
{
	register long i,j;
	long k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,c,"SurfA"))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("SurfA",cgid++);
	long ss = gr->AddTexture(sch);
	long *pos = new long[n*m];
	gr->Reserve(n*m*z->GetNz());

	mglPoint p,q,s,xx,yy;
	for(k=0;k<z->GetNz();k++)
	{
		for(j=0;j<m;j++)	for(i=0;i<n;i++)
		{
			if(gr->Stop)	{	delete []pos;	return;	}
			xx = GetX(x,i,j,k);		yy = GetY(y,i,j,k);
			mreal vv = z->v(i,j,k);	p = mglPoint(xx.x, yy.x, vv);
			q = mglPoint(xx.y, yy.y, z->dvx(i,j,k));
			s = mglPoint(xx.z, yy.z, z->dvy(i,j,k));
			pos[i+n*j] = gr->AddPnt(p,gr->GetC(ss,vv),q^s,gr->GetA(c->v(i,j,k)));
		}
		if(sch && mglchr(sch,'.'))	for(i=0;i<n*m;i++)	gr->mark_plot(pos[i],'.');
		else	mgl_surf_plot(gr,pos,n,m);
		if(mglchr(sch,'#'))
		{
			gr->Reserve(n*m);	gr->SetPenPal("k-");
			for(i=0;i<n*m;i++)	pos[i] = gr->CopyNtoC(pos[i],gr->CDef);
			mgl_mesh_plot(gr,pos,n,m,3);
		}
	}
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfa(HMGL gr, HCDT z, HCDT c, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()), y(z->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_surfa_xy(gr,&x,&y,z,c,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfa_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *z, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_surfa_xy(_GR_, _DA_(x), _DA_(y), _DA_(z), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_surfa_(uintptr_t *gr, uintptr_t *z, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_surfa(_GR_, _DA_(z), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Boxs series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_boxs_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Boxs",true))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Boxs",cgid++);
	long ly = y->GetNy()>=m ? y->GetNy() : y->GetNx(), lx = x->GetNx();
	int d = gr->MeshNum>0 ? gr->MeshNum+1 : n*m, dx = n>d?n/d:1, dy = m>d?m/d:1;

	long ss = gr->AddTexture(sch);
	bool wire = mglchr(sch,'#');
	bool full = mglchr(sch,'@');
	gr->Reserve(8*n*m*z->GetNz());

	mglPoint p1,p2,p3,p4,q,s,t(wire||full?NAN:0,0,1),xx,yy;
	mreal zz,z1,z2,x1,y1,x2,y2,x3,y3,c,z0=gr->GetOrgZ('x');
	long k1,k2,k3,k4,k5,k6,k7,k8;
	for(k=0;k<z->GetNz();k++)
	{
		for(i=0;i<n;i+=dx)	for(j=0;j<m;j+=dy)
		{
			if(gr->Stop)	return;
			zz = z->v(i,j,k);		c  = gr->GetC(ss,zz);
			xx = GetX(x,i,j,k);		yy = GetY(y,i,j,k);
			x1 = i<lx-dx ? GetX(x,i+dx,j,k).x:NAN;
			y1 = i<lx-dx ? GetY(y,i+dx,j,k).x:NAN;
			x2 = j<ly-dy ? GetX(x,i,j+dy,k).x:NAN;
			y2 = j<ly-dy ? GetY(y,i,j+dy,k).x:NAN;
			x3 = i<lx-dx && j<ly-dy ? GetX(x,i+dx,j+dy,k).x:NAN;
			y3 = i<lx-dx && j<ly-dy ? GetY(y,i+dx,j+dy,k).x:NAN;
			z1 = i<n-dx?z->v(i+dx,j,k):NAN;
			z2 = j<m-dy?z->v(i,j+dy,k):NAN;
			q = mglPoint(xx.y,yy.y,0);
			s = mglPoint(xx.z,yy.z,0);
			p1 = mglPoint(xx.x,yy.x,zz);	k1 = gr->AddPnt(p1,c,t);
			p2 = mglPoint(x1,y1,zz);		k2 = gr->AddPnt(p2,c,t);
			p3 = mglPoint(x2,y2,zz);		k3 = gr->AddPnt(p3,c,t);
			p4 = mglPoint(x3,y3,zz);		k4 = gr->AddPnt(p4,c,t);
			if(wire)
			{
				gr->line_plot(k1,k2);	gr->line_plot(k1,k3);
				gr->line_plot(k4,k2);	gr->line_plot(k4,k3);
			}
			else	gr->quad_plot(k1,k2,k3,k4);

			if(full)
			{
				p1 = mglPoint(xx.x,yy.x,z0);	k5 = gr->AddPnt(p1,c,t);
				p2 = mglPoint(x1,y1,z0);		k6 = gr->AddPnt(p2,c,t);
				p3 = mglPoint(x2,y2,z0);		k7 = gr->AddPnt(p3,c,t);
				p4 = mglPoint(x3,y3,z0);		k8 = gr->AddPnt(p4,c,t);
				if(wire)
				{
					gr->line_plot(k5,k6);	gr->line_plot(k5,k7);
					gr->line_plot(k8,k6);	gr->line_plot(k8,k7);
					gr->line_plot(k1,k5);	gr->line_plot(k3,k7);
					gr->line_plot(k2,k6);	gr->line_plot(k4,k8);
				}
				else
				{
					gr->quad_plot(k1,k2,k5,k6);	gr->quad_plot(k1,k3,k5,k7);
					gr->quad_plot(k4,k2,k8,k6);	gr->quad_plot(k4,k3,k8,k7);
					gr->quad_plot(k5,k6,k7,k8);
				}
			}
			else
			{
				p3 = mglPoint(x1,y1,z1);		k5 = gr->AddPnt(p3,c,wire?t:q);
				p4 = mglPoint(x3,y3,z1);		k6 = gr->AddPnt(p4,c,wire?t:q);
				if(wire)
				{	gr->line_plot(k2,k5);	gr->line_plot(k6,k5);	gr->line_plot(k6,k4);	}
				else	gr->quad_plot(k2,k4,k5,k6);
				p3 = mglPoint(x2,y2,z2);		k7 = gr->AddPnt(p3,c,wire?t:s);
				p4 = mglPoint(x3,y3,z2);		k8 = gr->AddPnt(p4,c,wire?t:s);
				if(wire)
				{	gr->line_plot(k3,k7);	gr->line_plot(k4,k8);	gr->line_plot(k7,k8);	}
				else	gr->quad_plot(k3,k4,k7,k8);
			}
		}
	}
	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_boxs(HMGL gr, HCDT z, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()+1), y(z->GetNy()+1);
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_boxs_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_boxs_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_boxs_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_boxs_(uintptr_t *gr, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_boxs(_GR_, _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Tile series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tile_xy(HMGL gr, HCDT x, HCDT y, HCDT z, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,0,"Tile",true))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Tile",cgid++);
	long ly = x->GetNy()>=z->GetNy() ? y->GetNy() : y->GetNx(), lx = x->GetNx();
	int d = gr->MeshNum>0 ? gr->MeshNum+1 : n*m, dx = n>d?n/d:1, dy = m>d?m/d:1;

	long ss = gr->AddTexture(sch);
	gr->Reserve(4*n*m*z->GetNz());

	mglPoint p1,p2,p3,p4,s=mglPoint(0,0,1);
	mreal zz,x0,y0,x1,y1,x2,y2,x3,y3,c;
	long k1,k2,k3,k4;
	for(k=0;k<z->GetNz();k++)
	{
		for(j=0;j<m;j+=dx)	for(i=0;i<n;i+=dy)
		{
			if(gr->Stop)	return;
			zz = z->v(i,j,k);		c = gr->GetC(ss,zz);
			x0 = GetX(x,i,j,k).x;	y0 = GetY(y,i,j,k).x;
			x1 = i<lx-dx ? GetX(x,i+dx,j,k).x:NAN;
			y1 = i<lx-dx ? GetY(y,i+dx,j,k).x:NAN;
			x2 = j<ly-dy ? GetX(x,i,j+dy,k).x:NAN;
			y2 = j<ly-dy ? GetY(y,i,j+dy,k).x:NAN;
			x3 = i<lx-dx && j<ly-dy ? GetX(x,i+dx,j+dy,k).x:NAN;
			y3 = i<lx-dx && j<ly-dy ? GetY(y,i+dx,j+dy,k).x:NAN;
			p1 = mglPoint(x0,y0,zz);	k1 = gr->AddPnt(p1,c,s);
			p2 = mglPoint(x1,y1,zz);	k2 = gr->AddPnt(p2,c,s);
			p3 = mglPoint(x2,y2,zz);	k3 = gr->AddPnt(p3,c,s);
			p4 = mglPoint(x3,y3,zz);	k4 = gr->AddPnt(p4,c,s);
			gr->quad_plot(k1,k2,k3,k4);
		}
	}
	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tile(HMGL gr, HCDT z, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()+1), y(z->GetNy()+1);
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_tile_xy(gr,&x,&y,z,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tile_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_tile_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tile_(uintptr_t *gr, uintptr_t *a, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_tile(_GR_, _DA_(a), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	TileS series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tiles_xy(HMGL gr, HCDT x, HCDT y, HCDT z, HCDT s, const char *sch, const char *opt)
{
	register long i,j,k,n=z->GetNx(),m=z->GetNy();
	if(mgl_check_dim2(gr,x,y,z,s,"TileS",true))	return;

	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("TileS",cgid++);
	long ly = x->GetNy()>=z->GetNy() ? y->GetNy() : y->GetNx(), lx = x->GetNx();
	int d = gr->MeshNum>0 ? gr->MeshNum+1 : n*m, dx = n>d?n/d:1, dy = m>d?m/d:1;

	long cc = gr->AddTexture(sch);
	gr->Reserve(4*n*m*z->GetNz());

	mglPoint p1,p2,p3,p4,t=mglPoint(0,0,1);
	mreal zz,x1,x2,x3,x4,y1,y2,y3,y4,ss,sm,c;
	long k1,k2,k3,k4;
	for(k=0;k<z->GetNz();k++)
	{
		for(j=0;j<m;j+=dx)	for(i=0;i<n;i+=dy)
		{
			if(gr->Stop)	return;
			zz = z->v(i,j,k);	c = gr->GetC(cc,zz);
			ss = (1-gr->GetA(s->v(i,j,k)))/2;	sm = 1-ss;

			x1 = GetX(x,i,j,k).x;	y1 = GetY(y,i,j,k).x;
			x2 = x3 = x4 = y2 = y3 = y4 = NAN;
			if(i<lx-dx)
			{	x2 = GetX(x,i+dx,j,k).x-x1;	y2 = GetY(y,i+dx,j,k).x-y1;	}
			if(j<ly-dy)
			{	x4 = GetX(x,i,j+dy,k).x-x1;	y4 = GetY(y,i,j+dy,k).x-y1;	}
			if(i<lx-dx && j<ly-dy)
			{	x3 = GetX(x,i+dx,j+dy,k).x-x2-x4-x1;
				y3 = GetY(y,i+dx,j+dy,k).x-y2-y4-y1;	}

			p1 = mglPoint(x1+x2*ss+x4*ss+x3*ss*ss, y1+y2*ss+y4*ss+y3*ss*ss, zz);
			k1 = gr->AddPnt(p1,c,t);
			p2 = mglPoint(x1+x2*sm+x4*ss+x3*ss*sm, y1+y2*sm+y4*ss+y3*ss*sm, zz);
			k2 = gr->AddPnt(p2,c,t);
			p3 = mglPoint(x1+x2*ss+x4*sm+x3*ss*sm, y1+y2*ss+y4*sm+y3*ss*sm, zz);
			k3 = gr->AddPnt(p3,c,t);
			p4 = mglPoint(x1+x2*sm+x4*sm+x3*sm*sm, y1+y2*sm+y4*sm+y3*sm*sm, zz);
			k4 = gr->AddPnt(p4,c,t);
			gr->quad_plot(k1,k2,k3,k4);
		}
	}
	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tiles(HMGL gr, HCDT z, HCDT s, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(z->GetNx()+1), y(z->GetNy()+1);
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_tiles_xy(gr,&x,&y,z,s,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tiles_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, uintptr_t *r, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_tiles_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), _DA_(r), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_tiles_(uintptr_t *gr, uintptr_t *a, uintptr_t *r, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_tiles(_GR_, _DA_(a), _DA_(r), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
//
//	Map series
//
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_map_xy(HMGL gr, HCDT x, HCDT y, HCDT ax, HCDT ay, const char *sch, const char *opt)
{
	register long i,j,n=ax->GetNx(),m=ax->GetNy();
	if(mgl_check_dim2(gr,x,y,ax,ay,"Map"))	return;

	bool both = x->GetNx()==n && y->GetNx()==n && x->GetNy()==m && y->GetNy()==m;
	gr->SaveState(opt);
	static int cgid=1;	gr->StartGroup("Map",cgid++);

	long ss = gr->AddTexture(mgl_have_color(sch)?sch:"rgb",2);
	long s = both ? n:1, s1, s2;

	mreal xdy,xdx,ydx,ydy,xx,yy;
	mglPoint p,t=mglPoint(NAN);
	long *pos = new long[n*m];
	gr->Reserve(n*m);

	for(j=0;j<m;j++)	for(i=0;i<n;i++)
	{
		if(gr->Stop)	{	delete []pos;	return;	}
		s1 = i>0 ? 1:0;		s2 = i<n-1 ? 1:0;
		xdx = (ax->v(i+s2,j)-ax->v(i-s1,j))/(GetX(x,i+s2,j).x-GetX(x,i-s1,j).x);
		ydx = (ay->v(i+s2,j)-ay->v(i-s1,j))/(GetX(x,i+s2,j).x-GetX(x,i-s1,j).x);
		s1 = j>0 ? s:0;		s2 = j<m-1 ? s:0;
		xdy = (ax->v(i,j+s2)-ax->v(i,j-s1))/(GetY(y,i,j+s2).x-GetY(y,i,j-s1).x);
		ydy = (ay->v(i,j+s2)-ay->v(i,j-s1))/(GetY(y,i,j+s2).x-GetY(y,i,j-s1).x);
		xdx = xdx*ydy - xdy*ydx;	// Jacobian

		p = mglPoint(ax->v(i,j), ay->v(i,j), xdx);
		if(both)
		{
			xx = (x->v(i,j) - gr->Min.x)/(gr->Max.x - gr->Min.x);
			yy = (y->v(i,j) - gr->Min.y)/(gr->Max.y - gr->Min.y);
		}
		else
		{
			xx = (x->v(i) - gr->Min.x)/(gr->Max.x - gr->Min.x);
			yy = (y->v(j) - gr->Min.y)/(gr->Max.y - gr->Min.y);
		}
		if(xx<0)	xx=0;	if(xx>=1)	xx=1/MGL_FEPSILON;
		if(yy<0)	yy=0;	if(yy>=1)	yy=1/MGL_FEPSILON;
		pos[i+n*j] = gr->AddPnt(p,gr->GetC(ss,xx,false),t,yy);
	}
	if(sch && mglchr(sch,'.'))	for(i=0;i<n*m;i++)	gr->mark_plot(pos[i],'.');
	else	mgl_surf_plot(gr,pos,n,m);
	delete []pos;	gr->EndGroup();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_map(HMGL gr, HCDT ax, HCDT ay, const char *sch, const char *opt)
{
	gr->SaveState(opt);
	mglData x(ax->GetNx()), y(ax->GetNy());
	x.Fill(gr->Min.x,gr->Max.x);
	y.Fill(gr->Min.y,gr->Max.y);
	mgl_map_xy(gr,&x,&y,ax,ay,sch,0);
	gr->LoadState();
}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_map_xy_(uintptr_t *gr, uintptr_t *x, uintptr_t *y, uintptr_t *a, uintptr_t *b, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_map_xy(_GR_, _DA_(x), _DA_(y), _DA_(a), _DA_(b), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
void MGL_EXPORT mgl_map_(uintptr_t *gr, uintptr_t *a, uintptr_t *b, const char *sch, const char *opt,int l,int lo)
{	char *s=new char[l+1];	memcpy(s,sch,l);	s[l]=0;
	char *o=new char[lo+1];		memcpy(o,opt,lo);	o[lo]=0;
	mgl_map(_GR_, _DA_(a), _DA_(b), s, o);	delete []o;	delete []s;	}
//-----------------------------------------------------------------------------
