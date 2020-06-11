#ifndef QGLK_CHUNKBUF_HPP
#define QGLK_CHUNKBUF_HPP

#include <memory>

#include "blorb/chunk.hpp"

#include "membuf.hpp"

namespace Glk {
    class ChunkBuf : public MemBuf<true> {
        public:
            ChunkBuf(Blorb::Chunk chunk);

        private:
            Blorb::Chunk m_Chunk;
    };
}

#endif //QGLK_CHUNKBUF_HPP
