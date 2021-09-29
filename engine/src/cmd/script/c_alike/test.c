/**
* test.c
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

module test {

globals {
  class int g1,g2,g5,g6;
  float g3;
  object g4;
};

import a1;

class int aty:=34.3;
int arss=4.3;

 void t0(){
   float x=a*b+c*d+e;
   float y=a*b+c+d*e/f-g*q;

   float x2=b+!a*b+!c-d;
   float x3=!a+b==!b;

 };

 void t1(){
   int blah="whatever";
   a="whatever"+"nothing"+_io.print("never","ever");

   float b1=a*b;
   float b2:=3.4;
   a->push(a,b,cd);
   
 };

 int m1;
 //int ms1;
 float sdm2;

 int m321;
 /* int ms1;
    float ms234;
 */
 float sdm2;


bool blah:=true;

 float v,blubb,schnupf,blahs,df,a,vf,b,c;
 
 void loop(){
   if(a){
     int blah;
   };

   if(a){
     int sdhfj;
   }
   else {
     int sdjk;
   };

   if(a){
     int sdhfj;
   }
   else if(b) {
     int sdjk;
   };

 };

 void loop2(){
   if(a){
     int blah;
   }

   if(a){
     int sdhfj;
   }
   else {
     int sdjk;
   }

   if(a){
     int sdhfj;
   }
   else if(b) {
     int sdjk;
   };

 };

 void gram(void d){};

  float first(int a,float b) { 
    _io.three(:s1="flubb"; :s2="blah" ;a,b,c,d);
    _io.qu();
    _io.js1(a);
    _io.js2(a,b);
    _io.js3(:s1="blah";a,b,c);
    _io.js4(:s1="blah";:s2="fghdg";a,b,c);
    _io.js5(:s1="fsjf";);

    return a;
    blubb=four(a);
  };


  float second(int a1,float d3) {
    int hallo;
    float fasel;

	if(whatever==soever) {
		int h2;
		float b3;

		b3=13.3;
		b34=(sowhat==nover);

		if(nowhat+never==323.4){
		  return;
		}
		else{
		}

		a46=34.2 + 3.4;

		a3784=37+4278.54*dh+4.5;
	}
	else {
		int h3;
		float h4;
	};

  };

}
