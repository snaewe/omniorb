// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniutilities.h            Created on: 11/98
//                            Author    : David Riddoch
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//	

/*
  $Log$
  Revision 1.1.6.2  2003/09/26 16:12:53  dgrisby
  Start of valuetype support.

  Revision 1.1.6.1  2003/03/23 21:04:06  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.4.2  2002/11/21 14:24:59  dgrisby
  More unique guard name (thanks Daniel Bell).

  Revision 1.1.4.1  2001/04/18 17:26:25  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __OMNIUTILITIES_H__
#define __OMNIUTILITIES_H__


template <class InputIterator, class OutputIterator>
inline OutputIterator omnicopy(InputIterator first, InputIterator last,
			       OutputIterator result)
{
  while( first != last )  *result++ = *first++;

  return result;
}


//: A vector template.
//  This is similar to the STL vector type, but not as good!  In
// particular objects of type T are constructed when they are
// allocated - not when they are first inserted. However this makes
// little difference for primitive types (eg.  pointers). The
// interface mimics that of STL vector so that transition to STL will
// be simple.

template <class T>
class omnivector {
public:
  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef size_t size_type;

  inline omnivector() : start(0), finish(0), end_of_storage(0) {}
  inline omnivector(size_type n, const T& value = T())
    : start(0), finish(0), end_of_storage(0) {
    reserve(n);
    while( n-- )  push_back(value);
  }
  inline ~omnivector() { deallocate(); }

  inline void reserve(size_type n) {
    if( capacity() < n ) {
      size_type old_size = size();
      iterator tmp = allocate(n);
      omnicopy(start, finish, tmp);
      deallocate();
      start = tmp;
      finish = start + old_size;
      end_of_storage = start + n;
    }
  }

  inline iterator begin() { return start;  }
  inline const_iterator begin() const { return start;  }
  inline iterator end() { return finish; }
  inline const_iterator end() const   { return finish; }
  inline size_type size() const { return size_type(finish - start); }
  inline size_type capacity() const {
    return size_type(end_of_storage - start);
  }
  inline int empty() const { return start == finish; }
  inline reference operator[](size_type n) { return *(start + n); }
  inline const_reference operator[](size_type n) const { return *(start + n); }
  inline reference front() { return *start; }
  inline reference back() { return *(finish - 1); }
  inline void push_back(const T& x) {
    if( finish == end_of_storage )  more();
    *finish++ = x;
  }
  inline void pop_back() { finish--; }
  inline iterator erase(iterator position) {
    if( position + 1 != finish )
      omnicopy(position + 1, finish, position);
    finish--;
    return position;
  }
  inline iterator erase(iterator first, iterator last) {
    iterator i = omnicopy(last, finish, first);
    finish -= (last - first);
    return first;
  }
#if 0
  void resize(size_type new_size, const T& x) {
    if (new_size < size())
      erase(begin() + new_size, end());
    else
      insert(end(), new_size - size(), x);
  }
  void resize(size_type new_size) { resize(new_size, T()); }
#endif

private:
  inline void more() { reserve(size() ? size() * 2 : 1); }
  inline void deallocate() { if( start )  delete[] start; }
  inline iterator allocate(size_type n) {
    return new value_type[n];
  }

  omnivector<T>& operator=(const omnivector<T>& x);
  //?? Not yet implemented

  iterator start;
  iterator finish;
  iterator end_of_storage;
};


#endif  // __OMNIUTILITIES_H__
