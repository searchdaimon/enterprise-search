/*
    ppt2txt (C) Magnus Gal√•en <mg@searchdaimon.com>
 
   libppt - library to read PowerPoint presentation
   Copyright (C) 2005 Yolla Indria <yolla.indria@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA
*/

#include "libppt.h"

#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;
using namespace Libppt;


bool textonly;


void recursiveSearch( GroupObject* group, ofstream &out )
{
    if (!group) return;

    for( unsigned i=0; i<group->objectCount(); i++ )
        {
	    Object* object = group->object(i);
	    if( object->isText() )
	        {
	    	    TextObject* T = static_cast<TextObject*>(object);
		    if(T)
			{
//			    out << "  " << T->typeAsString() << " (" << T->listSize() << "): ";

			    bool	written = false;
			    for (int j=0; j<T->listSize(); j++)
				{
				    if (T->text(j).cstring().c_str()!=NULL
					&& T->text(j).cstring().c_str()[0]!='\0'
					&& strncmp(T->text(j).cstring().c_str(),"Click to edit",13)
					&& strncmp(T->text(j).cstring().c_str(),"Klikk for Â redigere",20))
					{
					    if (!written)
						{
						    if (!textonly)
							{
							    if (T->type()==TextObject::Body || T->type()==TextObject::CenterBody
								|| T->type()==TextObject::HalfBody || T->type()==TextObject::QuarterBody)
								out << "<p>";
							    else if (T->type()==TextObject::Title || T->type()==TextObject::CenterTitle)
								out << "<h2>";
							    else out << "<div>";
							}

						    written = true;
						}

					    if (!textonly) out << "<span>";
					    out << T->text(j).cstring().c_str();
					    if (!textonly) out << "</span> ";
					}
				}

			    if (written && !textonly)
				{
				    if (T->type()==TextObject::Body || T->type()==TextObject::CenterBody
					|| T->type()==TextObject::HalfBody || T->type()==TextObject::QuarterBody)
					out << "</p>";
				    else if (T->type()==TextObject::Title || T->type()==TextObject::CenterTitle)
					out << "</h2>";
				    else out << "</div>";
				    out << endl;
				}
			    else
				out << endl;
			}
    		}

	    if( object->isGroup() )
	        recursiveSearch( static_cast<GroupObject*>(object), out );
	}
}


int main( int argc, char* argv[] )
{
    int		argnr = 1;

    if (argc < 3)
	{
	    cerr << "Usage: " << argv[0] << " [-t] <input.ppt> <output.html>" << endl;
	    return -1;
	}

    textonly = false;

    if (!strcmp(argv[argnr], "-t"))
	{
	    textonly = true;
	    argnr++;
	}

    Presentation	*P = new Presentation();

    if (!P->load( argv[argnr++] ))
	{
	    cerr << "Error: Could not load presentation." << endl;
	    delete P;
	    return -1;
	}

    int		numSlides = P->slideCount();
    Slide	*S;

    cout << numSlides << " slides." << endl;

    ofstream	out(argv[argnr++]);

    if (!out)
	{
	    cerr << "Error: Could not open file for writing." << endl;
	    delete P;
	    return -1;
	}

    for (int i=0; i<numSlides; i++)
	{
	    UString	s;
	    GroupObject	*G;

	    S = P->slide(i);
	    s = S->title();
//	    cout << "--------------------" << endl;

	    recursiveSearch( S->rootObject(), out );
	}

    delete P;

    return 0;
}

