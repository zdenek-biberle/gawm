#ifndef GAWM_GL_HPP
#define GAWM_GL_HPP

#include <GL/gl.h>
#include <GL/glx.h>

typedef void ( * PFNGLXBINDTEXIMAGEEXTPROC) (Display* display, GLXDrawable drawable, int buffer, const int *attrib_list);
typedef void ( * PFNGLXRELEASETEXIMAGEEXTPROC) (Display* display, GLXDrawable drawable, int buffer);

extern PFNGLXBINDTEXIMAGEEXTPROC glXBindTexImageEXT;
extern PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT;

void initGlFunctions();

#endif
