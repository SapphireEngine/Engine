#pragma once

#if SE_OPENGL

#	if SE_PLATFORM_WINDOWS
#		if !defined(SE_OPENGL_VERSION_MAJOR)
#			define SE_OPENGL_VERSION_MAJOR 4
#		endif
#		if !defined(SE_OPENGL_VERSION_MINOR)
#			define SE_OPENGL_VERSION_MINOR 6
#		endif
#       include <glad/glad.h>
#		include <GL/gl.h>
#		define GL_EXT_color_subtable
#		include <GL/glext.h>
#		include <GL/wglext.h>
#	endif

#	include "Platform/Window/Window.h"

#endif

//=============================================================================
SE_NAMESPACE_BEGIN

#if SE_OPENGL

#if SE_PLATFORM_WINDOWS
inline PROC GetExtension(const char *functionName)
{
    return wglGetProcAddress(functionName);
}
#elif SE_PLATFORM_LINUX
void (*GetExtension(const char* functionName))()
{
    return glXGetProcAddress((const GLubyte*)functionName);
}
#endif

#if !defined(GL_SR8_EXT)
#   define GL_SR8_EXT 0x8FBD
#endif
#if !defined(GL_SRG8_EXT)
#   define GL_SRG8_EXT 0x8FBE
#endif
#if !defined(EGL_OPENGL_ES3_BIT)
#   define EGL_OPENGL_ES3_BIT 0x0040
#endif

// Cross Platform OpenGL Context State
struct OpenGLState
{
#if SE_PLATFORM_WINDOWS
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
#elif SE_PLATFORM_XCB
    Display* display;
    uint32_t screen;
    xcb_connection_t* connection;
    xcb_glx_fbconfig_t fbconfigid;
    xcb_visualid_t visualid;
    xcb_glx_drawable_t glxDrawable;
    xcb_glx_context_t glxContext;
    xcb_glx_context_tag_t glxContextTag;
#elif SE_PLATFORM_XLIB
    Display* display;
    int screen;
    uint32_t visualid;
    GLXFBConfig glxFBConfig;
    GLXDrawable glxDrawable;
    GLXContext glxContext;
#elif SE_PLATFORM_ANDROID
    EGLDisplay display;
    EGLConfig config;
    EGLSurface tinySurface;
    EGLSurface mainSurface;
    EGLContext context;
#elif SE_PLATFORM_EMSCRIPTEN
    /// Window handle
    EGLNativeWindowType hWnd;

    /// EGL display
    EGLDisplay eglDisplay;

    /// EGL context
    EGLContext eglContext;

    /// EGL surface
    EGLSurface eglSurface;

#endif
};

// OpenGL context initialization data
struct OpenGLDesc
{
    int pixelFormat;
};

inline OpenGLState createContext(SE_NAMESPACE_WND::Window* window, const OpenGLDesc& desc)
{
    OpenGLState state;

#if SE_PLATFORM_WINDOWS
    state.hwnd = window->hwnd;

    state.hdc = GetDC(state.hwnd);

    if ( !state.hdc )
    {
        // Failed to acquire device context.
        return state;
    }

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                       // version
        PFD_DRAW_TO_WINDOW |     // must support windowed
            PFD_SUPPORT_OPENGL | // must support OpenGL
            PFD_DOUBLEBUFFER,    // must support double buffering
        PFD_TYPE_RGBA,           // iPixelType
        32,                      // cColorBits
        0,
        0, // cRedBits, cRedShift
        0,
        0, // cGreenBits, cGreenShift
        0,
        0, // cBlueBits, cBlueShift
        0,
        0,              // cAlphaBits, cAlphaShift
        0,              // cAccumBits
        0,              // cAccumRedBits
        0,              // cAccumGreenBits
        0,              // cAccumBlueBits
        0,              // cAccumAlphaBits
        0,              // cDepthBits
        0,              // cStencilBits
        0,              // cAuxBuffers
        PFD_MAIN_PLANE, // iLayerType
        0,              // bReserved
        0,              // dwLayerMask
        0,              // dwVisibleMask
        0               // dwDamageMask
    };

    int pixelFormat = ChoosePixelFormat(state.hdc, &pfd);
    if ( pixelFormat == 0 )
    {
        // Failed to find a suitable PixelFormat.
        return state;
    }

    if ( !SetPixelFormat(state.hdc, pixelFormat, &pfd) )
    {
        // Failed to set the PixelFormat.
        return state;
    }

    // Now that the pixel format is set, create a temporary context to get the
    // extensions.
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    {
        HGLRC hglrc = wglCreateContext(state.hdc);
        wglMakeCurrent(state.hdc, hglrc);

        wglCreateContextAttribsARB =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)GetExtension(
                "wglCreateContextAttribsARB");

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hglrc);
    }

    int attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB,
                     SE_OPENGL_VERSION_MAJOR,
                     WGL_CONTEXT_MINOR_VERSION_ARB,
                     SE_OPENGL_VERSION_MINOR,
                     WGL_CONTEXT_PROFILE_MASK_ARB,
                     WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                     WGL_CONTEXT_FLAGS_ARB,
                     WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                     0 };

    state.hglrc = wglCreateContextAttribsARB(state.hdc, NULL, attribs);
    if ( !state.hglrc )
    {
        // Failed to create GL context.
        return state;
    }

#elif SE_PLATFORM_XCB
    const char* displayName = NULL;
    state.display = XOpenDisplay(displayName);
    if ( !state.display )
    {
        // Unable to open X Display.");
        return state;
    }
    state.screen = 0;
    state.connection = xcb_connect(displayName, &state.screen);
    if ( xcb_connection_has_error(state.connection) )
    {
        // Failed to open XCB connection.");
        return state;
    }

    xcb_glx_query_version_cookie_t glx_query_version_cookie =
        xcb_glx_query_version(state.connection, OPENGL_VERSION_MAJOR,
            OPENGL_VERSION_MINOR);
    xcb_glx_query_version_reply_t* glx_query_version_reply =
        xcb_glx_query_version_reply(state.connection, glx_query_version_cookie,
            NULL);
    if ( glx_query_version_reply == NULL )
    {
        // Unable to retrieve GLX version.");
        return state;
    }
    free(glx_query_version_reply);

    xcb_glx_get_fb_configs_cookie_t get_fb_configs_cookie =
        xcb_glx_get_fb_configs(state.connection, state.screen);
    xcb_glx_get_fb_configs_reply_t* get_fb_configs_reply =
        xcb_glx_get_fb_configs_reply(state.connection, get_fb_configs_cookie,
            NULL);

    if ( get_fb_configs_reply == NULL ||
        get_fb_configs_reply->num_FB_configs == 0 )
    {
        // No valid framebuffer configurations found.");
        return state;
    }

    const GpuSurfaceBits_t bits =
        ksGpuContext_BitsForSurfaceFormat(colorFormat, depthFormat);

    const uint32_t* fb_configs_properties =
        xcb_glx_get_fb_configs_property_list(get_fb_configs_reply);
    const uint32_t fb_configs_num_properties =
        get_fb_configs_reply->num_properties;

    bool foundFbConfig = false;
    for ( int i = 0; i < get_fb_configs_reply->num_FB_configs; i++ )
    {
        const uint32_t* fb_config =
            fb_configs_properties + i * fb_configs_num_properties * 2;

        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_FBCONFIG_ID) == 0 )
        {
            continue;
        }
        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_VISUAL_ID) == 0 )
        {
            continue;
        }
        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_DOUBLEBUFFER) == 0 )
        {
            continue;
        }
        if ( (xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_RENDER_TYPE) &
            GLX_RGBA_BIT) == 0 )
        {
            continue;
        }
        if ( (xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_DRAWABLE_TYPE) &
            GLX_WINDOW_BIT) == 0 )
        {
            continue;
        }
        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_RED_SIZE) != bits.redBits )
        {
            continue;
        }
        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_GREEN_SIZE) != bits.greenBits )
        {
            continue;
        }
        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_BLUE_SIZE) != bits.blueBits )
        {
            continue;
        }
        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_ALPHA_SIZE) != bits.alphaBits )
        {
            continue;
        }
        if ( xcb_glx_get_property(fb_config, fb_configs_num_properties,
            GLX_DEPTH_SIZE) != bits.depthBits )
        {
            continue;
        }

        state.fbconfigid = xcb_glx_get_property(
            fb_config, fb_configs_num_properties, GLX_FBCONFIG_ID);
        state.visualid = xcb_glx_get_property(
            fb_config, fb_configs_num_properties, GLX_VISUAL_ID);
        foundFbConfig = true;
        break;
    }

    free(get_fb_configs_reply);

    if ( !foundFbConfig )
    {
        // Failed to to find desired framebuffer configuration.");
        return state;
    }

    state.connection = connection;

    // Create the context.
    uint32_t attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB,
                          OPENGL_VERSION_MAJOR,
                          GLX_CONTEXT_MINOR_VERSION_ARB,
                          OPENGL_VERSION_MINOR,
                          GLX_CONTEXT_PROFILE_MASK_ARB,
                          GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                          GLX_CONTEXT_FLAGS_ARB,
                          GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                          0 };

    state.glxContext = xcb_generate_id(state.connection);
    xcb_glx_create_context_attribs_arb(
        state.connection, // xcb_connection_t *	connection
        state.glxContext, // xcb_glx_context_t	context
        state.fbconfigid, // xcb_glx_fbconfig_t	fbconfig
        state.screen,     // uint32_t				screen
        0,                // xcb_glx_context_t	share_list
        1,                // uint8_t				is_direct
        4,                // uint32_t				num_attribs
        attribs);         // const uint32_t *		attribs

    // Make sure the context is direct.
    xcb_generic_error_t* error;
    xcb_glx_is_direct_cookie_t glx_is_direct_cookie =
        xcb_glx_is_direct_unchecked(state.connection, state.glxContext);
    xcb_glx_is_direct_reply_t* glx_is_direct_reply =
        xcb_glx_is_direct_reply(state.connection, glx_is_direct_cookie, &error);
    const bool is_direct =
        (glx_is_direct_reply != NULL && glx_is_direct_reply->is_direct);
    free(glx_is_direct_reply);

    if ( !is_direct )
    {
        // Unable to create direct rendering context.");
        return state;
    }

#elif SE_PLATFORM_XLIB

    const char* displayName = NULL;
    state.display = XOpenDisplay(displayName);
    if ( !state.display )
    {
        // Unable to open X Display.");
        return state;
    }
    state.screen = XDefaultScreen(state.display);

    int glxErrorBase;
    int glxEventBase;
    if ( !glXQueryExtension(state.display, &glxErrorBase, &glxEventBase) )
    {
        // X display does not support the GLX extension.");
        return state;
    }

    int glxVersionMajor;
    int glxVersionMinor;
    if ( !glXQueryVersion(state.display, &glxVersionMajor, &glxVersionMinor) )
    {
        // Unable to retrieve GLX version.");
        return state;
    }

    int fbConfigCount = 0;
    GLXFBConfig* fbConfigs =
        glXGetFBConfigs(state.display, state.screen, &fbConfigCount);
    if ( fbConfigCount == 0 )
    {
        // No valid framebuffer configurations found.");
        return state;
    }

    bool foundFbConfig = false;
    for ( int i = 0; i < fbConfigCount; i++ )
    {
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i],
            GLX_FBCONFIG_ID) == 0 )
        {
            continue;
        }
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i], GLX_VISUAL_ID) ==
            0 )
        {
            continue;
        }
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i],
            GLX_DOUBLEBUFFER) == 0 )
        {
            continue;
        }
        if ( (glxGetFBConfigAttrib2(state.display, fbConfigs[i],
            GLX_RENDER_TYPE) &
            GLX_RGBA_BIT) == 0 )
        {
            continue;
        }
        if ( (glxGetFBConfigAttrib2(state.display, fbConfigs[i],
            GLX_DRAWABLE_TYPE) &
            GLX_WINDOW_BIT) == 0 )
        {
            continue;
        }
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i], GLX_RED_SIZE) !=
            8 )
        {
            continue;
        }
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i],
            GLX_GREEN_SIZE) != 8 )
        {
            continue;
        }
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i], GLX_BLUE_SIZE) !=
            8 )
        {
            continue;
        }
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i],
            GLX_ALPHA_SIZE) != 8 )
        {
            continue;
        }
        if ( glxGetFBConfigAttrib2(state.display, fbConfigs[i],
            GLX_DEPTH_SIZE) != 0 )
        {
            continue;
        }

        state.visualid =
            glxGetFBConfigAttrib2(state.display, fbConfigs[i], GLX_VISUAL_ID);
        state.glxFBConfig = fbConfigs[i];
        foundFbConfig = true;
        break;
    }

    XFree(fbConfigs);

    if ( !foundFbConfig )
    {
        // Failed to to find desired framebuffer configuration.");
        return state;
    }

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
        (PFNGLXCREATECONTEXTATTRIBSARBPROC)GetExtension(
            "glXCreateContextAttribsARB");

    int attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB,
                     OPENGL_VERSION_MAJOR,
                     GLX_CONTEXT_MINOR_VERSION_ARB,
                     OPENGL_VERSION_MINOR,
                     GLX_CONTEXT_PROFILE_MASK_ARB,
                     GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                     GLX_CONTEXT_FLAGS_ARB,
                     GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                     0 };

    state.glxContext =
        glXCreateContextAttribsARB(state.display,     // Display *	dpy
            state.glxFBConfig, // GLXFBConfig	config
            NULL,     // GLXContext	share_context
            True,     // Bool			direct
            attribs); // const int *	attrib_list

    if ( stateglxContext == NULL )
    {
        // Unable to create GLX context.");
        return state;
    }

    if ( !glXIsDirect(statedisplay, stateglxContext) )
    {
        // Unable to create direct rendering context.");
        return state;
    }
#elif SE_PLATFORM_ANDROID || SE_PLATFORM_EMSCRIPTEN

    EGLint majorVersion = OPENGL_VERSION_MAJOR;
    EGLint minorVersion = OPENGL_VERSION_MINOR;

    state.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGL(eglInitialize(state.display, &majorVersion, &minorVersion));

    const int MAX_CONFIGS = 1024;
    EGLConfig configs[MAX_CONFIGS];
    EGLint numConfigs = 0;
    EGL(eglGetConfigs(state.display, configs, MAX_CONFIGS, &numConfigs));

    const EGLint configAttribs[] = { EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8,
                                    EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
                                    EGL_DEPTH_SIZE, 0,
        // EGL_STENCIL_SIZE,	0,
        EGL_SAMPLES, 0, EGL_NONE };

    state.config = 0;
    for ( int i = 0; i < numConfigs; i++ )
    {
        EGLint value = 0;

        eglGetConfigAttrib(state.display, configs[i], EGL_RENDERABLE_TYPE,
            &value);
        if ( (value & EGL_OPENGL_ES3_BIT) != EGL_OPENGL_ES3_BIT )
        {
            continue;
        }

        // Without EGL_KHR_surfaceless_context, the config needs to support both
        // pbuffers and window surfaces.
        eglGetConfigAttrib(state.display, configs[i], EGL_SURFACE_TYPE, &value);
        if ( (value & (EGL_WINDOW_BIT | EGL_PBUFFER_BIT)) !=
            (EGL_WINDOW_BIT | EGL_PBUFFER_BIT) )
        {
            continue;
        }

        int j = 0;
        for ( ; configAttribs[j] != EGL_NONE; j += 2 )
        {
            eglGetConfigAttrib(state.display, configs[i], configAttribs[j],
                &value);
            if ( value != configAttribs[j + 1] )
            {
                break;
            }
        }
        if ( configAttribs[j] == EGL_NONE )
        {
            state.config = configs[i];
            break;
        }
    }
    if ( state.config == 0 )
    {
        return state;
    }

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, OPENGL_VERSION_MAJOR,
                               EGL_NONE, EGL_NONE, EGL_NONE };
    state.context = eglCreateContext(state.display, state.config,
        EGL_NO_CONTEXT, contextAttribs);
    if ( state.context == EGL_NO_CONTEXT )
    {
        // eglCreateContext() failed: %s", EglErrorString(eglGetError()));
        return state;
    }

    const EGLint surfaceAttribs[] = { EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
    state.tinySurface =
        eglCreatePbufferSurface(state.display, state.config, surfaceAttribs);
    if ( state.tinySurface == EGL_NO_SURFACE )
    {
        // eglCreatePbufferSurface() failed: %s",
        // EglErrorString(eglGetError()));
        eglDestroyContext(state.display, state.context);
        state.context = EGL_NO_CONTEXT;
        return state;
    }
    state.mainSurface = state.tinySurface;

#endif

    return state;
}

inline void setContext(const OpenGLState& state)
{
#if SE_PLATFORM_WINDOWS
    wglMakeCurrent(state.hdc, state.hglrc);
#elif SE_PLATFORM_XCB
    xcb_glx_make_current_cookie_t glx_make_current_cookie =
        xcb_glx_make_current(state.connection, state.glxDrawable,
            state.glxContext, 0);
    xcb_glx_make_current_reply_t* glx_make_current_reply =
        xcb_glx_make_current_reply(state.connection, glx_make_current_cookie,
            NULL);
    state.glxContextTag = glx_make_current_reply->context_tag;
    free(glx_make_current_reply);
#elif SE_PLATFORM_XLIB
    glXMakeCurrent(state.display, state.glxDrawable, state.glxContext);
#elif SE_PLATFORM_ANDROID || SE_PLATFORM_EMSCRIPTEN
    EGL(eglMakeCurrent(state.display, state.mainSurface, state.mainSurface,
        state.context));
#endif
}

inline void unsetContext(const OpenGLState& state)
{
#if SE_PLATFORM_WINDOWS
    wglMakeCurrent(state.hdc, NULL);
#elif SE_PLATFORM_XLIB
    glXMakeCurrent(state.display, None, NULL);
#elif SE_PLATFORM_XCB
    xcb_glx_make_current(state.connection, 0, 0, 0);
#elif SE_PLATFORM_ANDROID || SE_PLATFORM_EMSCRIPTEN
    EGL(eglMakeCurrent(state.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
        EGL_NO_CONTEXT));
#endif
}

inline void destroyContext(const OpenGLState& state)
{
#if SE_PLATFORM_WINDOWS
    wglDeleteContext(state.hglrc);
#elif SE_PLATFORM_XLIB
    glXDeleteContext();
#elif SE_PLATFORM_XCB
    xcb_glx_delete_current(state.connection, 0, 0, 0);
#elif SE_PLATFORM_ANDROID || SE_PLATFORM_EMSCRIPTEN
    eglDeleteContext();
#endif
}

inline bool getCurrentContext(const OpenGLState& state)
{
#if SE_PLATFORM_WINDOWS
    return (wglGetCurrentContext() == state.hglrc);
#elif SE_PLATFORM_XLIB
    return (glXGetCurrentContext() == state.glxContext);
#elif SE_PLATFORM_XCB
    return true;
#elif SE_PLATFORM_ANDROID || SE_PLATFORM_EMSCRIPTEN
    return (eglGetCurrentContext() == state.context);
#endif
}

inline void swapBuffers(const OpenGLState& state)
{
#if SE_PLATFORM_WINDOWS
    SwapBuffers(state.hdc);
#elif SE_PLATFORM_XLIB
    glXSwapBuffers(state.x11.display, state.context.glx.window);
#elif SE_PLATFORM_XCB
    return true;
#elif SE_PLATFORM_ANDROID || SE_PLATFORM_EMSCRIPTEN
    eglSwapBuffers(state.display, state.surface);
#endif
}

#endif

SE_NAMESPACE_END
//=============================================================================