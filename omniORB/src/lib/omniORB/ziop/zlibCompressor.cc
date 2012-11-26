// -*- Mode: C++; -*-
//                            Package   : omniORB
// zlibCompressor.cc          Created on: 2012/10/05
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

#include "zlibCompressor.h"
#include <zlib.h>


OMNI_NAMESPACE_BEGIN(omni)


//
// Factory

zlibCompressorFactory::~zlibCompressorFactory() {}

Compression::Compressor_ptr
zlibCompressorFactory::
get_compressor(Compression::CompressionLevel compression_level)
{
  if (compression_level > 9)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidCompressionLevel,
                  CORBA::COMPLETED_NO);

  return new zlibCompressor(this, compression_level);
}

Compression::CompressorId
zlibCompressorFactory::
compressor_id()
{
  return Compression::COMPRESSORID_ZLIB;
}

void
zlibCompressorFactory::
_add_ref()
{
  omni_tracedmutex_lock l(pd_lock);
  ++pd_refcount;
}

void
zlibCompressorFactory::
_remove_ref()
{
  {
    omni_tracedmutex_lock l(pd_lock);
    if (--pd_refcount != 0)
      return;
  }
  delete this;
}


//
// Compressor

static const char* errReason(int ret)
{
  switch (ret) {
  case Z_MEM_ERROR:
    return "Not enough memory";
  case Z_BUF_ERROR:
    return "Not enough room in output buffer";
  case Z_STREAM_ERROR:
    return "Invalid compression level";
  case Z_DATA_ERROR:
    return "Data error";
  default:
    return "Unknown ZLIB error";
  }
}

zlibCompressor::~zlibCompressor() {}

void
zlibCompressor::
compress(const Compression::Buffer& source, Compression::Buffer& target)
{
  // omniORB pre-populates target with a suitable buffer.
  uLongf target_len = target.length();

  if (target_len == 0) {
    // In case application code calls this without pre-populating target.
    target_len = compressBound(source.length());
    target.length(target_len);
  }

  const Bytef* src  = (const Bytef*)source.NP_data();
  Bytef*       dest = (Bytef*)      target.NP_data();
  
  int ret = compress2(dest, &target_len, src, source.length(), pd_level);
  if (ret == Z_OK) {
    target.length(target_len);
    {
      omni_tracedmutex_lock l(pd_lock);
      pd_compressed_bytes   += target_len;
      pd_uncompressed_bytes += source.length();
    }
  }
  else {
    throw Compression::CompressionException(ret, errReason(ret));
  }
}

void
zlibCompressor::
decompress(const Compression::Buffer& source, Compression::Buffer& target)
{
  // target length is initialised with correct uncompressed length.

  const Bytef* src  = (const Bytef*)source.NP_data();
  Bytef*       dest = (Bytef*)      target.NP_data();
  uLongf       dlen = target.length();

  int ret = uncompress(dest, &dlen, src, source.length());
  if (ret == Z_OK) {
    if (dlen != target.length())
      target.length(dlen);

    return;
  }
  else {
    throw Compression::CompressionException(ret, errReason(ret));
  }
}


Compression::CompressorFactory_ptr
zlibCompressor::
compressor_factory()
{
  return Compression::CompressorFactory::_duplicate(pd_factory);
}

Compression::CompressionLevel
zlibCompressor::
compression_level()
{
  return pd_level;
}

CORBA::ULongLong
zlibCompressor::
compressed_bytes()
{
  omni_tracedmutex_lock l(pd_lock);
  return pd_compressed_bytes;
}

CORBA::ULongLong
zlibCompressor::
uncompressed_bytes()
{
  omni_tracedmutex_lock l(pd_lock);
  return pd_uncompressed_bytes;
}

Compression::CompressionRatio
zlibCompressor::
compression_ratio()
{
  omni_tracedmutex_lock l(pd_lock);
  return (((Compression::CompressionRatio)pd_compressed_bytes) /
          ((Compression::CompressionRatio)pd_uncompressed_bytes));
}

void
zlibCompressor::
_add_ref()
{
  omni_tracedmutex_lock l(pd_lock);
  ++pd_refcount;
}

void
zlibCompressor::
_remove_ref()
{
  {
    omni_tracedmutex_lock l(pd_lock);
    if (--pd_refcount != 0)
      return;
  }
  delete this;
}


OMNI_NAMESPACE_END(omni)
