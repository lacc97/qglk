#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <memory>

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

        class Chunk {
                friend bool unloadChunk(Chunk& chunk);
            public:
                enum class Type : glui32 {
                    NONE = 0,
                    ANNO = giblorb_ID_ANNO,
                    AUTH = giblorb_ID_AUTH,
                    BINA = giblorb_ID_BINA,
                    JPEG = giblorb_ID_JPEG,
                    PNG = giblorb_ID_PNG,
                    TEXT = giblorb_ID_TEXT
                };

                struct Data {
                    Type type;
                    glui32 number, length;
                    const void* data;
                };

                static Chunk loadByNumber(glui32 number) noexcept;

                constexpr Chunk() noexcept = default;

                [[nodiscard]] inline glui32 number() const {
                    return isValid() ? mp_Data->number : 0;
                }
                [[nodiscard]] inline const char* data() const {
                    return isValid() ? static_cast<const char*>(mp_Data->data) : nullptr;
                }
                [[nodiscard]] inline glui32 length() const {
                    return isValid() ? mp_Data->length : 0;
                }
                [[nodiscard]] inline Type type() const {
                    return isValid() ? mp_Data->type : Type::NONE;
                }

                [[nodiscard]] inline bool isValid() const {
                    return static_cast<bool>(mp_Data);
                }

            private:
                explicit Chunk(std::shared_ptr<Data> data) noexcept : mp_Data{std::move(data)} {}

                std::shared_ptr<Data> mp_Data;
        };

        Chunk loadResource(glui32 filenum, ResourceUsage usage = ResourceUsage::None) noexcept;
        [[deprecated]] bool isResourceLoaded(glui32 filenum, ResourceUsage usage = ResourceUsage::None) noexcept;

        inline Chunk loadChunk(glui32 chunknum) noexcept {
            return Chunk::loadByNumber(chunknum);
        }
        Chunk loadChunkByType(glui32 chunktype, glui32 count) noexcept;
        [[deprecated]] bool isChunkLoaded(glui32 chunknum) noexcept;
    }
}

#endif



