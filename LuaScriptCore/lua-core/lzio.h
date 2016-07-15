/*
** $Id: lzio.h,v 1.30 2014/12/19 17:26:14 roberto Exp $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#include "lua.h"

#include "lmem.h"


#define EOZ	(-1)			/* end of stream */

typedef struct NameDef(Zio) NameDef(ZIO);

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : NameDef(luaZ_fill)(z))


typedef struct NameDef(Mbuffer) {
  char *buffer;
  size_t n;
  size_t buffsize;
} NameDef(Mbuffer);

#define luaZ_initbuffer(L, buff) ((buff)->buffer = NULL, (buff)->buffsize = 0)

#define luaZ_buffer(buff)	((buff)->buffer)
#define luaZ_sizebuffer(buff)	((buff)->buffsize)
#define luaZ_bufflen(buff)	((buff)->n)

#define luaZ_buffremove(buff,i)	((buff)->n -= (i))
#define luaZ_resetbuffer(buff) ((buff)->n = 0)


#define luaZ_resizebuffer(L, buff, size) \
	((buff)->buffer = luaM_reallocvchar(L, (buff)->buffer, \
				(buff)->buffsize, size), \
	(buff)->buffsize = size)

#define luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC char *NameDef(luaZ_openspace) (NameDef(lua_State) *L, NameDef(Mbuffer) *buff, size_t n);
LUAI_FUNC void NameDef(luaZ_init) (NameDef(lua_State) *L, NameDef(ZIO) *z, NameDef(lua_Reader) reader,
                                        void *data);
LUAI_FUNC size_t NameDef(luaZ_read) (NameDef(ZIO)* z, void *b, size_t n);	/* read next n bytes */



/* --------- Private Part ------------------ */

struct NameDef(Zio) {
  size_t n;			/* bytes still unread */
  const char *p;		/* current position in buffer */
  NameDef(lua_Reader) reader;		/* reader function */
  void *data;			/* additional data */
  NameDef(lua_State) *L;			/* Lua state (for reader) */
};


LUAI_FUNC int NameDef(luaZ_fill) (NameDef(ZIO) *z);

#endif
