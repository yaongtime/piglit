/* Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file bindfragdataindexed-invalid-parameters.c
 * Verify that passing invalid parameters to glBindFragDataLocationIndexed
 * generates the correct errors.
 *
 * \author Ian Romanick + Dave Airlie (extended tests)
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 10;
#else // PIGLIT_USE_OPENGLES3
	config.supports_gl_es_version = 30;
#endif
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint max_draw_buffers, max_dual_source;
	GLuint prog;

#ifdef PIGLIT_USE_OPENGL
	piglit_require_gl_version(30);
	piglit_require_extension("GL_ARB_blend_func_extended");
#else // PIGLIT_USE_OPENGLES3
	piglit_require_extension("GL_EXT_blend_func_extended");
#endif

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
	glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &max_dual_source);

	/* Page 237 (page 253 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "BindFragDataLocation may be issued before any shader objects
	 *     are attached to a program object."
	 *
	 * As a result, all of the invalid location tests can be performed
	 * without a shader at all.  Only a program object is necessary.
	 */
	prog = glCreateProgram();
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Page 236 (page 252 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "The error INVALID VALUE is generated if colorNumber is equal
	 *     or greater than MAX DRAW BUFFERS."
	 *
	 * Since the colorNumber parameter is unsigned, this statement means
	 * an error should be generated if a negative number is used.
	 */
	printf("Trying location = -1...\n");
	glBindFragDataLocationIndexed(prog, -1, 0, "foo");
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	printf("Trying location = GL_MAX_DRAW_BUFFERS...\n");
	glBindFragDataLocationIndexed(prog, max_draw_buffers, 0, "foo");
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);


	/* ARB_blend_func_extended says,
	 * The error INVALID_VALUE is generated if <colorNumber> is equal
	 * or greater than MAX_DRAW_BUFFERS and <index> is zero, or if
	 * <colorNumber> is equal or greater than MAX_DUAL_SOURCE_DRAW_BUFFERS
	 * and <index> is greater than or equal to one.
	 */
	printf("Trying index > 1...\n");
	glBindFragDataLocationIndexed(prog, 0, 2, "foo");
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	printf("Trying location = GL_MAX_DUAL_SOURCE_DRAW_BUFFERS with index 1...\n");
	glBindFragDataLocationIndexed(prog, max_dual_source, 1, "foo");
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	/* Page 236 (page 252 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "The error INVALID_OPERATION is generated if name starts with
	 *     the reserved gl prefix."
	 *
	 * This was changed in a later version of the spec.  Page 279 (page
	 * 296 of the PDF) of the OpenGL 4.2 Core spec says:
	 *
	 *     "The error INVALID_OPERATION is generated if name starts with
	 *     the reserved gl_ prefix."
	 *
	 * The OpenGL 4.2 spec also matches the specified behavior of
	 * glBindAttribLocation as far back as OpenGL 2.0.
	 */
	printf("Trying name = `gl_FragColor'...\n");
	glBindFragDataLocationIndexed(prog, 0, 0, "gl_FragColor");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	printf("Trying name = `gl_FragDepth'...\n");
	glBindFragDataLocationIndexed(prog, 0, 0, "gl_FragDepth");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	printf("Trying name = `gl_'...\n");
	glBindFragDataLocationIndexed(prog, 0, 0, "gl_");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
