// -*- Mode: C++; -*-
//                            Package   : omniORB
// zlibCompressor.h           Created on: 2012/10/05
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2012 Apasphere Ltd.
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
// Description:
//    zlib compressor

#ifndef _zlibCompressor_h_
#define _zlibCompressor_h_


#include <omniORB4/omniZIOP.h>

OMNI_NAMESPACE_BEGIN(omni)

class zlibCompressorFactory : public Compression::CompressorFactory
{
public:
  inline zlibCompressorFactory()
    : pd_refcount(1)
  { }

  ~zlibCompressorFactory();

  Compression::Compressor_ptr
  get_compressor(Compression::CompressionLevel compression_level);

  Compression::CompressorId
  compressor_id();

  void _add_ref();
  void _remove_ref();

private:
  int              pd_refcount;
  omni_tracedmutex pd_lock;
};

class zlibCompressor : public Compression::Compressor
{
public:
  inline zlibCompressor(Compression::CompressorFactory_ptr factory,
                        Compression::CompressionLevel      level)
    : pd_factory(Compression::CompressorFactory::_duplicate(factory)),
      pd_level(level),
      pd_compressed_bytes(0),
      pd_uncompressed_bytes(0),
      pd_refcount(1)
  {}

  ~zlibCompressor();

  void
  compress(const Compression::Buffer& source, Compression::Buffer& target);

  void
  decompress(const Compression::Buffer& source, Compression::Buffer& target);

  Compression::CompressorFactory_ptr
  compressor_factory();

  Compression::CompressionLevel
  compression_level();

  CORBA::ULongLong
  compressed_bytes();

  CORBA::ULongLong
  uncompressed_bytes();

  Compression::CompressionRatio
  compression_ratio();

  void _add_ref();
  void _remove_ref();

private:
  Compression::CompressorFactory_var pd_factory;
  Compression::CompressionLevel      pd_level;
  CORBA::ULongLong                   pd_compressed_bytes;
  CORBA::ULongLong                   pd_uncompressed_bytes;

  int              pd_refcount;
  omni_tracedmutex pd_lock;
};


OMNI_NAMESPACE_END(omni)

#endif // _zlibCompressor_h_
