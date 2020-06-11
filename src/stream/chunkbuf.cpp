#include "chunkbuf.hpp"

Glk::ChunkBuf::ChunkBuf(Glk::Blorb::Chunk chunk)
        : MemBuf<true>(chunk.data(), chunk.length()), m_Chunk{std::move(chunk)} {
    assert(m_Chunk.isValid());
}
