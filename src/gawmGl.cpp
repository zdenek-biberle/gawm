#include "gawmGl.hpp"
#include <iostream>

PFNGLXBINDTEXIMAGEEXTPROC glXBindTexImageEXT;
PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT;

void initGlFunctions()
{
    glXBindTexImageEXT = reinterpret_cast<PFNGLXBINDTEXIMAGEEXTPROC>(glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXBindTexImageEXT")));
    glXReleaseTexImageEXT = reinterpret_cast<PFNGLXRELEASETEXIMAGEEXTPROC>(glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXReleaseTexImageEXT")));
}

void displayGlErrors()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "GL chyba: " << err << std::endl;
    }
}
