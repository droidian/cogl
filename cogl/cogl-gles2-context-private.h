/*
 * Cogl
 *
 * An object oriented GL/GLES Abstraction/Utility Layer
 *
 * Copyright (C) 2011 Collabora Ltd.
 * Copyright (C) 2012 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Tomeu Vizoso <tomeu.vizoso@collabora.com>
 *  Robert Bragg <robert@linux.intel.com>
 *
 */

#ifndef __COGL_GLES2_CONTEXT_PRIVATE_H
#define __COGL_GLES2_CONTEXT_PRIVATE_H

#include <glib.h>

#include "cogl-object-private.h"
#include "cogl-framebuffer-private.h"
#include "cogl-queue.h"

typedef struct _CoglGLES2Offscreen CoglGLES2Offscreen;

COGL_LIST_HEAD (CoglGLES2OffscreenList, CoglGLES2Offscreen);

struct _CoglGLES2Offscreen
{
  COGL_LIST_ENTRY (CoglGLES2Offscreen) list_node;
  CoglOffscreen *original_offscreen;
  CoglGLFramebuffer gl_framebuffer;
};

typedef struct
{
  /* GL's ID for the shader */
  GLuint object_id;
  /* Shader type */
  GLenum type;

  /* Number of references to this shader. The shader will have one
   * reference when it is created. This reference will be removed when
   * glDeleteShader is called. An additional reference will be taken
   * whenever the shader is attached to a program. This is necessary
   * to correctly detect when a shader is destroyed because
   * glDeleteShader doesn't actually delete the object if it is
   * attached to a program */
  int ref_count;

  /* Set once this object has had glDeleteShader called on it. We need
   * to keep track of this so we don't deref the data twice if the
   * application calls glDeleteShader multiple times */
  CoglBool deleted;
} CoglGLES2ShaderData;

typedef struct
{
  /* GL's ID for the program */
  GLuint object_id;

  /* List of shaders attached to this program */
  GList *attached_shaders;

  /* Reference count. There can be up to two references. One of these
   * will exist between glCreateProgram and glDeleteShader, the other
   * will exist while the program is made current. This is necessary
   * to correctly detect when the program is deleted because
   * glDeleteShader will delay the deletion if the program is
   * current */
  int ref_count;

  /* Set once this object has had glDeleteProgram called on it. We need
   * to keep track of this so we don't deref the data twice if the
   * application calls glDeleteProgram multiple times */
  CoglBool deleted;

  CoglGLES2Context *context;
} CoglGLES2ProgramData;

struct _CoglGLES2Context
{
  CoglObject _parent;

  CoglContext *context;

  /* This is set to FALSE until the first time the GLES2 context is
   * bound to something. We need to keep track of this so we can set
   * the viewport and scissor the first time it is bound. */
  CoglBool has_been_bound;

  CoglFramebuffer *read_buffer;
  CoglGLES2Offscreen *gles2_read_buffer;
  CoglFramebuffer *write_buffer;
  CoglGLES2Offscreen *gles2_write_buffer;

  GLuint current_fbo_handle;

  CoglGLES2OffscreenList foreign_offscreens;

  CoglGLES2Vtable *vtable;

  /* Hash table mapping GL's IDs for shaders and objects to ShaderData
   * and ProgramData so that we can maintain extra data for these
   * objects. Although technically the IDs will end up global across
   * all GLES2 contexts because they will all be in the same share
   * list, we don't really want to expose this outside of the Cogl API
   * so we will assume it is undefined behaviour if an application
   * relies on this. */
  GHashTable *shader_map;
  GHashTable *program_map;

  /* Currently in use program. We need to keep track of this so that
   * we can keep a reference to the data for the program while it is
   * current */
  CoglGLES2ProgramData *current_program;

  void *winsys;
};

#endif /* __COGL_GLES2_CONTEXT_PRIVATE_H */
