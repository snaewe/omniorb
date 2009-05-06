// -*- Mode: C++; -*-
//                            Package   : omniORB2
// codeSets.h                 Created on: 18/10/2000
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2002-2008 Apasphere Ltd
//    Copyright (C) 2000      AT&T Laboratories, Cambridge
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
//	*** PROPRIETARY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.4.4  2009/05/06 16:16:13  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.3  2008/08/08 18:45:49  dgrisby
  Add missing ISO-8859 and Windows code sets.

  Revision 1.1.4.2  2005/12/08 14:22:31  dgrisby
  Better string marshalling performance; other minor optimisations.

  Revision 1.1.4.1  2003/03/23 21:04:17  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.9  2003/03/03 12:32:32  dgrisby
  EBCDIC code sets. Thanks Coleman Corrigan.

  Revision 1.1.2.8  2003/01/14 11:48:15  dgrisby
  Remove warnings from gcc -Wshadow. Thanks Pablo Mejia.

  Revision 1.1.2.7  2002/12/19 13:56:58  dgrisby
  New Windows 1251 code set. (Thanks Vasily Tchekalkin).

  Revision 1.1.2.6  2001/06/13 20:06:18  sll
  Minor fix to make the ORB compile with MSVC++.

  Revision 1.1.2.5  2000/11/22 14:37:58  dpg1
  Code set marshalling functions now take a string length argument.

  Revision 1.1.2.4  2000/11/16 12:33:43  dpg1
  Minor fixes to permit use of UShort as WChar.

  Revision 1.1.2.3  2000/11/15 17:15:42  sll
  Added definition for the TAG_CODE_SETS component.

  Revision 1.1.2.2  2000/11/09 12:23:04  dpg1
  Update to compile on Solaris

  Revision 1.1.2.1  2000/10/27 15:42:03  dpg1
  Initial code set conversion support. Not yet enabled or fully tested.

*/

#ifndef __CODESETS_H__
#define __CODESETS_H__


//#include <CONV_FRAME.hh>

class cdrStream;

OMNI_NAMESPACE_BEGIN(omni)

class CONV_FRAME {
public:
  typedef _CORBA_ULong CodeSetId;
  typedef _CORBA_Unbounded_Sequence_w_FixSizeElement<CodeSetId,4,4> CodeSetIdList;

  struct CodeSetComponent {
    CodeSetId     native_code_set;
    CodeSetIdList conversion_code_sets;

    void operator>>= (cdrStream& s) const;
    void operator<<= (cdrStream& s);
  };

  struct CodeSetComponentInfo {
    CodeSetComponent  ForCharData;
    CodeSetComponent  ForWcharData;

    void operator>>= (cdrStream& s) const;
    void operator<<= (cdrStream& s);
  };
};


// Code set conversion framework
// =============================
//
// For any transmission involving char or wchar data, there are two
// code sets involved -- the `native' code set, which is the code set
// the application is using, and the `transmission' code set, which is
// the code set negotiated with the other end, used to send the data.
// At any given time, there are four code sets in force, since char
// and wchar code sets are separate. These are termed NCS-C, TCS-C,
// NCS-W, and TCS-W, for native char, transmission char, native wchar,
// transmission wchar respectively.
//
// These four code sets are represented by four objects, derived from
// the NCS_C, TCS_C, NCS_W, and TCS_W classes. To marshal or unmarshal
// char data (i.e. char and string), for example, the ORB calls
// methods of the NCS_C object, passing a pointer to the TCS_C object
// to be used. The ORB will not normally call methods of the TCS_C
// object directly.
//
// For each supported code set, there is a single object registered
// with the system. These objects are usually statically allocated.
// Since all interactions involving a particular code set use the same
// code set object, all objects must be stateless.
//
// By default, the NCS object comunicates with the TCS object via
// UTF-16 Unicode, so all NCS objects can translate to and from all
// TCS objects. For efficiency, either object is permitted to bypass
// the UTF-16 step, and use some more efficient translation. NCS
// implementations must always call the TCS's fast...() function
// before resorting to UTF-16, in case the TCS can deal with the NCS
// data directly.
//
// In all functions involving string lengths, the length does not
// include the terminating null.

class omniCodeSet {
public:

  // Common code set id constants
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_1;  // ISO 8859
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_2;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_3;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_4;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_5;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_6;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_7;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_8;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_9;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_10;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_11;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_13;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_14;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_15;
  static _core_attr const CONV_FRAME::CodeSetId ID_8859_16;
  static _core_attr const CONV_FRAME::CodeSetId ID_UCS_2;   // Unicode
  static _core_attr const CONV_FRAME::CodeSetId ID_UCS_4;
  static _core_attr const CONV_FRAME::CodeSetId ID_UTF_8;
  static _core_attr const CONV_FRAME::CodeSetId ID_UTF_16;  // Unicode with
                                                            //  surrogates
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1250;  // Windows-1250
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1251;  // Windows-1251
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1252;  // Windows-1252
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1253;  // Windows-1253
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1254;  // Windows-1254
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1255;  // Windows-1255
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1256;  // Windows-1256
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1257;  // Windows-1257
  static _core_attr const CONV_FRAME::CodeSetId ID_CP1258;  // Windows-1258
  static _core_attr const CONV_FRAME::CodeSetId ID_037;     // IBM Ebcdic
  static _core_attr const CONV_FRAME::CodeSetId ID_500;
  static _core_attr const CONV_FRAME::CodeSetId ID_EDF_4;


  class NCS_C; // Native code set for char and string
  class NCS_W; // Native code set for wide char and wide string
  class TCS_C; // Transmission code set for char and string
  class TCS_W; // Transmission code set for wide char and wide string

  static NCS_C* getNCS_C(CONV_FRAME::CodeSetId id);
  static NCS_W* getNCS_W(CONV_FRAME::CodeSetId id);
  static TCS_C* getTCS_C(CONV_FRAME::CodeSetId id, GIOP::Version v);
  static TCS_W* getTCS_W(CONV_FRAME::CodeSetId id, GIOP::Version v);
  // Static functions to get the code set object for a particular DCE
  // code set id. Return null if no suitable code set object exists.

  static NCS_C* getNCS_C(const char* name);
  static NCS_W* getNCS_W(const char* name);
  static TCS_C* getTCS_C(const char* name, GIOP::Version v);
  static TCS_W* getTCS_W(const char* name, GIOP::Version v);
  // Static functions to get the code set object for a particular
  // name. Return null if no suitable code set object exists.

  static void registerNCS_C(NCS_C* cs);
  static void registerNCS_W(NCS_W* cs);
  static void registerTCS_C(TCS_C* cs);
  static void registerTCS_W(TCS_W* cs);
  // Static functions to register new code set objects.
  // NO concurrency control. It is expected that these functions will
  // be called from static initialisers.

  static void logCodeSets();
  // Print out the supported code sets

#if (SIZEOF_WCHAR == 2)
  typedef _CORBA_WChar  UniChar; // Unicode code point (UTF-16)
#else
  typedef _CORBA_UShort UniChar;
#endif

  // Special code set kinds
  enum Kind {
    CS_Other, // Don't know anything special about this code set
    CS_8bit,  // Simple 8 bit set with one-to-one Unicode mapping
              //  Class must be NCS_C_8bit or TCS_C_8bit
    CS_16bit  // Simple 16 bit set with one-to-one Unicode mapping
              //  (not Unicode itself, though)
              //  Class must be NCS_W_16bit or TCS_W_16bit
  };


  // Base class for all code sets
  class Base {
  public:

    inline CONV_FRAME::CodeSetId id()     const { return pd_id; }
    inline const char*           name()   const { return pd_name; }
    inline Kind                  kind()   const { return pd_kind; }
    // DCE code set identifier, descriptive name string, code set kind

  protected:
    Base(CONV_FRAME::CodeSetId id_, const char* name_, Kind kind_)
      : pd_id(id_), pd_name(name_), pd_kind(kind_), pd_next(0) {}

    virtual ~Base() {}

  private:
    CONV_FRAME::CodeSetId pd_id;
    const char*           pd_name;
    Kind                  pd_kind;
    Base*                 pd_next; // Linked list
    friend class omniCodeSet;
  };


  class NCS_C : public Base {
  public:

    virtual void marshalChar(cdrStream& stream, TCS_C* tcs, _CORBA_Char c) = 0;

    virtual void marshalString(cdrStream& stream, TCS_C* tcs,
			       _CORBA_ULong bound, _CORBA_ULong len,
			       const char* s) = 0;
    // Marshal string. len is the length of the string or zero to
    // indicate that the length is not known (or actually zero).

    virtual _CORBA_Char unmarshalChar(cdrStream& stream, TCS_C* tcs) = 0;

    virtual _CORBA_ULong unmarshalString(cdrStream& stream, TCS_C* tcs,
					 _CORBA_ULong bound, char*& s) = 0;

    NCS_C(CONV_FRAME::CodeSetId id, const char* name, Kind kind);
    virtual ~NCS_C() {}
  };

  class TCS_C : public Base {
  public:

    inline GIOP::Version giopVersion() const { return pd_giopVersion; }

    // Unicode based marshalling
    virtual void marshalChar  (cdrStream& stream, UniChar uc) = 0;
    virtual void marshalString(cdrStream& stream, _CORBA_ULong bound,
			       _CORBA_ULong len, const UniChar* us) = 0;
    // len is the length of the unicode string in us. It may only be
    // zero if the string is of zero length.

    virtual UniChar unmarshalChar(cdrStream& stream) = 0;

    virtual _CORBA_ULong unmarshalString(cdrStream& stream,
					 _CORBA_ULong bound, UniChar*& us) = 0;

    // Fast marshalling functions. Return false if no fast case is
    // possible and the Unicode functions should be used.
    virtual _CORBA_Boolean fastMarshalChar    (cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_Char   c);

    virtual _CORBA_Boolean fastMarshalString  (cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_ULong  bound,
					       _CORBA_ULong  len,
					       const char*   s);
    // len may be zero if length is not known.

    virtual _CORBA_Boolean fastUnmarshalChar  (cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_Char&  c);

    virtual _CORBA_Boolean fastUnmarshalString(cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_ULong  bound,
					       _CORBA_ULong& length,
					       char*&        s);

    TCS_C(CONV_FRAME::CodeSetId id, const char* name, Kind kind,
	  GIOP::Version giopVersion);
    virtual ~TCS_C() {}

  private:
    GIOP::Version pd_giopVersion;
  };

  class NCS_W : public Base {
  public:

    virtual void marshalWChar(cdrStream& stream, TCS_W* tcs,
			      _CORBA_WChar c) = 0;

    virtual void marshalWString(cdrStream& stream, TCS_W* tcs,
				_CORBA_ULong bound, _CORBA_ULong len,
				const _CORBA_WChar* s) = 0;

    virtual _CORBA_WChar unmarshalWChar(cdrStream& stream, TCS_W* tcs) = 0;

    virtual _CORBA_ULong unmarshalWString(cdrStream& stream, TCS_W* tcs,
					  _CORBA_ULong bound,
					  _CORBA_WChar*& s) = 0;

    NCS_W(CONV_FRAME::CodeSetId id, const char* name, Kind kind);
    virtual ~NCS_W() {}
  };

  class TCS_W : public Base {
  public:

    inline GIOP::Version giopVersion() const { return pd_giopVersion; }

    // Unicode based marshalling
    virtual void marshalWChar  (cdrStream& stream, UniChar uc) = 0;
    virtual void marshalWString(cdrStream& stream, _CORBA_ULong bound,
				_CORBA_ULong len, const UniChar* us) = 0;

    virtual UniChar unmarshalWChar(cdrStream& stream) = 0;

    virtual _CORBA_ULong unmarshalWString(cdrStream& stream,
					  _CORBA_ULong bound,
					  UniChar*& us) = 0;

    // Fast marshalling functions. Return false if no fast case is
    // possible and Unicode functions should be used.
    virtual _CORBA_Boolean fastMarshalWChar    (cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_WChar        c);

    virtual _CORBA_Boolean fastMarshalWString  (cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_ULong        bound,
						_CORBA_ULong        len,
						const _CORBA_WChar* s);

    virtual _CORBA_Boolean fastUnmarshalWChar  (cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_WChar&       c);

    virtual _CORBA_Boolean fastUnmarshalWString(cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_ULong        bound,
						_CORBA_ULong&       length,
						_CORBA_WChar*&      s);

    TCS_W(CONV_FRAME::CodeSetId id, const char* name, Kind kind,
	  GIOP::Version giopVersion);
    virtual ~TCS_W() {}

  private:
    GIOP::Version pd_giopVersion;
  };



  //
  // Classes for code sets we know something special about
  //

  //
  // Single byte code sets.
  //

  // Single byte codes sets contain two mapping tables, to and from
  // UCS-2. The table to UCS-2 is a simple 256 element arrray of UCS-2
  // values. The table from UCS-2 is a two-level table. The high octet
  // of the UCS-2 value is looked-up in the top-level table; the low
  // octet is looked-up in the table found in the first look-up. The
  // majority of second-level table entries are usually the same empty
  // table.
  //
  // In both tables, an entry with value zero in any place except
  // index zero means there is no mapping for the character in
  // question, and a DATA_CONVERSION exception should be thrown.

  class NCS_C_8bit : public NCS_C {
  public:

    // Inherited virtual functions
    virtual void marshalChar(cdrStream& stream, TCS_C* tcs,
			     _CORBA_Char c);

    virtual void marshalString(cdrStream& stream, TCS_C* tcs,
			       _CORBA_ULong bound, _CORBA_ULong len,
			       const char* s);

    virtual _CORBA_Char unmarshalChar(cdrStream& stream, TCS_C* tcs);

    virtual _CORBA_ULong unmarshalString(cdrStream& stream,
					 TCS_C* tcs,
					 _CORBA_ULong bound, char*& s);

    inline const UniChar*      toU()   const { return pd_toU; }
    inline const _CORBA_Char** fromU() const { return pd_fromU; }

    NCS_C_8bit(CONV_FRAME::CodeSetId id_,
	       const char*           name_,
	       const UniChar*        toU_,
	       const _CORBA_Char**   fromU_)

      : NCS_C(id_, name_, CS_8bit),
	pd_toU(toU_),
	pd_fromU(fromU_)
    { }

    virtual ~NCS_C_8bit() {};

  private:
    const UniChar*      pd_toU;
    const _CORBA_Char** pd_fromU;
  };

  class TCS_C_8bit : public TCS_C {
  public:

    // Inherited virtual functions
    virtual void marshalChar  (cdrStream& stream, UniChar uc);
    virtual void marshalString(cdrStream& stream, _CORBA_ULong bound,
			       _CORBA_ULong len, const UniChar* us);

    virtual UniChar unmarshalChar(cdrStream& stream);

    virtual _CORBA_ULong unmarshalString(cdrStream& stream,
					 _CORBA_ULong bound,
					 UniChar*& us);

    virtual _CORBA_Boolean fastMarshalChar    (cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_Char   c);

    virtual _CORBA_Boolean fastMarshalString  (cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_ULong  bound,
					       _CORBA_ULong  len,
					       const char*   s);

    virtual _CORBA_Boolean fastUnmarshalChar  (cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_Char&  c);

    virtual _CORBA_Boolean fastUnmarshalString(cdrStream&    stream,
					       NCS_C*        ncs,
					       _CORBA_ULong  bound,
					       _CORBA_ULong& length,
					       char*&        s);
    // Translation tables:
    inline const UniChar*      toU()   const { return pd_toU; }
    inline const _CORBA_Char** fromU() const { return pd_fromU; }

    TCS_C_8bit(CONV_FRAME::CodeSetId id_,
	       const char*           name_,
	       GIOP::Version         giopVersion_,
	       const UniChar*        toU_,
	       const _CORBA_Char**   fromU_)

      : TCS_C(id_, name_, CS_8bit, giopVersion_),
	pd_toU(toU_),
	pd_fromU(fromU_)
    { }

    virtual ~TCS_C_8bit() {}

  private:
    const UniChar*      pd_toU;
    const _CORBA_Char** pd_fromU;
  };

  static _core_attr const _CORBA_Char empty8BitTable[];
  // Empty 8 bit table to be used as a second-level table in
  // Unicode -> 8 bit mappings


  //
  // Two byte code sets.
  //
  // If sizeof(whar_t) == 4, the native code set makes sure each wide
  // character uses only the low 16 bits.
  //

  // Two byte code sets contain two two-level tables, one mapping to
  // UCS-2, the other mapping from it.

  class NCS_W_16bit : public NCS_W {
  public:

    virtual void marshalWChar(cdrStream& stream, TCS_W* tcs,
			      _CORBA_WChar c);

    virtual void marshalWString(cdrStream& stream, TCS_W* tcs,
				_CORBA_ULong bound, _CORBA_ULong len,
				const _CORBA_WChar* s);

    virtual _CORBA_WChar unmarshalWChar(cdrStream& stream, TCS_W* tcs);

    virtual _CORBA_ULong unmarshalWString(cdrStream& stream, TCS_W* tcs,
					  _CORBA_ULong bound,
					  _CORBA_WChar*& s);

    inline const UniChar**       toU()   const { return pd_toU; }
    inline const _CORBA_UShort** fromU() const { return pd_fromU; }

    NCS_W_16bit(CONV_FRAME::CodeSetId id_,
		const char*           name_,
		const UniChar**       toU_,
		const _CORBA_UShort** fromU_)

      : NCS_W(id_, name_, CS_16bit),
	pd_toU(toU_),
	pd_fromU(fromU_)
    { }
    virtual ~NCS_W_16bit() {}

  private:
    const UniChar**       pd_toU;
    const _CORBA_UShort** pd_fromU;
  };

  class TCS_W_16bit : public TCS_W {
  public:

    // Unicode based marshalling
    virtual void marshalWChar  (cdrStream& stream, UniChar uc);
    virtual void marshalWString(cdrStream& stream, _CORBA_ULong bound,
				_CORBA_ULong len, const UniChar* us);

    virtual UniChar unmarshalWChar(cdrStream& stream);

    virtual _CORBA_ULong unmarshalWString(cdrStream& stream,
					  _CORBA_ULong bound,
					  UniChar*& us);

    // Fast marshalling functions. Return false if no fast case is
    // possible and Unicode functions should be used.
    virtual _CORBA_Boolean fastMarshalWChar    (cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_WChar        c);

    virtual _CORBA_Boolean fastMarshalWString  (cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_ULong        bound,
						_CORBA_ULong        len,
						const _CORBA_WChar* s);

    virtual _CORBA_Boolean fastUnmarshalWChar  (cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_WChar&       c);

    virtual _CORBA_Boolean fastUnmarshalWString(cdrStream&          stream,
						NCS_W*              ncs,
						_CORBA_ULong        bound,
						_CORBA_ULong&       length,
						_CORBA_WChar*&      s);

    inline const UniChar**       toU()   const { return pd_toU; }
    inline const _CORBA_UShort** fromU() const { return pd_fromU; }

    TCS_W_16bit(CONV_FRAME::CodeSetId id_,
		const char*           name_,
		GIOP::Version         giopVersion_,
		const UniChar**       toU_,
		const _CORBA_UShort** fromU_)

      : TCS_W(id_, name_, CS_16bit, giopVersion_),
	pd_toU(toU_),
	pd_fromU(fromU_)
    { }
    virtual ~TCS_W_16bit() {}

  private:
    const UniChar**       pd_toU;
    const _CORBA_UShort** pd_fromU;
  };

  static _core_attr const _CORBA_UShort empty16BitTable[];
  // Empty 16 bit table to be used as a second-level table in 16 bit
  // mappings

};

OMNI_NAMESPACE_END(omni)

#endif // __CODESETS_H__
