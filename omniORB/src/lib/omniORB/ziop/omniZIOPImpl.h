// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniZIOPImpl.h             Created on: 2012/10/02
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
//    ZIOP support

#ifndef _omniZIOPImpl_h_
#define _omniZIOPImpl_h_

#include <omniORB4/omniZIOP.h>
#include <giopStream.h>


OMNI_NAMESPACE_BEGIN(omni)

//
// Standard CompressionManager

class omniCompressionManager
  : public virtual Compression::CompressionManager
{
public:
  inline omniCompressionManager()
    : pd_refcount(1)
  {}

  ~omniCompressionManager();

  void
  register_factory(Compression::CompressorFactory_ptr compressor_factory);

  void
  unregister_factory(Compression::CompressorId compressor_id);

  Compression::CompressorFactory_ptr
  get_factory(Compression::CompressorId compressor_id);

  Compression::Compressor_ptr
  get_compressor(Compression::CompressorId     compressor_id,
                 Compression::CompressionLevel compression_level);

  Compression::CompressorFactorySeq*
  get_factories();

  void _add_ref();
  void _remove_ref();

private:
  Compression::CompressorFactorySeq pd_factories;

  int              pd_refcount;
  omni_tracedmutex pd_lock;
};


//
// Internal giopCompressor implementation

class giopCompressorImpl : public giopCompressor {
public:
  giopCompressorImpl();

  giopCompressorImpl(const Compression::CompressorIdLevelList& compressor_ids);


  giopCompressorImpl(const Compression::CompressorIdLevelList& compressor_ids,
                     CORBA::ULong low_value, CORBA::Float min_ratio);


  void setPolicies(const Compression::CompressorIdLevelList& compressor_ids,
                   CORBA::ULong low_value, CORBA::Float min_ratio);

  virtual ~giopCompressorImpl();

  virtual giopStream_Buffer* compressBuffer(giopStream*        stream,
                                            giopStream_Buffer* buf);

  virtual giopStream_Buffer* decompressBuffer(giopStream*        stream,
                                              giopStream_Buffer* buf);

private:
  CORBA::ULong compressorIndex(giopStream*               stream,
                               Compression::CompressorId compressor_id);

  Compression::CompressorSeq         pd_compressors;
  Compression::CompressorIdLevelList pd_compressor_ids;
  CORBA::ULong                       pd_min_size;
  CORBA::Float                       pd_ratio;
};


//
// Internal giopCompressorFactory implementation

class giopCompressorFactoryImpl : public giopCompressorFactory {
public:
  virtual ~giopCompressorFactoryImpl();
  virtual giopCompressor* newCompressor();
};


OMNI_NAMESPACE_END(omni)


#endif // _omniZIOPImpl_h_
