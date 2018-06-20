#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <QSharedPointer>

#include "glk.hpp"

namespace Glk {
    namespace Blorb {
        enum class LoadMethod : glui32 {
            DontLoad = giblorb_method_DontLoad,
            Memory = giblorb_method_Memory,
            FilePos = giblorb_method_FilePos
        };

        enum class ResourceUsage : glui32 {
            None = 0,
            Picture = giblorb_ID_Pict,
            Sound = giblorb_ID_Snd,
            Executable = giblorb_ID_Exec
        };

        enum class ChunkType : glui32 {
            NONE = 0,
            ANNO = giblorb_ID_ANNO,
            AUTH = giblorb_ID_AUTH,
            BINA = giblorb_ID_BINA,
            JPEG = giblorb_ID_JPEG,
            PNG = giblorb_ID_PNG,
            TEXT = giblorb_ID_TEXT
        };

        // NOT THREAD SAFE
        class Chunk {
                friend bool unloadChunk(Chunk& chunk);
            public:
                Chunk(giblorb_map_t* map_ = NULL, glui32 chunknum_ = 0, glui32 startpos_ = 0, void* ptr_ = NULL, glui32 length_ = 0, ChunkType type_ = ChunkType::NONE);
                Chunk(const Chunk&);
                Chunk(Chunk&&);
                ~Chunk();

                Chunk& operator=(const Chunk&);
                Chunk& operator=(Chunk &&);

                inline glui32 number() const {
                    return m_Number;
                }
                inline glui32 startPosition() const {
                    return m_StartPosition;
                }
                inline const char* data() const {
                    return mp_Data;
                }
                inline glui32 length() const {
                    return m_Length;
                }
                inline ChunkType type() const {
                    return m_Type;
                }

                inline bool isValid() const {
                    return mp_Data != NULL;
                }

            private:
                giblorb_map_t* mp_Map;
                glui32 m_Number;
                glui32 m_StartPosition;
                glui32* mp_Count;
                char* mp_Data;
                glui32 m_Length;
                ChunkType m_Type;
        };

        Chunk loadResource(glui32 filenum, ResourceUsage usage = ResourceUsage::None);
        bool isResourceLoaded(glui32 filenum, ResourceUsage usage = ResourceUsage::None);

        Chunk loadChunk(glui32 chunknum);
        Chunk loadChunkByType(glui32 chunktype, glui32 count);
        bool isChunkLoaded(glui32 chunknum);
        bool unloadChunk(Chunk& chunk);
    }
}

#endif



