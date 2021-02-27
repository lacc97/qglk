#include "chunk.hpp"

#include <cassert>

#include <mutex>
#include <unordered_map>

namespace {
    class SafeMap {
      public:
        [[nodiscard]] std::shared_ptr<Glk::Blorb::Chunk::Data> get(glui32 number) const noexcept {
            std::lock_guard lock{m_mutex};
            auto it = m_map.find(number);
            if(it != m_map.end())
                return it->second.lock();
            else
                return {};
        }
        void set(glui32 number, std::weak_ptr<Glk::Blorb::Chunk::Data> ptr) noexcept {
            std::lock_guard lock{m_mutex};
            m_map.try_emplace(number, std::move(ptr));
        }

      private:
        mutable std::mutex m_mutex;
        std::unordered_map<glui32, std::weak_ptr<Glk::Blorb::Chunk::Data>> m_map;
    };

    SafeMap s_chunk_map;
}

bool Glk::Blorb::isChunkLoaded(glui32 chunknum) noexcept {
    return static_cast<bool>(s_chunk_map.get(chunknum));
}

bool Glk::Blorb::isResourceLoaded(glui32 filenum, Glk::Blorb::ResourceUsage usage) noexcept {
    giblorb_map_t* rmap;
    if(!(rmap = giblorb_get_resource_map()))
        return false;

    giblorb_result_t res;
    if(giblorb_load_resource(rmap, giblorb_method_DontLoad, &res, static_cast<glui32>(usage), filenum) != giblorb_err_None)
        return false;

    return static_cast<bool>(s_chunk_map.get(res.chunknum));
}
Glk::Blorb::Chunk Glk::Blorb::loadResource(glui32 filenum, Glk::Blorb::ResourceUsage usage) noexcept {
    giblorb_map_t* rmap;

    if(!(rmap = giblorb_get_resource_map()))
        return Chunk();

    giblorb_result_t res;

    if(giblorb_load_resource(rmap, giblorb_method_DontLoad, &res, static_cast<glui32>(usage), filenum) != giblorb_err_None)
        return Chunk();

    return loadChunk(res.chunknum);
}

Glk::Blorb::Chunk Glk::Blorb::loadChunkByType(glui32 chunktype, glui32 count) noexcept {
    giblorb_map_t* rmap;
    if(!(rmap = giblorb_get_resource_map()))
        return {};

    giblorb_result_t res;
    if(giblorb_load_chunk_by_type(rmap, giblorb_method_DontLoad, &res, static_cast<glui32>(chunktype), count) != giblorb_err_None)
        return {};

    return Chunk::loadByNumber(res.chunknum);
}

Glk::Blorb::Chunk Glk::Blorb::Chunk::loadByNumber(glui32 number) noexcept {
    constexpr auto fn_deleter = [](Chunk::Data* ptr) -> void {
      if(!ptr)
          return;

      giblorb_unload_chunk(giblorb_get_resource_map(), ptr->number);
      delete ptr;
    };

    if(auto ptr = s_chunk_map.get(number))
        return Chunk{std::move(ptr)};

    giblorb_map_t* rmap;
    if(!(rmap = giblorb_get_resource_map()))
        return {};

    giblorb_result_t res;
    if(giblorb_load_chunk_by_number(rmap, giblorb_method_Memory, &res, number) != giblorb_err_None)
        return {};

    std::shared_ptr<Data> ptr(new Data{static_cast<Type>(res.chunktype), res.chunknum, res.length, res.data.ptr}, fn_deleter);
    s_chunk_map.set(number, ptr);
    return Chunk{std::move(ptr)};
}
