from libc.stdlib cimport calloc

cdef extern from "stdarg.h":
    ctypedef struct va_list:
        pass

cdef extern from "stdio.h":
    int vsnprintf(char *str, size_t size, const char *format, va_list ap)

cdef extern from "nodegl.h":
    cdef int NGL_LOG_VERBOSE
    cdef int NGL_LOG_DEBUG
    cdef int NGL_LOG_INFO
    cdef int NGL_LOG_WARNING
    cdef int NGL_LOG_ERROR

    ctypedef void (*ngl_log_callback_type)(void *arg, int level, const char *filename,
                                           int ln, const char *fn, const char *fmt, va_list vl)
    void ngl_log_set_callback(void *arg, ngl_log_callback_type callback)
    void ngl_log_set_min_level(int level)

    cdef struct ngl_node

    ngl_node *ngl_node_create(int type, ...)
    void ngl_node_ref(ngl_node *node)
    void ngl_node_unrefp(ngl_node **op)

    int ngl_node_param_add(ngl_node *node, const char *key,
                           int nb_elems, void *elems)
    int ngl_node_param_set(ngl_node *node, const char *key, ...)
    char *ngl_node_dot(const ngl_node *node)

    cdef int NGL_GLPLATFORM_GLX
    cdef int NGL_GLPLATFORM_CGL
    cdef int NGL_GLAPI_OPENGL3

    cdef struct ngl_ctx

    ngl_ctx *ngl_create()
    int ngl_set_glcontext(ngl_ctx *s, void *display, void *window, void *handle, int platform, int api)
    int ngl_draw(ngl_ctx *s, ngl_node *scene, double t)
    int ngl_set_viewport(ngl_ctx *s, int x, int y, int w, int h)
    void ngl_free(ngl_ctx **ss)

GLPLATFORM_GLX = NGL_GLPLATFORM_GLX
GLPLATFORM_CGL = NGL_GLPLATFORM_CGL
GLAPI_OPENGL3 = NGL_GLAPI_OPENGL3

LOG_VERBOSE = NGL_LOG_VERBOSE
LOG_DEBUG   = NGL_LOG_DEBUG
LOG_INFO    = NGL_LOG_INFO
LOG_WARNING = NGL_LOG_WARNING
LOG_ERROR   = NGL_LOG_ERROR

include "nodes_def.pyx"

def log_set_min_level(int level):
    ngl_log_set_min_level(level)

_callback_func = None

cdef void _log_callback_wrapper(void *arg, int level, const char *filename,
                                int ln, const char *fn, const char *fmt, va_list vl):
    global _callback_func
    cdef char[512] buf
    vsnprintf(buf, 512, fmt, vl);
    _callback_func(<object>arg, level, filename, ln, fn, buf)

def log_set_callback(arg, f):
    global _callback_func
    _callback_func = f
    ngl_log_set_callback(<void *>arg, _log_callback_wrapper)

cdef class Viewer:
    cdef ngl_ctx *ctx

    def __cinit__(self):
        self.ctx = ngl_create()
        if self.ctx is NULL:
            raise MemoryError()

    def set_window(self, int platform, int api):
        return ngl_set_glcontext(self.ctx, NULL, NULL, NULL, platform, api);

    def draw(self, _Node scene, double t):
        return ngl_draw(self.ctx, scene.ctx, t)

    def set_viewport(self, int x, int y, int w, int h):
        return ngl_set_viewport(self.ctx, x, y, w, h)

    def __dealloc__(self):
        ngl_free(&self.ctx)
