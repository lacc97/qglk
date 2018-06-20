#include "chunk.hpp"

#include <cassert>

#include <QHash>

Glk::Blorb::Chunk::Chunk(giblorb_map_t* map_, glui32 chunknum_, glui32 startpos_, void* ptr_, glui32 length_, Glk::Blorb::ChunkType type_) : mp_Map(map_), m_Number(chunknum_), m_StartPosition(startpos_), mp_Count(), mp_Data(reinterpret_cast<char*>(ptr_)), m_Length(length_), m_Type(type_) {
    if(mp_Data) {
        assert(mp_Map);
        mp_Count = new glui32(1);
    }
}

Glk::Blorb::Chunk::Chunk(const Glk::Blorb::Chunk& c) : mp_Map(c.mp_Map), m_Number(c.m_Number), m_StartPosition(c.m_StartPosition), mp_Count(c.mp_Count), mp_Data(c.mp_Data), m_Length(c.m_Length), m_Type(c.m_Type) {
    if(c.isValid())
        (*mp_Count)++;
}

Glk::Blorb::Chunk::Chunk(Glk::Blorb::Chunk&&  c) : mp_Map(c.mp_Map), m_Number(c.m_Number), m_StartPosition(c.m_StartPosition), mp_Count(c.mp_Count), mp_Data(c.mp_Data), m_Length(c.m_Length), m_Type(c.m_Type) {
    c.mp_Count = NULL;
    c.mp_Data = NULL;
}

Glk::Blorb::Chunk::~Chunk() {
    if(!mp_Data)
        return;

    if(!mp_Count)
        return;

    (*mp_Count)--;

    if((*mp_Count) != 0)
        return;

    delete mp_Count;
    giblorb_unload_chunk(mp_Map, m_Number);
}

Glk::Blorb::Chunk& Glk::Blorb::Chunk::operator=(const Glk::Blorb::Chunk& c) {
    if(mp_Data == c.mp_Data && mp_Count == c.mp_Count)
        return (*this);

    if(mp_Data && mp_Count) {
        (*mp_Count)--;

        if((*mp_Count) == 0) {
            delete mp_Count;
            giblorb_unload_chunk(mp_Map, m_Number);
        }
    }

    mp_Map = c.mp_Map;
    m_Number = c.m_Number;
    m_StartPosition = c.m_StartPosition;
    mp_Count = c.mp_Count;
    mp_Data = c.mp_Data;
    m_Length = c.m_Length;
    m_Type = c.m_Type;

    if(c.isValid())
        (*mp_Count)++;

    return (*this);
}

Glk::Blorb::Chunk& Glk::Blorb::Chunk::operator=(Glk::Blorb::Chunk && c) {
    if(mp_Data == c.mp_Data && mp_Count == c.mp_Count)
        return (*this);

    if(mp_Data && mp_Count) {
        (*mp_Count)--;

        if((*mp_Count) == 0) {
            delete mp_Count;
            giblorb_unload_chunk(mp_Map, m_Number);
        }
    }
    
    mp_Map = c.mp_Map;
    m_Number = c.m_Number;
    m_StartPosition = c.m_StartPosition;
    mp_Count = c.mp_Count;
    mp_Data = c.mp_Data;
    m_Length = c.m_Length;
    m_Type = c.m_Type;

    c.mp_Count = NULL;
    c.mp_Data = NULL;

    return (*this);
}

Glk::Blorb::Chunk Glk::Blorb::loadResource(glui32 filenum, Glk::Blorb::ResourceUsage usage) {
    giblorb_map_t* rmap;

    if(!(rmap = giblorb_get_resource_map()))
        return Chunk();

    giblorb_result_t res;

    if(giblorb_load_resource(rmap, giblorb_method_DontLoad, &res, static_cast<glui32>(usage), filenum) != giblorb_err_None)
        return Chunk();

    return loadChunk(res.chunknum);
}

bool Glk::Blorb::isResourceLoaded(glui32 filenum, Glk::Blorb::ResourceUsage usage) {
    giblorb_map_t* rmap;

    if(!(rmap = giblorb_get_resource_map()))
        return false;

    giblorb_result_t res;

    if(giblorb_load_resource(rmap, giblorb_method_DontLoad, &res, static_cast<glui32>(usage), filenum) != giblorb_err_None)
        return false;

    return isChunkLoaded(res.chunknum);
}

QHash<glui32, Glk::Blorb::Chunk> s_ChunkMap;
Glk::Blorb::Chunk Glk::Blorb::loadChunk(glui32 chunknum) {
    if(s_ChunkMap.contains(chunknum))
        return s_ChunkMap[chunknum];

    giblorb_map_t* rmap;

    if(!(rmap = giblorb_get_resource_map()))
        return Chunk();

    giblorb_result_t res;

    if(giblorb_load_chunk_by_number(rmap, giblorb_method_FilePos, &res, chunknum) != giblorb_err_None)
        return Chunk();

    glui32 startpos = res.data.startpos;

    if(giblorb_load_chunk_by_number(rmap, giblorb_method_Memory, &res, chunknum) != giblorb_err_None)
        return Chunk();

    s_ChunkMap[res.chunknum] = Chunk(rmap, res.chunknum, startpos, res.data.ptr, res.length, static_cast<ChunkType>(res.chunktype));

    return s_ChunkMap[res.chunknum];
}

Glk::Blorb::Chunk Glk::Blorb::loadChunkByType(glui32 chunktype, glui32 count) {
    giblorb_map_t* rmap;

    if(!(rmap = giblorb_get_resource_map()))
        return Chunk();

    giblorb_result_t res;

    if(giblorb_load_chunk_by_type(rmap, giblorb_method_FilePos, &res, chunktype, count) != giblorb_err_None)
        return Chunk();

    glui32 startpos = res.data.startpos;
    glui32 chunknum = res.chunknum;

    if(giblorb_load_chunk_by_number(rmap, giblorb_method_Memory, &res, chunknum) != giblorb_err_None)
        return Chunk();

    s_ChunkMap[res.chunknum] = Chunk(rmap, res.chunknum, startpos, res.data.ptr, res.length, static_cast<ChunkType>(res.chunktype));

    return s_ChunkMap[res.chunknum];
}

bool Glk::Blorb::isChunkLoaded(glui32 chunknum) {
    return s_ChunkMap.contains(chunknum);
}

bool Glk::Blorb::unloadChunk(Glk::Blorb::Chunk& chunk) {
    if(!chunk.isValid())
        return true;
    
    glui32 chunknum;
    {
        Chunk temp(std::move(chunk));
        chunknum = temp.number();
    } // temp destroyed, chunk invalid

    if((*s_ChunkMap[chunknum].mp_Count) != 1)
        return false;

    s_ChunkMap.remove(chunknum);
    return true;
}

