#pragma once

#include <cstddef>
#include <cstring>
#include <algorithm>

#ifdef __cplusplus
extern "C"{
#endif

struct buffer_s {
    char* data;
    size_t size;
    size_t read_pos;
    size_t write_pos;
};

static inline struct buffer_s* ox_buffer_new(size_t size) {
    struct buffer_s* buf = new struct buffer_s;
    buf->data = new char[size];
    buf->size = size;
    buf->read_pos = 0;
    buf->write_pos = 0;
    return buf;
}

static inline void ox_buffer_delete(struct buffer_s* buf) {
    if (buf) {
        delete[] buf->data;
        delete buf;
    }
}

static inline char* ox_buffer_getwriteptr(struct buffer_s* buf) {
    return buf->data + buf->write_pos;
}

static inline size_t ox_buffer_getwritevalidcount(const struct buffer_s* buf) {
    return buf->size - buf->write_pos;
}

static inline void ox_buffer_addwritepos(struct buffer_s* buf, size_t n) {
    buf->write_pos += n;
}

static inline size_t ox_buffer_getreadvalidcount(const struct buffer_s* buf) {
    return buf->write_pos - buf->read_pos;
}

static inline const char* ox_buffer_getreader(const struct buffer_s* buf) {
    return buf->data + buf->read_pos;
}

// 有些地方可能使用了 getreadptr，与 getreader 功能相同
static inline const char* ox_buffer_getreadptr(const struct buffer_s* buf) {
    return buf->data + buf->read_pos;
}

static inline void ox_buffer_addreadpos(struct buffer_s* buf, size_t n) {
    buf->read_pos += n;
}

static inline size_t ox_buffer_getsize(const struct buffer_s* buf) {
    return buf->size;
}

// 将未读数据移到缓冲区头部，重置读写指针
static inline void ox_buffer_adjustto_head(struct buffer_s* buf) {
    if (buf->read_pos > 0) {
        size_t valid = buf->write_pos - buf->read_pos;
        if (valid > 0) {
            memmove(buf->data, buf->data + buf->read_pos, valid);
        }
        buf->write_pos = valid;
        buf->read_pos = 0;
    }
}

// 将 src 指向的 len 字节数据写入 dest 缓冲区的当前写位置，并更新写指针
static inline void ox_buffer_write(struct buffer_s* dest, const char* src, size_t len) {
    memcpy(dest->data + dest->write_pos, src, len);
    dest->write_pos += len;
}


#ifdef __cplusplus
}
#endif