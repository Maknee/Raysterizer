#include "opengl_hooks.h"

#undef glAccum
#undef glAlphaFunc
#undef glAreTexturesResident
#undef glArrayElement
#undef glBegin
#undef glBindTexture
#undef glBitmap
#undef glBlendFunc
#undef glCallList
#undef glCallLists
#undef glClear
#undef glClearAccum
#undef glClearColor
#undef glClearDepth
#undef glClearIndex
#undef glClearStencil
#undef glClipPlane
#undef glColor3b
#undef glColor3bv
#undef glColor3d
#undef glColor3dv
#undef glColor3f
#undef glColor3fv
#undef glColor3i
#undef glColor3iv
#undef glColor3s
#undef glColor3sv
#undef glColor3ub
#undef glColor3ubv
#undef glColor3ui
#undef glColor3uiv
#undef glColor3us
#undef glColor3usv
#undef glColor4b
#undef glColor4bv
#undef glColor4d
#undef glColor4dv
#undef glColor4f
#undef glColor4fv
#undef glColor4i
#undef glColor4iv
#undef glColor4s
#undef glColor4sv
#undef glColor4ub
#undef glColor4ubv
#undef glColor4ui
#undef glColor4uiv
#undef glColor4us
#undef glColor4usv
#undef glColorMask
#undef glColorMaterial
#undef glColorPointer
#undef glCopyPixels
#undef glCopyTexImage1D
#undef glCopyTexImage2D
#undef glCopyTexSubImage1D
#undef glCopyTexSubImage2D
#undef glCullFace
#undef glDeleteLists
#undef glDeleteRenderbuffers
#undef glDeleteTextures
#undef glDepthFunc
#undef glDepthMask
#undef glDepthRange
#undef glDisable
#undef glDisableClientState
#undef glDrawArrays
#undef glDrawArraysIndirect
#undef glDrawArraysInstanced
#undef glDrawArraysInstancedARB
#undef glDrawArraysInstancedEXT
#undef glDrawArraysInstancedBaseInstance
#undef glDrawBuffer
#undef glDrawElements
#undef glDrawElementsBaseVertex
#undef glDrawElementsIndirect
#undef glDrawElementsInstanced
#undef glDrawElementsInstancedARB
#undef glDrawElementsInstancedEXT
#undef glDrawElementsInstancedBaseVertex
#undef glDrawElementsInstancedBaseInstance
#undef glDrawElementsInstancedBaseVertexBaseInstance
#undef glDrawPixels
#undef glDrawRangeElements
#undef glDrawRangeElementsBaseVertex
#undef glEdgeFlag
#undef glEdgeFlagPointer
#undef glEdgeFlagv
#undef glEnable
#undef glEnableClientState
#undef glEnd
#undef glEndList
#undef glEvalCoord1d
#undef glEvalCoord1dv
#undef glEvalCoord1f
#undef glEvalCoord1fv
#undef glEvalCoord2d
#undef glEvalCoord2dv
#undef glEvalCoord2f
#undef glEvalCoord2fv
#undef glEvalMesh1
#undef glEvalMesh2
#undef glEvalPoint1
#undef glEvalPoint2
#undef glFeedbackBuffer
#undef glFinish
#undef glFlush
#undef glFogf
#undef glFogfv
#undef glFogi
#undef glFogiv
#undef glFramebufferRenderbuffer
#undef glFramebufferRenderbufferEXT
#undef glFramebufferTexture
#undef glFramebufferTextureARB
#undef glFramebufferTextureEXT
#undef glFramebufferTexture1D
#undef glFramebufferTexture1DEXT
#undef glFramebufferTexture2D
#undef glFramebufferTexture2DEXT
#undef glFramebufferTexture3D
#undef glFramebufferTexture3DEXT
#undef glFramebufferTextureLayer
#undef glFramebufferTextureLayerARB
#undef glFramebufferTextureLayerEXT
#undef glFrontFace
#undef glFrustum
#undef glGenLists
#undef glGenTextures
#undef glGetBooleanv
#undef glGetDoublev
#undef glGetFloatv
#undef glGetIntegerv
#undef glGetClipPlane
#undef glGetError
#undef glGetLightfv
#undef glGetLightiv
#undef glGetMapdv
#undef glGetMapfv
#undef glGetMapiv
#undef glGetMaterialfv
#undef glGetMaterialiv
#undef glGetPixelMapfv
#undef glGetPixelMapuiv
#undef glGetPixelMapusv
#undef glGetPointerv
#undef glGetPolygonStipple
#undef glGetString
#undef glGetTexEnvfv
#undef glGetTexEnviv
#undef glGetTexGendv
#undef glGetTexGenfv
#undef glGetTexGeniv
#undef glGetTexImage
#undef glGetTexLevelParameterfv
#undef glGetTexLevelParameteriv
#undef glGetTexParameterfv
#undef glGetTexParameteriv
#undef glHint
#undef glIndexMask
#undef glIndexPointer
#undef glIndexd
#undef glIndexdv
#undef glIndexf
#undef glIndexfv
#undef glIndexi
#undef glIndexiv
#undef glIndexs
#undef glIndexsv
#undef glIndexub
#undef glIndexubv
#undef glInitNames
#undef glInterleavedArrays
#undef glIsEnabled
#undef glIsList
#undef glIsTexture
#undef glLightModelf
#undef glLightModelfv
#undef glLightModeli
#undef glLightModeliv
#undef glLightf
#undef glLightfv
#undef glLighti
#undef glLightiv
#undef glLineStipple
#undef glLineWidth
#undef glListBase
#undef glLoadIdentity
#undef glLoadMatrixd
#undef glLoadMatrixf
#undef glLoadName
#undef glLogicOp
#undef glMapBuffer
#undef glMap1d
#undef glMap1f
#undef glMap2d
#undef glMap2f
#undef glMapGrid1d
#undef glMapGrid1f
#undef glMapGrid2d
#undef glMapGrid2f
#undef glMaterialf
#undef glMaterialfv
#undef glMateriali
#undef glMaterialiv
#undef glMatrixMode
#undef glMultMatrixd
#undef glMultMatrixf
#undef glMultiDrawArrays
#undef glMultiDrawArraysIndirect
#undef glMultiDrawElements
#undef glMultiDrawElementsBaseVertex
#undef glMultiDrawElementsIndirect
#undef glNewList
#undef glNormal3b
#undef glNormal3bv
#undef glNormal3d
#undef glNormal3dv
#undef glNormal3f
#undef glNormal3fv
#undef glNormal3i
#undef glNormal3iv
#undef glNormal3s
#undef glNormal3sv
#undef glNormalPointer
#undef glOrtho
#undef glPassThrough
#undef glPixelMapfv
#undef glPixelMapuiv
#undef glPixelMapusv
#undef glPixelStoref
#undef glPixelStorei
#undef glPixelTransferf
#undef glPixelTransferi
#undef glPixelZoom
#undef glPointSize
#undef glPolygonMode
#undef glPolygonOffset
#undef glPolygonStipple
#undef glPopAttrib
#undef glPopClientAttrib
#undef glPopMatrix
#undef glPopName
#undef glPrioritizeTextures
#undef glPushAttrib
#undef glPushClientAttrib
#undef glPushMatrix
#undef glPushName
#undef glRasterPos2d
#undef glRasterPos2dv
#undef glRasterPos2f
#undef glRasterPos2fv
#undef glRasterPos2i
#undef glRasterPos2iv
#undef glRasterPos2s
#undef glRasterPos2sv
#undef glRasterPos3d
#undef glRasterPos3dv
#undef glRasterPos3f
#undef glRasterPos3fv
#undef glRasterPos3i
#undef glRasterPos3iv
#undef glRasterPos3s
#undef glRasterPos3sv
#undef glRasterPos4d
#undef glRasterPos4dv
#undef glRasterPos4f
#undef glRasterPos4fv
#undef glRasterPos4i
#undef glRasterPos4iv
#undef glRasterPos4s
#undef glRasterPos4sv
#undef glReadBuffer
#undef glReadPixels
#undef glRectd
#undef glRectdv
#undef glRectf
#undef glRectfv
#undef glRecti
#undef glRectiv
#undef glRects
#undef glRectsv
#undef glRenderMode
#undef glRotated
#undef glRotatef
#undef glScaled
#undef glScalef
#undef glScissor
#undef glSelectBuffer
#undef glShadeModel
#undef glStencilFunc
#undef glStencilMask
#undef glStencilOp
#undef glTexCoord1d
#undef glTexCoord1dv
#undef glTexCoord1f
#undef glTexCoord1fv
#undef glTexCoord1i
#undef glTexCoord1iv
#undef glTexCoord1s
#undef glTexCoord1sv
#undef glTexCoord2d
#undef glTexCoord2dv
#undef glTexCoord2f
#undef glTexCoord2fv
#undef glTexCoord2i
#undef glTexCoord2iv
#undef glTexCoord2s
#undef glTexCoord2sv
#undef glTexCoord3d
#undef glTexCoord3dv
#undef glTexCoord3f
#undef glTexCoord3fv
#undef glTexCoord3i
#undef glTexCoord3iv
#undef glTexCoord3s
#undef glTexCoord3sv
#undef glTexCoord4d
#undef glTexCoord4dv
#undef glTexCoord4f
#undef glTexCoord4fv
#undef glTexCoord4i
#undef glTexCoord4iv
#undef glTexCoord4s
#undef glTexCoord4sv
#undef glTexCoordPointer
#undef glTexEnvf
#undef glTexEnvfv
#undef glTexEnvi
#undef glTexEnviv
#undef glTexGend
#undef glTexGendv
#undef glTexGenf
#undef glTexGenfv
#undef glTexGeni
#undef glTexGeniv
#undef glTexImage1D
#undef glTexImage2D
#undef glTexImage3D
#undef glTexParameterf
#undef glTexParameterfv
#undef glTexParameteri
#undef glTexParameteriv
#undef glTexSubImage1D
#undef glTexSubImage2D
#undef glTranslated
#undef glTranslatef
#undef glVertex2d
#undef glVertex2dv
#undef glVertex2f
#undef glVertex2fv
#undef glVertex2i
#undef glVertex2iv
#undef glVertex2s
#undef glVertex2sv
#undef glVertex3d
#undef glVertex3dv
#undef glVertex3f
#undef glVertex3fv
#undef glVertex3i
#undef glVertex3iv
#undef glVertex3s
#undef glVertex3sv
#undef glVertex4d
#undef glVertex4dv
#undef glVertex4f
#undef glVertex4fv
#undef glVertex4i
#undef glVertex4iv
#undef glVertex4s
#undef glVertex4sv
#undef glVertexPointer
#undef glViewport

HOOK_EXPORT void HOOK_CALL_CONVENTION glAccum(GLenum op, GLfloat value)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", op, value);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glAccum);
	redirect_function(op, value);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glAlphaFunc(GLenum func, GLclampf ref)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", func, ref);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glAlphaFunc);
	redirect_function(func, ref);
}

HOOK_EXPORT GLboolean HOOK_CALL_CONVENTION glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", n, textures, residences);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glAreTexturesResident);
	auto result = redirect_function(n, textures, residences);
	return result;
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glArrayElement(GLint i)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", i);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glArrayElement);
	redirect_function(i);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glBegin(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glBegin);
	redirect_function(mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glBindTexture(GLenum target, GLuint texture)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", target, texture);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glBindTexture);
	redirect_function(target, texture);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {}", width, height, xorig, yorig, xmove, ymove, bitmap);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glBitmap);
	redirect_function(width, height, xorig, yorig, xmove, ymove, bitmap);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", sfactor, dfactor);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glBlendFunc);
	redirect_function(sfactor, dfactor);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glCallList(GLuint list)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", list);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCallList);
	redirect_function(list);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glCallLists(GLsizei n, GLenum type, const GLvoid* lists)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", n, type, lists);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCallLists);
	redirect_function(n, type, lists);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glClear(GLbitfield mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glClear);
	redirect_function(mask);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glClearAccum);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glClearColor);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glClearDepth(GLclampd depth)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", depth);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glClearDepth);
	redirect_function(depth);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glClearIndex(GLfloat c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glClearIndex);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glClearStencil(GLint s)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", s);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glClearStencil);
	redirect_function(s);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glClipPlane(GLenum plane, const GLdouble* equation)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", plane, equation);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glClipPlane);
	redirect_function(plane, equation);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3b);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3bv(const GLbyte* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3bv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3d);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3f);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3i(GLint red, GLint green, GLint blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3i);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3s(GLshort red, GLshort green, GLshort blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3s);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3ub);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3ubv(const GLubyte* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3ubv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3ui(GLuint red, GLuint green, GLuint blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3ui);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3uiv(const GLuint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3uiv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3us(GLushort red, GLushort green, GLushort blue)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", red, green, blue);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3us);
	redirect_function(red, green, blue);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor3usv(const GLushort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor3usv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4b);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4bv(const GLbyte* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4bv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4d);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4f);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4i);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4s);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4ub);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4ubv(const GLubyte* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4ubv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4ui);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4uiv(const GLuint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4uiv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4us);
	redirect_function(red, green, blue, alpha);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glColor4usv(const GLushort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColor4usv);
	redirect_function(v);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", red, green, blue, alpha);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColorMask);
	redirect_function(red, green, blue, alpha);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glColorMaterial(GLenum face, GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", face, mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColorMaterial);
	redirect_function(face, mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", size, type, stride, pointer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glColorPointer);
	redirect_function(size, type, stride, pointer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", x, y, width, height, type);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCopyPixels);
	redirect_function(x, y, width, height, type);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {}", target, level, internalFormat, x, y, width, border);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCopyTexImage1D);
	redirect_function(target, level, internalFormat, x, y, width, border);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {} {}", target, level, internalFormat, x, y, width, height, border);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCopyTexImage2D);
	redirect_function(target, level, internalFormat, x, y, width, height, border);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", target, level, xoffset, x, y, width);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCopyTexSubImage1D);
	redirect_function(target, level, xoffset, x, y, width);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {} {}", target, level, xoffset, yoffset, x, y, width, height);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCopyTexSubImage2D);
	redirect_function(target, level, xoffset, yoffset, x, y, width, height);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glCullFace(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glCullFace);
	redirect_function(mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDeleteLists(GLuint list, GLsizei range)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", list, range);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDeleteLists);
	redirect_function(list, range);
}

void HOOK_CALL_CONVENTION glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", n, renderbuffers);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDeleteTextures(GLsizei n, const GLuint* textures)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", n, textures);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDeleteTextures);
	redirect_function(n, textures);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDepthFunc(GLenum func)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", func);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDepthFunc);
	redirect_function(func);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDepthMask(GLboolean flag)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", flag);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDepthMask);
	redirect_function(flag);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDepthRange(GLclampd zNear, GLclampd zFar)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", zNear, zFar);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDepthRange);
	redirect_function(zNear, zFar);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDisable(GLenum cap)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", cap);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDisable);
	redirect_function(cap);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glDisableClientState(GLenum array)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", array);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDisableClientState);
	redirect_function(array);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", mode, first, count);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDrawArrays);
	redirect_function(mode, first, count);
}
void HOOK_CALL_CONVENTION glDrawArraysIndirect(GLenum mode, const GLvoid* indirect)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", mode, indirect);
}
void HOOK_CALL_CONVENTION glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", mode, first, count, primcount);
}
void HOOK_CALL_CONVENTION glDrawArraysInstancedARB(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", mode, first, count, primcount);
}
void HOOK_CALL_CONVENTION glDrawArraysInstancedEXT(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", mode, first, count, primcount);
}
void HOOK_CALL_CONVENTION glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", mode, first, count, primcount, baseinstance);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDrawBuffer(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDrawBuffer);
	redirect_function(mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", mode, count, type, indices);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDrawElements);
	redirect_function(mode, count, type, indices);
}
void HOOK_CALL_CONVENTION glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", mode, count, type, indices, basevertex);
}
void HOOK_CALL_CONVENTION glDrawElementsIndirect(GLenum mode, GLenum type, const GLvoid* indirect)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", mode, type, indirect);
}
void HOOK_CALL_CONVENTION glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", mode, count, type, indices, primcount);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDrawElementsInstanced);
	redirect_function(mode, count, type, indices, primcount);
}
void HOOK_CALL_CONVENTION glDrawElementsInstancedARB(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", mode, count, type, indices, primcount);
}
void HOOK_CALL_CONVENTION glDrawElementsInstancedEXT(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", mode, count, type, indices, primcount);
}
void HOOK_CALL_CONVENTION glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount, GLint basevertex)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", mode, count, type, indices, primcount, basevertex);
}
void HOOK_CALL_CONVENTION glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount, GLuint baseinstance)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", mode, count, type, indices, primcount, baseinstance);
}
void HOOK_CALL_CONVENTION glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount, GLint basevertex, GLuint baseinstance)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {}", mode, count, type, indices, primcount, basevertex, baseinstance);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", width, height, format, type, pixels);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glDrawPixels);
	redirect_function(width, height, format, type, pixels);
}

void HOOK_CALL_CONVENTION glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", mode, start, end, count, type, indices);
}
void HOOK_CALL_CONVENTION glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices, GLint basevertex)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {}", mode, start, end, count, type, indices, basevertex);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glEdgeFlag(GLboolean flag)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", flag);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEdgeFlag);
	redirect_function(flag);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEdgeFlagPointer(GLsizei stride, const GLvoid* pointer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", stride, pointer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEdgeFlagPointer);
	redirect_function(stride, pointer);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEdgeFlagv(const GLboolean* flag)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", flag);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEdgeFlagv);
	redirect_function(flag);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glEnable(GLenum cap)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", cap);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEnable);
	redirect_function(cap);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEnableClientState(GLenum array)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", array);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEnableClientState);
	redirect_function(array);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glEnd()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEnd);
	redirect_function();
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glEndList()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEndList);
	redirect_function();
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord1d(GLdouble u)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", u);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord1d);
	redirect_function(u);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord1dv(const GLdouble* u)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", u);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord1dv);
	redirect_function(u);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord1f(GLfloat u)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", u);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord1f);
	redirect_function(u);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord1fv(const GLfloat* u)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", u);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord1fv);
	redirect_function(u);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord2d(GLdouble u, GLdouble v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", u, v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord2d);
	redirect_function(u, v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord2dv(const GLdouble* u)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", u);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord2dv);
	redirect_function(u);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord2f(GLfloat u, GLfloat v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", u, v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord2f);
	redirect_function(u, v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalCoord2fv(const GLfloat* u)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", u);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalCoord2fv);
	redirect_function(u);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", mode, i1, i2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalMesh1);
	redirect_function(mode, i1, i2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", mode, i1, i2, j1, j2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalMesh2);
	redirect_function(mode, i1, i2, j1, j2);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalPoint1(GLint i)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", i);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalPoint1);
	redirect_function(i);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glEvalPoint2(GLint i, GLint j)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", i, j);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glEvalPoint2);
	redirect_function(i, j);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", size, type, buffer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFeedbackBuffer);
	redirect_function(size, type, buffer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glFinish()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFinish);
	redirect_function();
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glFlush()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFlush);
	redirect_function();
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glFogf(GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFogf);
	redirect_function(pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glFogfv(GLenum pname, const GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFogfv);
	redirect_function(pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glFogi(GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFogi);
	redirect_function(pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glFogiv(GLenum pname, const GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFogiv);
	redirect_function(pname, params);
}

void HOOK_CALL_CONVENTION glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", target, attachment, renderbuffertarget, renderbuffer);
}
void HOOK_CALL_CONVENTION glFramebufferRenderbufferEXT(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", target, attachment, renderbuffertarget, renderbuffer);
}
void HOOK_CALL_CONVENTION glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", target, attachment, texture, level);
}
void HOOK_CALL_CONVENTION glFramebufferTextureARB(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", target, attachment, texture, level);
}

void HOOK_CALL_CONVENTION glFramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", target, attachment, texture, level);
}

void HOOK_CALL_CONVENTION glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, attachment, textarget, texture, level);
}

void HOOK_CALL_CONVENTION glFramebufferTexture1DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, attachment, textarget, texture, level);
}

void HOOK_CALL_CONVENTION glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, attachment, textarget, texture, level);
}

void HOOK_CALL_CONVENTION glFramebufferTexture2DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, attachment, textarget, texture, level);
}

void HOOK_CALL_CONVENTION glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", target, attachment, textarget, texture, level, zoffset);
}
void HOOK_CALL_CONVENTION glFramebufferTexture3DEXT(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", target, attachment, textarget, texture, level, zoffset);
}
void HOOK_CALL_CONVENTION glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, attachment, texture, level, layer);
}
void HOOK_CALL_CONVENTION glFramebufferTextureLayerARB(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, attachment, texture, level, layer);
}
void HOOK_CALL_CONVENTION glFramebufferTextureLayerEXT(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, attachment, texture, level, layer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glFrontFace(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFrontFace);
	redirect_function(mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", left, right, bottom, top, zNear, zFar);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glFrustum);
	redirect_function(left, right, bottom, top, zNear, zFar);
}

HOOK_EXPORT GLuint HOOK_CALL_CONVENTION glGenLists(GLsizei range)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", range);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGenLists);
	auto result = redirect_function(range);
	return result;
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGenTextures(GLsizei n, GLuint* textures)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", n, textures);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGenTextures);
	redirect_function(n, textures);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetBooleanv(GLenum pname, GLboolean* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetBooleanv);
	redirect_function(pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetDoublev(GLenum pname, GLdouble* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetDoublev);
	redirect_function(pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetFloatv(GLenum pname, GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetFloatv);
	redirect_function(pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetIntegerv(GLenum pname, GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetIntegerv);
	redirect_function(pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetClipPlane(GLenum plane, GLdouble* equation)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", plane, equation);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetClipPlane);
	redirect_function(plane, equation);
}

HOOK_EXPORT GLenum HOOK_CALL_CONVENTION glGetError()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetError);
	auto result = redirect_function();
	return result;
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetLightfv(GLenum light, GLenum pname, GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", light, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetLightfv);
	redirect_function(light, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetLightiv(GLenum light, GLenum pname, GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", light, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetLightiv);
	redirect_function(light, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetMapdv(GLenum target, GLenum query, GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, query, v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetMapdv);
	redirect_function(target, query, v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetMapfv(GLenum target, GLenum query, GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, query, v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetMapfv);
	redirect_function(target, query, v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetMapiv(GLenum target, GLenum query, GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, query, v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetMapiv);
	redirect_function(target, query, v);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", face, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetMaterialfv);
	redirect_function(face, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetMaterialiv(GLenum face, GLenum pname, GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", face, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetMaterialiv);
	redirect_function(face, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetPixelMapfv(GLenum map, GLfloat* values)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", map, values);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetPixelMapfv);
	redirect_function(map, values);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetPixelMapuiv(GLenum map, GLuint* values)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", map, values);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetPixelMapuiv);
	redirect_function(map, values);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetPixelMapusv(GLenum map, GLushort* values)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", map, values);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetPixelMapusv);
	redirect_function(map, values);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetPointerv(GLenum pname, GLvoid** params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetPointerv);
	redirect_function(pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetPolygonStipple(GLubyte* mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetPolygonStipple);
	redirect_function(mask);
}

HOOK_EXPORT const GLubyte* HOOK_CALL_CONVENTION glGetString(GLenum name)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", name);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetString);
	auto result = redirect_function(name);
	return result;
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexEnvfv);
	redirect_function(target, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexEnviv(GLenum target, GLenum pname, GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexEnviv);
	redirect_function(target, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexGendv);
	redirect_function(coord, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", coord, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexGenfv);
	redirect_function(coord, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexGeniv(GLenum coord, GLenum pname, GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexGeniv);
	redirect_function(coord, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", target, level, format, type, pixels);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexImage);
	redirect_function(target, level, format, type, pixels);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", target, level, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexLevelParameterfv);
	redirect_function(target, level, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", target, level, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexLevelParameteriv);
	redirect_function(target, level, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexParameterfv);
	redirect_function(target, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glGetTexParameteriv);
	redirect_function(target, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glHint(GLenum target, GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", target, mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glHint);
	redirect_function(target, mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexMask(GLuint mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexMask);
	redirect_function(mask);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", type, stride, pointer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexPointer);
	redirect_function(type, stride, pointer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexd(GLdouble c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexd);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexdv(const GLdouble* c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexdv);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexf(GLfloat c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexf);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexfv(const GLfloat* c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexfv);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexi(GLint c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexi);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexiv(const GLint* c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexiv);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexs(GLshort c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexs);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexsv(const GLshort* c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexsv);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexub(GLubyte c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexub);
	redirect_function(c);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glIndexubv(const GLubyte* c)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", c);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIndexubv);
	redirect_function(c);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glInitNames()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glInitNames);
	redirect_function();
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* pointer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", format, stride, pointer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glInterleavedArrays);
	redirect_function(format, stride, pointer);
}

HOOK_EXPORT GLboolean HOOK_CALL_CONVENTION glIsEnabled(GLenum cap)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", cap);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIsEnabled);
	auto result = redirect_function(cap);
	return result;
}

HOOK_EXPORT GLboolean HOOK_CALL_CONVENTION glIsList(GLuint list)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", list);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIsList);
	auto result = redirect_function(list);
	return result;
}

HOOK_EXPORT GLboolean HOOK_CALL_CONVENTION glIsTexture(GLuint texture)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", texture);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glIsTexture);
	auto result = redirect_function(texture);
	return result;
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glLightModelf(GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLightModelf);
	redirect_function(pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLightModelfv(GLenum pname, const GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLightModelfv);
	redirect_function(pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLightModeli(GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLightModeli);
	redirect_function(pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLightModeliv(GLenum pname, const GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLightModeliv);
	redirect_function(pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glLightf(GLenum light, GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", light, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLightf);
	redirect_function(light, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLightfv(GLenum light, GLenum pname, const GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", light, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLightfv);
	redirect_function(light, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLighti(GLenum light, GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", light, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLighti);
	redirect_function(light, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLightiv(GLenum light, GLenum pname, const GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", light, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLightiv);
	redirect_function(light, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glLineStipple(GLint factor, GLushort pattern)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", factor, pattern);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLineStipple);
	redirect_function(factor, pattern);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLineWidth(GLfloat width)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", width);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLineWidth);
	redirect_function(width);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glListBase(GLuint base)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", base);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glListBase);
	redirect_function(base);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glLoadIdentity()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLoadIdentity);
	redirect_function();
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLoadMatrixd(const GLdouble* m)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", m);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLoadMatrixd);
	redirect_function(m);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glLoadMatrixf(const GLfloat* m)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", m);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLoadMatrixf);
	redirect_function(m);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glLoadName(GLuint name)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", name);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLoadName);
	redirect_function(name);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glLogicOp(GLenum opcode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", opcode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glLogicOp);
	redirect_function(opcode);
}

HOOK_EXPORT void* HOOK_CALL_CONVENTION glMapBuffer(GLenum target, GLenum access)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", target, access);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMapBuffer);
	auto result = redirect_function(target, access);
	return result;
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glUnmapBuffer(GLenum target)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", target);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glUnmapBuffer);
	redirect_function(target);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", target, u1, u2, stride, order, points);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMap1d);
	redirect_function(target, u1, u2, stride, order, points);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", target, u1, u2, stride, order, points);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMap1f);
	redirect_function(target, u1, u2, stride, order, points);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {} {} {} {}", target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMap2d);
	redirect_function(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {} {} {} {}", target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMap2f);
	redirect_function(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", un, u1, u2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMapGrid1d);
	redirect_function(un, u1, u2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", un, u1, u2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMapGrid1f);
	redirect_function(un, u1, u2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", un, u1, u2, vn, v1, v2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMapGrid2d);
	redirect_function(un, u1, u2, vn, v1, v2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", un, u1, u2, vn, v1, v2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMapGrid2f);
	redirect_function(un, u1, u2, vn, v1, v2);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", face, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMaterialf);
	redirect_function(face, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMaterialfv(GLenum face, GLenum pname, const GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", face, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMaterialfv);
	redirect_function(face, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMateriali(GLenum face, GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", face, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMateriali);
	redirect_function(face, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMaterialiv(GLenum face, GLenum pname, const GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", face, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMaterialiv);
	redirect_function(face, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glMatrixMode(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMatrixMode);
	redirect_function(mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glMultMatrixd(const GLdouble* m)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", m);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMultMatrixd);
	redirect_function(m);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glMultMatrixf(const GLfloat* m)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", m);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glMultMatrixf);
	redirect_function(m);
}

void HOOK_CALL_CONVENTION glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", mode, first, count, drawcount);
}
void HOOK_CALL_CONVENTION glMultiDrawArraysIndirect(GLenum mode, const void* indirect, GLsizei drawcount, GLsizei stride)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", mode, indirect, drawcount, stride);
}
void HOOK_CALL_CONVENTION glMultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", mode, count, type, indices, drawcount);
}
void HOOK_CALL_CONVENTION glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei* count, GLenum type, const GLvoid* const* indices, GLsizei drawcount, const GLint* basevertex)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", mode, count, type, indices, drawcount, basevertex);
}
void HOOK_CALL_CONVENTION glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {}", mode, type, indirect, drawcount, stride);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glNewList(GLuint list, GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", list, mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNewList);
	redirect_function(list, mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", nx, ny, nz);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3b);
	redirect_function(nx, ny, nz);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3bv(const GLbyte* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3bv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", nx, ny, nz);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3d);
	redirect_function(nx, ny, nz);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", nx, ny, nz);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3f);
	redirect_function(nx, ny, nz);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3i(GLint nx, GLint ny, GLint nz)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", nx, ny, nz);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3i);
	redirect_function(nx, ny, nz);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", nx, ny, nz);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3s);
	redirect_function(nx, ny, nz);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glNormal3sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormal3sv);
	redirect_function(v);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", type, stride, pointer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glNormalPointer);
	redirect_function(type, stride, pointer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", left, right, bottom, top, zNear, zFar);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glOrtho);
	redirect_function(left, right, bottom, top, zNear, zFar);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPassThrough(GLfloat token)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", token);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPassThrough);
	redirect_function(token);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", map, mapsize, values);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelMapfv);
	redirect_function(map, mapsize, values);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", map, mapsize, values);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelMapuiv);
	redirect_function(map, mapsize, values);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", map, mapsize, values);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelMapusv);
	redirect_function(map, mapsize, values);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelStoref(GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelStoref);
	redirect_function(pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelStorei(GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelStorei);
	redirect_function(pname, param);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelTransferf(GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelTransferf);
	redirect_function(pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelTransferi(GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelTransferi);
	redirect_function(pname, param);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", xfactor, yfactor);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPixelZoom);
	redirect_function(xfactor, yfactor);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPointSize(GLfloat size)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", size);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPointSize);
	redirect_function(size);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPolygonMode(GLenum face, GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", face, mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPolygonMode);
	redirect_function(face, mode);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPolygonOffset(GLfloat factor, GLfloat units)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", factor, units);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPolygonOffset);
	redirect_function(factor, units);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPolygonStipple(const GLubyte* mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPolygonStipple);
	redirect_function(mask);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPopAttrib()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPopAttrib);
	redirect_function();
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPopClientAttrib()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPopClientAttrib);
	redirect_function();
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPopMatrix()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPopMatrix);
	redirect_function();
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPopName()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPopName);
	redirect_function();
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", n, textures, priorities);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPrioritizeTextures);
	redirect_function(n, textures, priorities);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glPushAttrib(GLbitfield mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPushAttrib);
	redirect_function(mask);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPushClientAttrib(GLbitfield mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPushClientAttrib);
	redirect_function(mask);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPushMatrix()
{
	OPENGL_DEBUG_LOG_FUNCTION("");
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPushMatrix);
	redirect_function();
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glPushName(GLuint name)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", name);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glPushName);
	redirect_function(name);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2d(GLdouble x, GLdouble y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2d);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2f(GLfloat x, GLfloat y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2f);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2i(GLint x, GLint y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2i);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2s(GLshort x, GLshort y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2s);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos2sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos2sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3d);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3f);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3i(GLint x, GLint y, GLint z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3i);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3s);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos3sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos3sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4d);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4dv);
	redirect_function(v);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4f);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4i);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4s);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRasterPos4sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRasterPos4sv);
	redirect_function(v);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glReadBuffer(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glReadBuffer);
	redirect_function(mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {}", x, y, width, height, format, type, pixels);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glReadPixels);
	redirect_function(x, y, width, height, format, type, pixels);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x1, y1, x2, y2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRectd);
	redirect_function(x1, y1, x2, y2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRectdv(const GLdouble* v1, const GLdouble* v2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v1, v2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRectdv);
	redirect_function(v1, v2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x1, y1, x2, y2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRectf);
	redirect_function(x1, y1, x2, y2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRectfv(const GLfloat* v1, const GLfloat* v2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", v1, v2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRectfv);
	redirect_function(v1, v2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x1, y1, x2, y2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRecti);
	redirect_function(x1, y1, x2, y2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRectiv(const GLint* v1, const GLint* v2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", v1, v2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRectiv);
	redirect_function(v1, v2);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x1, y1, x2, y2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRects);
	redirect_function(x1, y1, x2, y2);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRectsv(const GLshort* v1, const GLshort* v2)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", v1, v2);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRectsv);
	redirect_function(v1, v2);
}

HOOK_EXPORT GLint HOOK_CALL_CONVENTION glRenderMode(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRenderMode);
	auto result = redirect_function(mode);
	return result;
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", angle, x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRotated);
	redirect_function(angle, x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", angle, x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glRotatef);
	redirect_function(angle, x, y, z);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glScaled(GLdouble x, GLdouble y, GLdouble z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glScaled);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glScalef);
	redirect_function(x, y, z);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, width, height);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glScissor);
	redirect_function(x, y, width, height);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glSelectBuffer(GLsizei size, GLuint* buffer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", size, buffer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glSelectBuffer);
	redirect_function(size, buffer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glShadeModel(GLenum mode)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mode);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glShadeModel);
	redirect_function(mode);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", func, ref, mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glStencilFunc);
	redirect_function(func, ref, mask);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glStencilMask(GLuint mask)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", mask);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glStencilMask);
	redirect_function(mask);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", fail, zfail, zpass);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glStencilOp);
	redirect_function(fail, zfail, zpass);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1d(GLdouble s)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", s);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1d);
	redirect_function(s);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1f(GLfloat s)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", s);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1f);
	redirect_function(s);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1i(GLint s)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", s);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1i);
	redirect_function(s);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1s(GLshort s)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", s);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1s);
	redirect_function(s);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord1sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord1sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2d(GLdouble s, GLdouble t)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", s, t);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2d);
	redirect_function(s, t);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2f(GLfloat s, GLfloat t)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", s, t);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2f);
	redirect_function(s, t);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2i(GLint s, GLint t)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", s, t);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2i);
	redirect_function(s, t);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2s(GLshort s, GLshort t)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", s, t);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2s);
	redirect_function(s, t);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord2sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord2sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", s, t, r);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3d);
	redirect_function(s, t, r);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", s, t, r);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3f);
	redirect_function(s, t, r);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3i(GLint s, GLint t, GLint r)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", s, t, r);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3i);
	redirect_function(s, t, r);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", s, t, r);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3s);
	redirect_function(s, t, r);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord3sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord3sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", s, t, r, q);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4d);
	redirect_function(s, t, r, q);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", s, t, r, q);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4f);
	redirect_function(s, t, r, q);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", s, t, r, q);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4i);
	redirect_function(s, t, r, q);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", s, t, r, q);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4s);
	redirect_function(s, t, r, q);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoord4sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoord4sv);
	redirect_function(v);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", size, type, stride, pointer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexCoordPointer);
	redirect_function(size, type, stride, pointer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexEnvf);
	redirect_function(target, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexEnvfv);
	redirect_function(target, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexEnvi);
	redirect_function(target, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexEnviv(GLenum target, GLenum pname, const GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexEnviv);
	redirect_function(target, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexGend);
	redirect_function(coord, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexGendv(GLenum coord, GLenum pname, const GLdouble* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexGendv);
	redirect_function(coord, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexGenf);
	redirect_function(coord, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexGenfv);
	redirect_function(coord, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexGeni(GLenum coord, GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexGeni);
	redirect_function(coord, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexGeniv(GLenum coord, GLenum pname, const GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", coord, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexGeniv);
	redirect_function(coord, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {} {}", target, level, internalformat, width, border, format, type, pixels);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexImage1D);
	redirect_function(target, level, internalformat, width, border, format, type, pixels);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {} {} {}", target, level, internalformat, width, height, border, format, type, pixels);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexImage2D);
	redirect_function(target, level, internalformat, width, height, border, format, type, pixels);
}
void HOOK_CALL_CONVENTION glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {} {} {} {} {} {}", target, level, internalformat, width, height, depth, border, format, type, pixels);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexParameterf);
	redirect_function(target, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexParameterfv);
	redirect_function(target, pname, params);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, param);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexParameteri);
	redirect_function(target, pname, param);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, pname, params);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexParameteriv);
	redirect_function(target, pname, params);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, level, xoffset, width, format, type, pixels);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexSubImage1D);
	redirect_function(target, level, xoffset, width, format, type, pixels);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", target, level, xoffset, yoffset, width, height, format, type, pixels);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTexSubImage2D);
	redirect_function(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTranslated);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glTranslatef);
	redirect_function(x, y, z);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2d(GLdouble x, GLdouble y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2d);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2f(GLfloat x, GLfloat y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2f);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2i(GLint x, GLint y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2i);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2s(GLshort x, GLshort y)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {}", x, y);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2s);
	redirect_function(x, y);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex2sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex2sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3d);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3f);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3i(GLint x, GLint y, GLint z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3i);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3s(GLshort x, GLshort y, GLshort z)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {}", x, y, z);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3s);
	redirect_function(x, y, z);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex3sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex3sv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4d);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4dv(const GLdouble* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4dv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4f);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4fv(const GLfloat* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4fv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4i);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4iv(const GLint* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4iv);
	redirect_function(v);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, z, w);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4s);
	redirect_function(x, y, z, w);
}
HOOK_EXPORT void HOOK_CALL_CONVENTION glVertex4sv(const GLshort* v)
{
	OPENGL_DEBUG_LOG_FUNCTION("{}", v);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertex4sv);
	redirect_function(v);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", size, type, stride, pointer);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glVertexPointer);
	redirect_function(size, type, stride, pointer);
}

HOOK_EXPORT void HOOK_CALL_CONVENTION glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	OPENGL_DEBUG_LOG_FUNCTION("{} {} {} {}", x, y, width, height);
	static const auto redirect_function = Raysterizer::Hooks::GetFunction(glViewport);
	redirect_function(x, y, width, height);
}
