/*
 * Copyright 2016 GoPro Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdio.h>
#include <EGL/egl.h>

#include "glcontext.h"
#include "nodegl.h"

struct glcontext_egl {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext handle;
    EGLConfig config;
};

static int glcontext_egl_init(struct glcontext *glcontext, void *display, void *window, void *handle)
{
    struct glcontext_egl *glcontext_egl = glcontext->priv_data;

    glcontext_egl->display = display ? *(EGLDisplay *)display : eglGetCurrentDisplay();
    glcontext_egl->surface = window  ? *(EGLSurface *)window  : eglGetCurrentSurface(EGL_DRAW);
    glcontext_egl->handle  = handle  ? *(EGLContext *)handle  : eglGetCurrentContext();

    if (!glcontext_egl->display || !glcontext_egl->surface || !glcontext_egl->handle)
        return -1;

    return 0;
}

static void glcontext_egl_uninit(struct glcontext *glcontext)
{
    struct glcontext_egl *glcontext_egl = glcontext->priv_data;

    if (!glcontext->wrapped) {

        eglDestroySurface(glcontext_egl->display, glcontext_egl->surface);
        eglDestroyContext(glcontext_egl->display, glcontext_egl->handle);
    }
}

static int glcontext_egl_create(struct glcontext *glcontext, struct glcontext *other)
{
    int ret;
    EGLint error;
    struct glcontext_egl *glcontext_egl = glcontext->priv_data;
    struct glcontext_egl *other_egl = other->priv_data;

    const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint width;
    EGLint height;

    if (!eglQuerySurface(other_egl->display, other_egl->surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(other_egl->display, other_egl->surface, EGL_HEIGHT, &height)) {
        return -1;
    }

    const EGLint surface_attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };

    EGLint egl_minor;
    EGLint egl_major;
    ret = eglInitialize (glcontext_egl->display, &egl_major, &egl_minor);
    if (!ret) {
        return -1;
    }

    EGLContext config;
    EGLint nb_configs;

    ret = eglChooseConfig(glcontext_egl->display, config_attribs, &config, 1, &nb_configs);
    if (!ret || !nb_configs) {
        return -1;
    }

    glcontext_egl->handle = eglCreateContext(glcontext_egl->display, config, other_egl->handle, ctx_attribs);
    if ((error = eglGetError()) != EGL_SUCCESS){
        return -1;
    }

    glcontext_egl->surface = eglCreatePbufferSurface(glcontext_egl->display, config, surface_attribs);
    if ((error = eglGetError()) != EGL_SUCCESS){
        return -1;
    }

    return 0;
}

static int glcontext_egl_make_current(struct glcontext *glcontext, int current)
{
    int ret;
    struct glcontext_egl *glcontext_egl = glcontext->priv_data;

    if (current) {
        ret = eglMakeCurrent(glcontext_egl->display, glcontext_egl->surface, glcontext_egl->surface, glcontext_egl->handle);
    } else {
        ret = eglMakeCurrent(glcontext_egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, glcontext_egl->handle);
    }

    return ret - 1;
}

static void *glcontext_egl_get_display(struct glcontext *glcontext)
{
    struct glcontext_egl *glcontext_egl = glcontext->priv_data;
    return &glcontext_egl->display;
}

static void *glcontext_egl_get_window(struct glcontext *glcontext)
{
    struct glcontext_egl *glcontext_egl = glcontext->priv_data;
    return &glcontext_egl->surface;
}

static void *glcontext_egl_get_handle(struct glcontext *glcontext)
{
    struct glcontext_egl *glcontext_egl = glcontext->priv_data;
    return &glcontext_egl->handle;
}

static void *glcontext_egl_get_proc_address(struct glcontext *glcontext, const char *name)
{
    return eglGetProcAddress(name);
}

const struct glcontext_class ngli_glcontext_egl_class = {
    .init = glcontext_egl_init,
    .uninit = glcontext_egl_uninit,
    .create = glcontext_egl_create,
    .make_current = glcontext_egl_make_current,
    .get_display = glcontext_egl_get_display,
    .get_window = glcontext_egl_get_window,
    .get_handle = glcontext_egl_get_handle,
    .get_proc_address = glcontext_egl_get_proc_address,
    .priv_size = sizeof(struct glcontext_egl),
};
