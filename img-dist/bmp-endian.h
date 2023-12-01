#ifndef BMP_ENDIAN_H__
#define BMP_ENDIAN_H__

/**
 * Gestion du boutisme. Utile car le format BMP utilise des valeurs
 * encodées en petit boutisme (little endian) et qu'il faut donc être
 * capable de les interpréter correctement même sur une machine avec
 * un grand boutisme.
 * 
 * Les architectures exotiques qui n'utilisent ni le grand ni le petit
 * boutisme ne sont pas supportées ici.
 **/

#if (defined(__GNUC__) || defined(__CLANG__))
   #define SWAP_FCT_16(v) __builtin_bswap16(v)
   #define SWAP_FCT_32(v) __builtin_bswap32(v)
   #define SWAP_FCT_64(v) __builtin_bswap64(v)
#elif (defined(_MSC_VER))
   #define SWAP_FCT_16(v) _byteswap_ushort(v)
   #define SWAP_FCT_32(v) _byteswap_ulong(v)
   #define SWAP_FCT_64(v) _byteswap_uint64(v)
#else
   #define SWAP_FCT_16(v) ((((v) & 0x0FFu) << 8) | (((v) & 0xFF00u) >> 8))
   #define SWAP_FCT_32(v) ((((v) & 0xFF000000u) >> 24) | (((v) & 0x00FF0000u) >> 8) | (((v) & 0x0000FF00u) << 8) | (((v) & 0x000000FFu) << 24))
   #define SWAP_FCT_64(v) ((((v) & 0xFF00000000000000ull) >> 56) | ((((v) & 0x00FF000000000000ull) >> 40))  \
                          |((((v) & 0x0000FF0000000000ull) >> 24)) | ((((v) & 0x000000FF00000000ull) >> 8)) \
                          |(((v) & 0x00000000000000FFull) << 56) | ((((v) & 0x00000000000000FF00ull) << 40))\
                          |((((v) & 0x0000000000FF0000ull) << 24)) | ((((v) & 0x00000000FF000000ull) << 8)))
#endif

#ifdef __cplusplus

#define IS_LITTLEENDIAN IsLittleEndian_()

static constexpr uint32_t endianness__(1);
static inline constexpr bool IsLittleEndian_() {
    return *reinterpret_cast<const uint8_t*>(&endianness__) == 1;
}

#else

#define IS_LITTLEENDIAN ((*(uint8_t*)&((uint32_t){1})) == 1)

#endif

#define TO_LITTLE_ENDIAN8(v) (v)
#define TO_LITTLE_ENDIAN16(v) (IS_LITTLEENDIAN ? (v) : SWAP_FCT_16(v))
#define TO_LITTLE_ENDIAN32(v) (IS_LITTLEENDIAN ? (v) : SWAP_FCT_32(v))
#define TO_LITTLE_ENDIAN64(v) (IS_LITTLEENDIAN ? (v) : SWAP_FCT_64(v))

#define TO_LOCAL_ENDIAN8(v) (v)
#define TO_LOCAL_ENDIAN16(v) (IS_LITTLEENDIAN ? (v) : SWAP_FCT_16(v))
#define TO_LOCAL_ENDIAN32(v) (IS_LITTLEENDIAN ? (v) : SWAP_FCT_32(v))
#define TO_LOCAL_ENDIAN64(v) (IS_LITTLEENDIAN ? (v) : SWAP_FCT_64(v))

#endif
