#include "gawmGl.hpp"

PFNGLXBINDTEXIMAGEEXTPROC glXBindTexImageEXT;
PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT;

void initGlFunctions()
{
    glXBindTexImageEXT = reinterpret_cast<PFNGLXBINDTEXIMAGEEXTPROC>(glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXBindTexImageEXT")));
    glXReleaseTexImageEXT = reinterpret_cast<PFNGLXRELEASETEXIMAGEEXTPROC>(glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXReleaseTexImageEXT")));
}
