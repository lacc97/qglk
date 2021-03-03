#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <memory>
#include <span>

#include "glk.hpp"

namespace qglk {
    namespace blorb {
        enum class load_method : glui32 { e_DontLoad = giblorb_method_DontLoad, e_Memory = giblorb_method_Memory, e_FilePos = giblorb_method_FilePos };

        enum class resource_usage : glui32 { e_None = 0, e_Picture = giblorb_ID_Pict, e_Sound = giblorb_ID_Snd, e_Executable = giblorb_ID_Exec };

        class chunk {
          public:
            enum class type : glui32 {
                e_None = 0,
                e_ANNO = giblorb_ID_ANNO,
                e_AUTH = giblorb_ID_AUTH,
                e_BINA = giblorb_ID_BINA,
                e_JPEG = giblorb_ID_JPEG,
                e_PNG = giblorb_ID_PNG,
                e_TEXT = giblorb_ID_TEXT
            };

            struct data {
                enum type type;
                glui32 number, length;
                const void* data;
            };

            static chunk load_by_number(glui32 number) noexcept;

            constexpr chunk() noexcept = default;

            [[nodiscard]] inline glui32 get_number() const {
                return is_valid() ? mp_data->number : 0;
            }
            [[nodiscard]] inline std::span<const char> get_data() const {
                return is_valid() ? std::span{static_cast<const char*>(mp_data->data), mp_data->length} : std::span<const char>{};
            }
            [[nodiscard]] inline type get_type() const {
                return is_valid() ? mp_data->type : type::e_None;
            }

            [[nodiscard]] inline bool is_valid() const {
                return static_cast<bool>(mp_data);
            }

          private:
            explicit chunk(std::shared_ptr<data> data) noexcept : mp_data{std::move(data)} {}

            std::shared_ptr<data> mp_data;
        };

        chunk load_resource(glui32 filenum, resource_usage usage = resource_usage::e_None) noexcept;

        inline chunk load_chunk(glui32 chunknum) noexcept {
            return chunk::load_by_number(chunknum);
        }
        chunk load_chunk_by_type(glui32 chunktype, glui32 count) noexcept;
    }    // namespace blorb
}    // namespace Glk

#endif
