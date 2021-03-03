#include "chunk.hpp"

#include <cassert>

#include <mutex>
#include <unordered_map>

namespace {
    class safe_map {
      public:
        [[nodiscard]] std::shared_ptr<qglk::blorb::chunk::data> get(glui32 number) const noexcept {
            std::lock_guard lock{m_mutex};
            auto it = m_map.find(number);
            if(it != m_map.end())
                return it->second.lock();
            else
                return {};
        }
        void set(glui32 number, std::weak_ptr<qglk::blorb::chunk::data> ptr) noexcept {
            std::lock_guard lock{m_mutex};
            m_map.try_emplace(number, std::move(ptr));
        }

      private:
        mutable std::mutex m_mutex;
        std::unordered_map<glui32, std::weak_ptr<qglk::blorb::chunk::data>> m_map;
    };

    safe_map s_chunk_map;
}    // namespace

qglk::blorb::chunk qglk::blorb::load_resource(glui32 filenum, qglk::blorb::resource_usage usage) noexcept {
    giblorb_map_t* rmap;

    if(!(rmap = giblorb_get_resource_map()))
        return chunk();

    giblorb_result_t res;

    if(giblorb_load_resource(rmap, giblorb_method_DontLoad, &res, static_cast<glui32>(usage), filenum) != giblorb_err_None)
        return chunk();

    return load_chunk(res.chunknum);
}

qglk::blorb::chunk qglk::blorb::load_chunk_by_type(glui32 chunktype, glui32 count) noexcept {
    giblorb_map_t* rmap;
    if(!(rmap = giblorb_get_resource_map()))
        return {};

    giblorb_result_t res;
    if(giblorb_load_chunk_by_type(rmap, giblorb_method_DontLoad, &res, static_cast<glui32>(chunktype), count) != giblorb_err_None)
        return {};

    return chunk::load_by_number(res.chunknum);
}

qglk::blorb::chunk qglk::blorb::chunk::load_by_number(glui32 number) noexcept {
    constexpr auto fn_deleter = [](chunk::data* ptr) -> void {
        if(!ptr)
            return;

        giblorb_unload_chunk(giblorb_get_resource_map(), ptr->number);
        delete ptr;
    };

    if(auto ptr = s_chunk_map.get(number))
        return chunk{std::move(ptr)};

    giblorb_map_t* rmap;
    if(!(rmap = giblorb_get_resource_map()))
        return {};

    giblorb_result_t res;
    if(giblorb_load_chunk_by_number(rmap, giblorb_method_Memory, &res, number) != giblorb_err_None)
        return {};

    std::shared_ptr<data> ptr(new data{static_cast<type>(res.chunktype), res.chunknum, res.length, res.data.ptr}, fn_deleter);
    s_chunk_map.set(number, ptr);
    return chunk{std::move(ptr)};
}
