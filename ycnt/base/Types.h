//
// Created by jason on 2019/9/18.
//

#ifndef YCNT_YCNT_BASE_TYPES_H_
#define YCNT_YCNT_BASE_TYPES_H_

#define CACHE_LINE 64

#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))

#define DISALLOW_COPY_AND_ASSIGN(TypeName)          \
TypeName(const TypeName&) = delete;                 \
TypeName& operator=(const TypeName&) = delete

#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName)     \
TypeName(const TypeName&) = delete;                 \
TypeName& operator=(const TypeName&) = delete;      \
TypeName(TypeName&&) = delete;                      \
TypeName& operator=(const TypeName&&) = delete

#endif //YCNT_YCNT_BASE_TYPES_H_
