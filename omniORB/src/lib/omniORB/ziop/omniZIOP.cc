// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniZIOP.cc                Created on: 2012/10/02
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

#include "omniZIOPImpl.h"
#include <omniORB4/omniInterceptors.h>
#include <omniORB4/messaging.hh>
#include <initialiser.h>
#include <initRefs.h>
#include <giopStrandFlags.h>
#include <giopStrand.h>
#include <giopRope.h>
#include <GIOP_C.h>
#include <GIOP_S.h>
#include <transportRules.h>
#include <poaimpl.h>
#include "zlibCompressor.h"

OMNI_USING_NAMESPACE(omni)

static const CORBA::ULong GIOP_HEADER      = 12;
static const CORBA::ULong ZIOP_HEADER      = 12;
static const CORBA::ULong GIOP_ZIOP_HEADER = GIOP_HEADER + ZIOP_HEADER;

//
// Global policies
static CORBA::Boolean                     g_compression_enabled   = 0;
static Compression::CompressorIdLevelList g_compressor_ids;
static CORBA::ULong                       g_compression_low_value = 1024;
static CORBA::Float                       g_compression_min_ratio = 0.75;


//
// Policies

ZIOP::CompressionEnablingPolicy_ptr
omniZIOP::
create_compression_enabling_policy(CORBA::Boolean compression_enabled)
{
  return new ZIOP::CompressionEnablingPolicy(compression_enabled);
}

ZIOP::CompressionIdLevelListPolicy_ptr
omniZIOP::
create_compression_id_level_list_policy(const Compression::CompressorIdLevelList& compressor_ids)
{
  return new ZIOP::CompressionIdLevelListPolicy(compressor_ids);
}

ZIOP::CompressionLowValuePolicy_ptr
omniZIOP::
create_compression_low_value_policy(CORBA::ULong low_value)
{
  return new ZIOP::CompressionLowValuePolicy(low_value);
}

ZIOP::CompressionMinRatioPolicy_ptr
omniZIOP::
create_compression_min_ratio_policy(Compression::CompressionRatio ratio)
{
  return new ZIOP::CompressionMinRatioPolicy(ratio);
}


static void
decodePolicies(const CORBA::PolicyList&            policies,
               CORBA::Boolean&                     enabled,
               Compression::CompressorIdLevelList& compressor_ids,
               CORBA::ULong&                       low_value,
               CORBA::Float&                       min_ratio)
{
  CORBA::ULong index;

  for (index=0; index != policies.length(); ++index) {
    CORBA::Policy* policy = policies[index];

    switch (policy->policy_type()) {
    case ZIOP::COMPRESSION_ENABLING_POLICY_ID:
      {
        ZIOP::CompressionEnablingPolicy_var ep =
          ZIOP::CompressionEnablingPolicy::_narrow(policy);
        
        if (CORBA::is_nil(ep)) {
          omniORB::logs(1, "Invalid Policy object with id "
                        "ZIOP::COMPRESSION_ENABLING_POLICY_ID");
          OMNIORB_THROW(INV_OBJREF, INV_OBJREF_InterfaceMisMatch,
                        CORBA::COMPLETED_NO);
        }

        enabled = ep->compression_enabled();
        break;
      }

    case ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID:
      {
        ZIOP::CompressionIdLevelListPolicy_var ep =
          ZIOP::CompressionIdLevelListPolicy::_narrow(policy);

        if (CORBA::is_nil(ep)) {
          omniORB::logs(1, "Invalid Policy object with id "
                        "ZIOP::COMPRESSION_ENABLING_POLICY_ID");
          OMNIORB_THROW(INV_OBJREF, INV_OBJREF_InterfaceMisMatch,
                        CORBA::COMPLETED_NO);
        }

        Compression::CompressorIdLevelList_var cids = ep->compressor_ids();
        compressor_ids = cids;
        break;
      }

    case ZIOP::COMPRESSION_LOW_VALUE_POLICY_ID:
      {
        ZIOP::CompressionLowValuePolicy_var ep =
          ZIOP::CompressionLowValuePolicy::_narrow(policy);

        if (CORBA::is_nil(ep)) {
          omniORB::logs(1, "Invalid Policy object with id "
                        "ZIOP::COMPRESSION_LOW_VALUE_POLICY_ID");
          OMNIORB_THROW(INV_OBJREF, INV_OBJREF_InterfaceMisMatch,
                        CORBA::COMPLETED_NO);
        }
        low_value = ep->low_value();
        break;
      }

    case ZIOP::COMPRESSION_MIN_RATIO_POLICY_ID:
      {
        ZIOP::CompressionMinRatioPolicy_var ep =
          ZIOP::CompressionMinRatioPolicy::_narrow(policy);

        if (CORBA::is_nil(ep)) {
          omniORB::logs(1, "Invalid Policy object with id "
                        "ZIOP::COMPRESSION_MIN_RATIO_POLICY_ID");
          OMNIORB_THROW(INV_OBJREF, INV_OBJREF_InterfaceMisMatch,
                        CORBA::COMPLETED_NO);
        }
        min_ratio = ep->ratio();
        break;
      }
    }
  }
}


static void
setPoliciesInProfile(const CORBA::PolicyList*       policies,
                     IOP::MultipleComponentProfile& components)
{
  CORBA::Boolean enabled   = g_compression_enabled;
  CORBA::ULong   low_value = g_compression_low_value;
  CORBA::Float   min_ratio = g_compression_min_ratio;

  Compression::CompressorIdLevelList compressor_ids(g_compressor_ids);

  if (policies)
    decodePolicies(*policies, enabled, compressor_ids, low_value, min_ratio);

  if (!enabled)
    return;

  Messaging::PolicyValueSeq pvseq(2);
  pvseq.length(2);
  
  // Enabling policy
  {
    cdrEncapsulationStream estream;
    estream.marshalBoolean(1);

    pvseq[0].ptype = ZIOP::COMPRESSION_ENABLING_POLICY_ID;
    estream.setOctetSeq(pvseq[0].pvalue);
  }

  // ID/level list
  {
    cdrEncapsulationStream estream;
    compressor_ids >>= estream;

    pvseq[1].ptype = ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID;
    estream.setOctetSeq(pvseq[1].pvalue);
  }

  // Tagged component
  {
    IOP::TaggedComponent& c = omniIOR::newIIOPtaggedComponent(components);
    c.tag = IOP::TAG_POLICIES;

    cdrEncapsulationStream estream;
    pvseq >>= estream;

    estream.setOctetSeq(c.component_data);
  }
}



CORBA::Object_ptr
omniZIOP::
setServerPolicies(CORBA::Object_ptr obj, const CORBA::PolicyList& policies)
{
  if (CORBA::is_nil(obj))
    return obj;

  if (obj->_NP_is_pseudo())
    OMNIORB_THROW(INV_OBJREF,
                  INV_OBJREF_InvalidPseudoObject,
                  CORBA::COMPLETED_NO);

  // Get the original object reference's IOR
  omniIOR* orig_ior = obj->_PR_getobj()->_getIOR();

  if (omniORB::trace(5)) {
    omniORB::logger log;
    log << "Set server ZIOP policies for object reference type '"
        << orig_ior->repositoryID() << "'.\n";
  }

  // Copy its profiles
  IOP::TaggedProfileList_var profiles(
    new IOP::TaggedProfileList(orig_ior->iopProfiles()));

  CORBA::Long selected = orig_ior->addr_selected_profile_index();

  CORBA::Boolean done = 0;

  if (selected >= 0 && profiles[selected].tag == IOP::TAG_INTERNET_IOP) {
    IIOP::ProfileBody iiop;
    IIOP::unmarshalProfile(profiles[selected], iiop);

    if (iiop.version.major > 1 || iiop.version.minor >= 2) {
      setPoliciesInProfile(&policies, iiop.components);
      IIOP::encodeProfile(iiop, profiles[selected]);
      done = 1;
    }
  }
  if (!done)
    OMNIORB_THROW(INV_OBJREF,
                  INV_OBJREF_UnsuitableGIOPVersion,
                  CORBA::COMPLETED_NO);

  // Finally, we build a new omniIOR with the modified profiles...
  omniIOR* ior = new omniIOR(CORBA::string_dup(orig_ior->repositoryID()),
			     profiles._retn());
  
  // ...and build a new object reference for it.
  omniObjRef* new_ref = omni::createObjRef(CORBA::Object::_PD_repoId, ior, 0);
  return (CORBA::Object_ptr)new_ref->_ptrToObjRef(CORBA::Object::_PD_repoId);
}



void
omniZIOP::
setGlobalPolicies(const CORBA::PolicyList& policies)
{
  // Set defaults
  g_compression_enabled = 1;

  g_compressor_ids.length(1);
  g_compressor_ids[0].compressor_id     = Compression::COMPRESSORID_ZLIB;
  g_compressor_ids[0].compression_level = 6;

  decodePolicies(policies,
                 g_compression_enabled,
                 g_compressor_ids,
                 g_compression_low_value,
                 g_compression_min_ratio);
}


#undef min

static inline
CORBA::Boolean
min(Compression::CompressionLevel a, Compression::CompressionLevel b)
{
  return a < b ? a : b;
}


static
void
filterCompressors(const Compression::CompressorIdLevelList& theirs,
                  const Compression::CompressorIdLevelList& mine,
                  Compression::CompressorIdLevelList&       combined)
{
  CORBA::ULong tidx, midx, cidx;

  combined.length(theirs.length());
  cidx = 0;

  for (tidx=0; tidx != theirs.length(); ++tidx) {
    Compression::CompressorId cid = theirs[tidx].compressor_id;

    for (midx=0; midx != mine.length(); ++midx) {

      if (mine[midx].compressor_id == cid) {
        combined[cidx].compressor_id     = cid;
        combined[cidx].compression_level = min(theirs[tidx].compression_level,
                                               mine[midx].compression_level);
        ++cidx;
      }
    }
  }
  combined.length(cidx);
}


OMNI_NAMESPACE_BEGIN(omni)


//
// Standard CompressionManager

omniCompressionManager::~omniCompressionManager()
{
}

void
omniCompressionManager::
register_factory(Compression::CompressorFactory_ptr compressor_factory)
{
  omni_tracedmutex_lock l(pd_lock);

  Compression::CompressorId new_id = compressor_factory->compressor_id();

  CORBA::ULong idx;
  for (idx=0; idx != pd_factories.length(); ++idx) {
    if (pd_factories[idx]->compressor_id() == new_id) {
      throw Compression::FactoryAlreadyRegistered();
    }
  }

  pd_factories.length(idx + 1);
  pd_factories[idx] =
    Compression::CompressorFactory::_duplicate(compressor_factory);
}

void
omniCompressionManager::
unregister_factory(Compression::CompressorId compressor_id)
{
  omni_tracedmutex_lock l(pd_lock);

  CORBA::ULong idx;
  for (idx=0; idx != pd_factories.length(); ++idx) {
    if (pd_factories[idx]->compressor_id() == compressor_id) {
      CORBA::ULong last = pd_factories.length() - 1;
      if (idx != last) {
        // Swap last item
        pd_factories[idx] = pd_factories[last];
      }
      pd_factories.length(last);
      return;
    }
  }
  throw Compression::UnknownCompressorId();
}

Compression::CompressorFactory_ptr
omniCompressionManager::
get_factory(Compression::CompressorId compressor_id)
{
  omni_tracedmutex_lock l(pd_lock);

  CORBA::ULong idx;
  for (idx=0; idx != pd_factories.length(); ++idx) {
    if (pd_factories[idx]->compressor_id() == compressor_id) {
      return Compression::CompressorFactory::_duplicate(pd_factories[idx]);
    }
  }
  throw Compression::UnknownCompressorId();
}

Compression::Compressor_ptr
omniCompressionManager::
get_compressor(Compression::CompressorId     compressor_id,
               Compression::CompressionLevel compression_level)
{
  Compression::CompressorFactory_var factory = get_factory(compressor_id);
  return factory->get_compressor(compression_level);
}

Compression::CompressorFactorySeq*
omniCompressionManager::
get_factories()
{
  omni_tracedmutex_lock l(pd_lock);
  return new Compression::CompressorFactorySeq(pd_factories);
}

void
omniCompressionManager::_add_ref()
{
  omni_tracedmutex_lock l(pd_lock);
  ++pd_refcount;
}

void
omniCompressionManager::_remove_ref()
{
  {
    omni_tracedmutex_lock l(pd_lock);
    if (--pd_refcount != 0)
      return;
  }
  delete this;
}


static Compression::CompressionManager_var the_manager;

static CORBA::Object_ptr resolveCompressionManagerFn()
{
  return CORBA::Object::_duplicate(the_manager);
}


//
// Internal giopCompressor implementation

static inline CORBA::ULong byteSwap(CORBA::ULong l) {
  return (((l & 0xff000000) >> 24) |
          ((l & 0x00ff0000) >> 8)  |
          ((l & 0x0000ff00) << 8)  |
          ((l & 0x000000ff) << 24));
}

static inline CORBA::UShort byteSwap(CORBA::UShort s) {
  return (((s & 0xff00) >> 8) |
          ((s & 0x00ff) << 8));
}


giopCompressorImpl::
giopCompressorImpl()
  : pd_min_size(g_compression_enabled ? g_compression_low_value : 0xffffffff),
    pd_ratio(g_compression_min_ratio)
{
}


giopCompressorImpl::
giopCompressorImpl(const Compression::CompressorIdLevelList& compressor_ids)
{
  setPolicies(compressor_ids,
              g_compression_enabled ? g_compression_low_value : 0xffffffff,
              g_compression_min_ratio);
}

giopCompressorImpl::
giopCompressorImpl(const Compression::CompressorIdLevelList& compressor_ids,
                   CORBA::ULong low_value, CORBA::Float min_ratio)
  : pd_min_size(low_value),
    pd_ratio(min_ratio)
{
  setPolicies(compressor_ids, low_value, min_ratio);
}


void
giopCompressorImpl::
setPolicies(const Compression::CompressorIdLevelList& compressor_ids,
            CORBA::ULong low_value, CORBA::Float min_ratio)
{
  pd_compressor_ids = compressor_ids;
  pd_compressors.length(compressor_ids.length());

  for (CORBA::ULong idx=0; idx != compressor_ids.length(); ++idx) {
    const Compression::CompressorIdLevel& cid = compressor_ids[idx];
    try {
      pd_compressors[idx] = the_manager->get_compressor(cid.compressor_id,
                                                        cid.compression_level);
    }
    catch (Compression::UnknownCompressorId&) {
      if (omniORB::trace(1)) {
        omniORB::logger log;
        log << "Unknown compressor id " << cid.compressor_id << "\n";
      }
      OMNIORB_THROW(BAD_PARAM,
                    BAD_PARAM_InvalidCompressionLevel,
                    CORBA::COMPLETED_NO);
    }
  }
  pd_min_size = low_value;
  pd_ratio    = min_ratio;
}


giopCompressorImpl::~giopCompressorImpl() {}


giopStream_Buffer*
giopCompressorImpl::compressBuffer(giopStream*        stream,
                                   giopStream_Buffer* g_buf)
{
  CORBA::ULong  g_buf_len   = g_buf->last - g_buf->start;
  if (g_buf_len < pd_min_size)
    return 0;

  CORBA::ULong  g_data_len  = g_buf_len - GIOP_HEADER;
  CORBA::Octet* g_buf_start = (CORBA::Octet*)g_buf + g_buf->start;
  CORBA::Octet* g_data      = g_buf_start + GIOP_HEADER;

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "Compress GIOP message of " << g_buf_len << " bytes\n";
  }
  if (omniORB::trace(30)) {
    giopStream::dumpbuf(g_buf_start, g_buf_len);
  }

  OMNIORB_ASSERT(g_buf_start[0] == 'G' &&
                 g_buf_start[1] == 'I' &&
                 g_buf_start[2] == 'O' &&
                 g_buf_start[3] == 'P');

  // Allocate new buffer
  giopStream_Buffer* z_buf  = giopStream_Buffer::newBuffer(
    g_buf_len + ZIOP_HEADER <= giopStream::bufferSize ?
    giopStream::bufferSize : g_buf_len + ZIOP_HEADER);

  CORBA::ULong  z_buf_len   = z_buf->last - z_buf->start;
  CORBA::ULong  z_data_len  = z_buf_len - GIOP_ZIOP_HEADER;
  CORBA::Octet* z_buf_start = (CORBA::Octet*)z_buf + z_buf->start;
  CORBA::Octet* z_data      = z_buf_start + GIOP_ZIOP_HEADER;

  Compression::Buffer source_seq(g_data_len, g_data_len, g_data);
  Compression::Buffer target_seq(z_data_len, z_data_len, z_data);

  // Compress it
  try {
    pd_compressors[0]->compress(source_seq, target_seq);
  }
  catch (Compression::CompressionException& ex) {
    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "CompressionException: " << ex.reason << " : " << ex.description
          << "\n";
    }
    giopStream_Buffer::deleteBuffer(z_buf);
    return 0;
  }
  catch (...) {
    omniORB::logs(1, "Unexpected exception from compressor.");
    giopStream_Buffer::deleteBuffer(z_buf);
    return 0;
  }

  if (target_seq.NP_data() != z_data) {
    omniORB::logs(10, "Compressor replaced target buffer, "
                  "so it cannot be used.");
    giopStream_Buffer::deleteBuffer(z_buf);
    return 0;
  }

  z_data_len = target_seq.length();

  if (z_data_len > g_data_len * pd_ratio) {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << "Message body of " << g_data_len
          << " bytes compressed insufficiently to " << z_data_len
          << " bytes.\n";
    }
    giopStream_Buffer::deleteBuffer(z_buf);
    return 0;
  }

  // Copy and update GIOP (now ZIOP) header
  memcpy(z_buf_start, g_buf_start, GIOP_HEADER);

  *z_buf_start = 'Z';
  *((CORBA::ULong*) (z_buf_start +  8)) = z_data_len + GIOP_HEADER;
  *((CORBA::UShort*)(z_buf_start + 12)) = pd_compressor_ids[0].compressor_id;
  *((CORBA::ULong*) (z_buf_start + 16)) = g_buf_len - ZIOP_HEADER;
  *((CORBA::ULong*) (z_buf_start + 20)) = z_data_len;

  z_buf->last = z_buf->start + GIOP_ZIOP_HEADER + z_data_len;

  return z_buf;
}


CORBA::ULong
giopCompressorImpl::compressorIndex(giopStream*               stream,
                                    Compression::CompressorId compressor_id)
{
  CORBA::ULong idx;

  // Is there an existing compressor for this id?
  for (idx = 0; idx != pd_compressors.length(); ++idx) {
    if (pd_compressor_ids[idx].compressor_id == compressor_id)
      return idx;
  }

  try {
    Compression::Compressor_ptr compressor;
    compressor = the_manager->get_compressor(compressor_id, 6);

    pd_compressors.length(idx+1);
    pd_compressor_ids.length(idx+1);

    pd_compressors[idx]                  = compressor;
    pd_compressor_ids[idx].compressor_id     = compressor_id;
    pd_compressor_ids[idx].compression_level = 6;
  }
  catch (Compression::UnknownCompressorId&) {
    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "Unknown compressor id " << compressor_id << "\n";
    }
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidCompressedData,
                  (CORBA::CompletionStatus)stream->completion());
  }
  return 0;
}


giopStream_Buffer*
giopCompressorImpl::decompressBuffer(giopStream*        stream,
                                     giopStream_Buffer* z_buf)
{
  CORBA::ULong  z_buf_len   = z_buf->last - z_buf->start;
  CORBA::ULong  z_data_len  = z_buf_len - GIOP_ZIOP_HEADER;
  CORBA::Octet* z_buf_start = (CORBA::Octet*)z_buf + z_buf->start;
  CORBA::Octet* z_data      = z_buf_start + GIOP_ZIOP_HEADER;

  OMNIORB_ASSERT(z_buf_start[0] == 'Z' &&
                 z_buf_start[1] == 'I' &&
                 z_buf_start[2] == 'O' &&
                 z_buf_start[3] == 'P');

  CORBA::UShort compressor_id = *((CORBA::UShort*)(z_buf_start + 12));
  CORBA::ULong  g_data_len    = *((CORBA::ULong*) (z_buf_start + 16));

  if ((z_buf_start[6] & 0x1) != _OMNIORB_HOST_BYTE_ORDER_) {
    compressor_id = byteSwap(compressor_id);
    g_data_len    = byteSwap(g_data_len);
  }

  // Allocate new buffer
  giopStream_Buffer* g_buf  = giopStream_Buffer::newBuffer(
    g_data_len + GIOP_HEADER <= giopStream::bufferSize ?
    giopStream::bufferSize : g_data_len + GIOP_HEADER);

  CORBA::ULong  g_buf_len   = g_buf->last - g_buf->start;
  CORBA::Octet* g_buf_start = (CORBA::Octet*)g_buf + g_buf->start;
  CORBA::Octet* g_data      = g_buf_start + GIOP_HEADER;

  Compression::Buffer source_seq(z_data_len, z_data_len, z_data);
  Compression::Buffer target_seq(g_data_len, g_data_len, g_data);

  if (z_buf_len < z_buf->size) {
    // Buffer does not contain the whole message. Resize the sequence
    // and read the rest in.
    source_seq.length(z_buf->size - GIOP_HEADER);
    stream->inputCopyChunk(source_seq.NP_data() + z_data_len,
                           z_buf->size - z_buf_len);
  }

  CORBA::ULong compressor_index = compressorIndex(stream, compressor_id);

  // Decompress it
  try {
    pd_compressors[compressor_index]->decompress(source_seq, target_seq);
  }
  catch (Compression::CompressionException& ex) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "CompressionException: " << ex.reason << " : " << ex.description
          << "\n";
    }
    giopStream_Buffer::deleteBuffer(g_buf);
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidCompressedData,
                  (CORBA::CompletionStatus)stream->completion());
  }
  catch (...) {
    omniORB::logs(1, "Unexpected exception from compressor.");
    giopStream_Buffer::deleteBuffer(g_buf);
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidCompressedData,
                  (CORBA::CompletionStatus)stream->completion());
  }

  if (target_seq.NP_data() != g_data) {
    omniORB::logs(1, "Compressor replaced target buffer, "
                  "so it cannot be used.");
    giopStream_Buffer::deleteBuffer(g_buf);
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidCompressedData,
                  (CORBA::CompletionStatus)stream->completion());
  }

  if (target_seq.length() != g_data_len) {
    if (omniORB::trace(10)) {
      omniORB::logger log;
      log << "ZIOP message decompressed to " << target_seq.length()
          << " bytes; expecting " << g_data_len << "\n";
    }
    g_data_len = target_seq.length();
  }

  // Copy and update header
  memcpy(g_buf_start, z_buf_start, GIOP_HEADER);

  *g_buf_start = 'G';

  if ((z_buf_start[6] & 0x1) != _OMNIORB_HOST_BYTE_ORDER_)
    *((CORBA::ULong*)(g_buf_start + 8)) = byteSwap(g_data_len);
  else
    *((CORBA::ULong*)(g_buf_start + 8)) = g_data_len;
  
  g_buf->size = g_data_len + GIOP_HEADER;
  g_buf->last = g_buf->start + g_data_len + GIOP_HEADER;

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "Decompressed ZIOP message to " << g_buf->last - g_buf->start
        << " bytes\n";
  }
  if (omniORB::trace(30)) {
    giopStream::dumpbuf((unsigned char*)g_buf + g_buf->start,
                        g_buf->last - g_buf->start);
  }
  return g_buf;
}


//
// Internal giopCompressorFactory implementation

giopCompressorFactoryImpl::~giopCompressorFactoryImpl() {}

giopCompressor*
giopCompressorFactoryImpl::newCompressor()
{
  return new giopCompressorImpl();
}


//
// IOR info

class ZIOPInfo : public omniIOR::IORExtraInfo {
public:
  ZIOPInfo(Messaging::PolicyValueSeq& pvseq)
    : omniIOR::IORExtraInfo(IOP::TAG_POLICIES)
  {
    for (CORBA::ULong idx=0; idx != pvseq.length(); ++idx) {
      Messaging::PolicyValue& pv = pvseq[idx];

      switch (pv.ptype) {
      case ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID:
        {
          cdrEncapsulationStream pstream(pv.pvalue, 1);

          Compression::CompressorIdLevelList cids;
          cids <<= pstream;

          filterCompressors(cids, g_compressor_ids, compressor_ids);
        }
        break;
      }
    }
  }

  virtual ~ZIOPInfo() {}

  Compression::CompressorIdLevelList compressor_ids;

private:
  ZIOPInfo(const ZIOPInfo&);
  ZIOPInfo& operator=(const ZIOPInfo&);
  // Not implemented
};


static CORBA::Boolean
decodeIORInterceptor(omniInterceptors::decodeIOR_T::info_T& iinfo)
{
  const IIOP::ProfileBody& iiop = iinfo.iiop;
  omniIOR&                 ior  = iinfo.ior;

  const IOP::MultipleComponentProfile& components = iiop.components;

  CORBA::ULong index;

  try {
    for (index=0; index < components.length(); ++index) {
      if (components[index].tag == IOP::TAG_POLICIES) {
	omniORB::logs(30, "Found a Messaging Policies IOR component.");

	const IOP::TaggedComponent& c = components[index];
	cdrEncapsulationStream stream(c.component_data, 1);

        Messaging::PolicyValueSeq pvseq;
        pvseq <<= stream;

        CORBA::Boolean enable_ziop = 0;

        // Is there a ZIOP enabling policy?
        for (CORBA::ULong pvidx = 0; pvidx != pvseq.length(); ++pvidx) {
          if (pvseq[pvidx].ptype == ZIOP::COMPRESSION_ENABLING_POLICY_ID) {

            cdrEncapsulationStream pstream(pvseq[pvidx].pvalue, 1);
            enable_ziop = pstream.unmarshalBoolean();
            break;
          }
        }
        if (!enable_ziop) {
          omniORB::logs(30, "Messaging Policies IOR component does not "
                        "enable ZIOP.");
          return 1;
        }

        ZIOPInfo* zinfo = new ZIOPInfo(pvseq);

        if (zinfo->compressor_ids.length()) {
          // Add the information to the omniIOR's extra info list.
          omniIOR::IORInfo* info = ior.getIORInfo();

          info->flags(info->flags() | GIOPSTRAND_COMPRESSION);

          omniIOR::IORExtraInfoList& infolist = info->extraInfo();
          CORBA::ULong i = infolist.length();
          infolist.length(i+1);
          infolist[i] = (omniIOR::IORExtraInfo*)zinfo;
        }
        else {
          delete zinfo;
          omniORB::logs(25, "No matching compressors found in "
                        "Messaging Policies IOR component.");
        }
        return 1;
      }
    }
  }
  catch (CORBA::SystemException&) {
    omniORB::logs(10, "Invalid Messaging Policies IOR component encountered.");
  }
  return 1;
}


static CORBA::Boolean
encodeIORInterceptor(omniInterceptors::encodeIOR_T::info_T& iinfo)
{
  const GIOP::Version&           version    = iinfo.iiop.version;
  IOP::MultipleComponentProfile& components = iinfo.iiop.components;
  const CORBA::PolicyList*       policies   = iinfo.hints.policies;

  // Only run if version is 1.2+
  if (!(version.major > 1 || version.minor >= 2))
    return 1;

  setPoliciesInProfile(policies, components);

  return 1;
}


static
CORBA::Boolean
clientSendRequestInterceptor(omniInterceptors::
                             clientSendRequest_T::info_T& iinfo)
{
  GIOP_C&     giop_c = iinfo.giop_c;
  giopStrand& strand = giop_c.strand();

  if (!(strand.first_call && strand.flags & GIOPSTRAND_COMPRESSION)) {
    // Only consider strands on the first call and if the compression
    // flag is set.
    return 1;
  }

  omniORB::logs(25, "Send ZIOP enabling service context");

  OMNIORB_ASSERT(!strand.compressor);

  CORBA::ULong idx;
  
  // Send service context and create compressor
  Messaging::PolicyValueSeq pvseq(2);
  
  {
    // Enabling policy
    cdrEncapsulationStream pvstream;
    pvstream.marshalBoolean(1);

    pvseq.length(1);
    pvseq[0].ptype = ZIOP::COMPRESSION_ENABLING_POLICY_ID;
    pvstream.setOctetSeq(pvseq[0].pvalue);
  }

  {
    // Compressor ids
    const omniIOR::IORInfo*          info     = giop_c.ior()->getIORInfo();
    const omniIOR::IORExtraInfoList& infolist = info->extraInfo();

    for (idx = 0; idx != infolist.length(); ++idx) {
      if (infolist[idx]->compid == IOP::TAG_POLICIES) {
        const ZIOPInfo* zinfo = (const ZIOPInfo*)infolist[idx];
        
        cdrEncapsulationStream pvstream;
        zinfo->compressor_ids >>= pvstream;

        pvseq.length(2);
        pvseq[1].ptype = ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID;
        pvstream.setOctetSeq(pvseq[1].pvalue);

        strand.compressor = new giopCompressorImpl(zinfo->compressor_ids);
        break;
      }
    }
  }
  
  OMNIORB_ASSERT(pvseq.length() == 2);

  cdrEncapsulationStream scstream;
  pvseq >>= scstream;

  idx = iinfo.service_contexts.length();
  iinfo.service_contexts.length(idx+1);
  iinfo.service_contexts[idx].context_id = IOP::INVOCATION_POLICIES;
  scstream.setOctetSeq(iinfo.service_contexts[idx].context_data);

  return 1;
}


static
CORBA::Boolean
serverReceiveRequestInterceptor(omniInterceptors::
                                serverReceiveRequest_T::info_T& iinfo)
{
  GIOP_S&     giop_s = iinfo.giop_s;
  giopStrand& strand = giop_s.strand();

  if (strand.flags & GIOPSTRAND_COMPRESSION) {
    // Already set up for compression
    return 1;
  }

  IOP::ServiceContextList& svc_ctxts = giop_s.service_contexts();

  CORBA::ULong total = svc_ctxts.length();

  for (CORBA::ULong idx = 0; idx != total; ++idx) {

    if (svc_ctxts[idx].context_id == IOP::INVOCATION_POLICIES) {
      omniORB::logs(30, "Unmarshal invocation policies service context.");

      cdrEncapsulationStream pvstream(svc_ctxts[idx].context_data, 1);

      Messaging::PolicyValueSeq pvseq;
      pvseq <<= pvstream;

      CORBA::Boolean                     enable_ziop = 0;
      Compression::CompressorIdLevelList compressor_ids;

      // Is there a ZIOP enabling policy?
      for (CORBA::ULong pvidx = 0; pvidx != pvseq.length(); ++pvidx) {
        Messaging::PolicyValue& pv = pvseq[pvidx];

        switch (pv.ptype) {
        case ZIOP::COMPRESSION_ENABLING_POLICY_ID:
          {
            cdrEncapsulationStream pstream(pv.pvalue, 1);
            enable_ziop = pstream.unmarshalBoolean();
            break;
          }
        case ZIOP::COMPRESSOR_ID_LEVEL_LIST_POLICY_ID:
          {
            cdrEncapsulationStream pstream(pv.pvalue, 1);
            compressor_ids <<= pstream;
            break;
          }
        }
      }
      if (!enable_ziop) {
        omniORB::logs(30, "Invocations Policies service context does not "
                      "enable ZIOP.");
        continue;
      }

      // Check serverTransportRule to see if we should allow ziop from
      // this client.
      {
	transportRules::sequenceString actions;
	CORBA::ULong   matched_rule;
	CORBA::Boolean accept_ziop;
	CORBA::Boolean dump_rule = 0;
	const char*    why;

        transportRules& rules = transportRules::serverRules();

	if ((accept_ziop = rules.match(strand.connection->peeraddress(),
                                       actions, matched_rule))) {
	  CORBA::ULong i;
	  for (i = 0; i != actions.length(); ++i) {
	    if (strcmp(actions[i], "ziop") == 0) {
	      break;
	    }
	  }
	  if (i == actions.length()) {
	    accept_ziop = 0;
	    dump_rule   = 1;
	    why = (const char*)"\"ziop\" is not found in the matched rule: ";
	  }
	}
	else {
	  why = (const char*)"no matching rule is found";
	}

	if (!accept_ziop) {
	  if (omniORB::trace(5)) {
	    omniORB::logger log;
	    log << "Request from " << strand.connection->peeraddress()
		<< " to enable ZIOP is rejected because " 	
		<< why;

	    if (dump_rule) {
	      CORBA::String_var rule;
	      rule = rules.dumpRule(matched_rule);
	      log << "\"" << (const char*)rule << "\"";
	    }
	    log << "\n";
	  }
	  continue;
	}

        // Filter compressor_ids according to our policies.

        Compression::CompressorIdLevelList my_ids(g_compressor_ids);
        CORBA::ULong low_value = g_compression_low_value;
        CORBA::Float min_ratio = g_compression_min_ratio;

        omniOrbPOA* poa =
          omniOrbPOA::_downcast(omniObjAdapter::getAdapter(giop_s.key(),
                                                           giop_s.keysize()));

        if (poa) {
          CORBA::Boolean enabled;
          decodePolicies(*poa->policy_list(), enabled, my_ids,
                         low_value, min_ratio);
        }

        Compression::CompressorIdLevelList selected_ids;
        filterCompressors(compressor_ids, my_ids, selected_ids);

        if (!selected_ids.length()) {
	  if (omniORB::trace(5)) {
	    omniORB::logger log;
	    log << "Request from " << strand.connection->peeraddress()
		<< " to enable ZIOP is rejected because there are no"
                << " matching compressors.\n";
	  }
	  continue;
	}

	if (omniORB::trace(5)) {
	  CORBA::String_var rule;
	  rule = transportRules::serverRules().dumpRule(matched_rule);

	  omniORB::logger log;
	  log << "Accepted request from " 
	      << strand.connection->peeraddress()
	      << " to enable ZIOP because of this rule: \""
	      << (const char*)rule << "\"\n";
	}

        // Set the strand's compressor
        if (strand.compressor) {
          giopCompressorImpl* compressor =
            (giopCompressorImpl*)strand.compressor;

          compressor->setPolicies(selected_ids, low_value, min_ratio);
        }
        else {
          strand.compressor = new giopCompressorImpl(selected_ids, low_value,
                                                     min_ratio);
        }

        strand.flags |= GIOPSTRAND_COMPRESSION;

        return 1;
      }
    }
  }
  return 1;
}


//
// Module initialiser

class omniZIOP_initialiser : public omniInitialiser {
public:
  inline omniZIOP_initialiser() {
    omniInitialiser::install(this);

    omniInitialReferences::registerPseudoObjFn("CompressionManager",
					       resolveCompressionManagerFn);
  }
  void attach() {
    the_manager = new omniCompressionManager();
    
    Compression::CompressorFactory_var zf = new zlibCompressorFactory();
    the_manager->register_factory(zf);

    giopStream::compressorFactory = new giopCompressorFactoryImpl();

    // Register interceptors
    omniInterceptors* interceptors = omniORB::getInterceptors();

    interceptors->decodeIOR           .add(decodeIORInterceptor);
    interceptors->encodeIOR           .add(encodeIORInterceptor);
    interceptors->clientSendRequest   .add(clientSendRequestInterceptor);
    interceptors->serverReceiveRequest.add(serverReceiveRequestInterceptor);

    omniORB::logs(2, "omniZIOP activated.");
  }

  void detach() {
    omniORB::logs(2, "Deactivate omniZIOP.");
    delete giopStream::compressorFactory;
    giopStream::compressorFactory = 0;

    omniInterceptors* interceptors = omniORB::getInterceptors();

    interceptors->decodeIOR           .remove(decodeIORInterceptor);
    interceptors->encodeIOR           .remove(encodeIORInterceptor);
    interceptors->clientSendRequest   .remove(clientSendRequestInterceptor);
    interceptors->serverReceiveRequest.remove(serverReceiveRequestInterceptor);

    the_manager = 0;
    omniORB::logs(2, "omniZIOP deactivated.");
  }
};

static omniZIOP_initialiser the_omniZIOP_initialiser;

OMNI_NAMESPACE_END(omni)
