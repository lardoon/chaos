#include <jni.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <GLES/gl.h>
#include <android/log.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES/glext.h>
#include "chaos/porting.h"
#include "port/android/jni/screen.h"
#include "chaos/rand.h"
#ifdef PROFILE
#include "prof.h"
#include <stdlib.h>
#endif

#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
#endif
#ifdef HAS_STRESS_TEST
void stress_mode(void);
#endif

extern int g_keys_down[];
extern int g_touch_x;
extern int g_touch_y;
GLuint texture;
static uint16_t *s_pixels = 0;
static int s_has_OES_draw_texture = 0;
static int s_already_started = 0;
static pthread_mutex_t s_vsync_mutex;
static pthread_cond_t s_vsync_cond;
char *s_language = 0;
#define S_PIXELS_SIZE (sizeof(s_pixels[0]) * (SCREEN_WIDTH * 2) * (SCREEN_HEIGHT + 1) * 64)
#define FADE_LEVEL(n)  ((n) << 6)

static void JNICALL chaos_gl_created(JNIEnv *env, jclass clazz UNUSED, jboolean is_arc, jint filter);
static void JNICALL chaos_gl_filter(JNIEnv *env, jclass clazz UNUSED, jint filter);
static void JNICALL chaos_gl_render(JNIEnv *env, jclass clazz);
static void JNICALL chaos_gl_resize(JNIEnv *env, jclass clazz, jint w, jint h);
static void JNICALL chaos_touch_event(JNIEnv *env, jclass clazz, jint x, jint y, jboolean is_down);
static void JNICALL chaos_key_up(JNIEnv *env, jclass clazz, jint key_code);
static void JNICALL chaos_key_down(JNIEnv *env, jclass clazz, jint key_code);
static void JNICALL chaos_start(JNIEnv *env, jclass clazz);

static JNINativeMethod s_methods[] = {
	{"chaos_start", "()V", (void*) chaos_start},
	{"chaos_key_down", "(I)V", (void*) chaos_key_down},
	{"chaos_key_up", "(I)V", (void*) chaos_key_up},
	{"chaos_touch_event", "(IIZ)V", (void*) chaos_touch_event},
	{"chaos_gl_created", "(ZI)V", (void*) chaos_gl_created},
	{"chaos_gl_resize", "(II)V", (void*) chaos_gl_resize},
	{"chaos_gl_render", "()V", (void*) chaos_gl_render},
	{"chaos_gl_filter", "(I)V", (void*) chaos_gl_filter},
};

static int register_natives(JNIEnv *env);

static void check_OES_draw_texture(jboolean is_arc)
{
	const char *extensions = (const char *) glGetString(GL_EXTENSIONS);
	__android_log_print(ANDROID_LOG_INFO, "CHAOS", "isARC %d, GL EXTENSIONS: %s", is_arc, extensions);
	s_has_OES_draw_texture = !is_arc && strstr(extensions, "GL_OES_draw_texture") != NULL;
}

static void calc_new_size(int w, int h, int *neww, int *newh,
		int *newx, int *newy)
{
	*newh = h;
	*neww = w;
	*newx = 0;
	*newy = 0;
	if (w > h) {
		*neww = (h * SCREEN_WIDTH) / SCREEN_HEIGHT;
		*newx = (w - (*neww)) / 2;
	} else {
		*newh = (w * SCREEN_HEIGHT / SCREEN_WIDTH);
		*newy = (h - (*newh)) / 2;
	}
}


/* disable these capabilities. */
static GLuint s_disable_caps[] = {
	GL_FOG,
	GL_LIGHTING,
	GL_CULL_FACE,
	GL_ALPHA_TEST,
	GL_BLEND,
	GL_COLOR_LOGIC_OP,
	GL_DITHER,
	GL_STENCIL_TEST,
	GL_DEPTH_TEST,
	GL_COLOR_MATERIAL,
	0
};

struct screen_pos {
	int x, y, w, h;
} s_screen_pos;

struct screen_pos s_screen_size;


struct fade_info {
	int pending;
	int level;
} s_fade_info;

static void switch_to_hud(void)
{
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	/* x, y, width, height */
	glOrthof(0, SCREEN_WIDTH * 8, 0, SCREEN_HEIGHT * 8, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static void JNICALL chaos_gl_render_slow(JNIEnv *env UNUSED, jclass clazz UNUSED)
{
	memset(s_pixels, 0, S_PIXELS_SIZE);
	android_render_screen(s_pixels);
	pthread_cond_signal(&s_vsync_cond);
	if (s_fade_info.pending) {
		int level = s_fade_info.level;
		glColor4x(level, level, level, 0x10000);
		s_fade_info.pending = 0;
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glTexSubImage2D(GL_TEXTURE_2D,		/* target */
			0,			/* level */
			0,			/* xoffset */
			0,			/* yoffset */
			SCREEN_WIDTH * 8 * 2,	/* width */
			SCREEN_HEIGHT * 8,	/* height */
			GL_RGB,			/* format */
			GL_UNSIGNED_SHORT_5_6_5, /* type */
			s_pixels);		/* pixels */
	static const unsigned char indices[] = {
		0, 1, 3, 3, 2, 1
	};
	glDrawElements(GL_TRIANGLE_STRIP, sizeof(indices),
			GL_UNSIGNED_BYTE, indices);
}

static void JNICALL chaos_gl_filter(JNIEnv *env, jclass clazz UNUSED, jint filter)
{
	if (s_pixels == 0)
		return;
	glTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER, filter);
	glTexParameterf(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER, filter);
}

/**
 * is_arc means "are we running on ARC?"
 */
static void JNICALL chaos_gl_created(JNIEnv *env, jclass clazz, jboolean is_arc, jint filter)
{
	check_OES_draw_texture(is_arc);
	if (!s_has_OES_draw_texture) {
		s_methods[6].fnPtr = chaos_gl_render_slow;
		register_natives(env);
	} else {
		GLuint *start = s_disable_caps;
		while (*start) {
			glDisable(*start++);
		}
	}
	if (s_pixels == 0)
		s_pixels = malloc(S_PIXELS_SIZE);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	chaos_gl_filter(env, clazz, filter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glShadeModel(GL_FLAT);
	glTexImage2D(GL_TEXTURE_2D,		/* target */
			0,			/* level */
			GL_RGB,			/* internal format */
			TEXTURE_WIDTH,		/* width */
			TEXTURE_HEIGHT,		/* height */
			0,			/* border */
			GL_RGB,			/* format */
			GL_UNSIGNED_SHORT_5_6_5,/* type */
			NULL);			/* pixels */
	glClearColor(0, 0, 0, 0);
}

/* init and resize */
static void JNICALL chaos_gl_resize(JNIEnv *env UNUSED, jclass clazz UNUSED, jint w, jint h)
{
	int neww, newh, newx, newy;
	calc_new_size(w, h, &neww, &newh, &newx, &newy);
	s_fade_info.pending = 1;
	s_fade_info.level = FADE_LEVEL(1024);

	s_screen_pos.x = newx;
	s_screen_pos.y = newy;
	s_screen_pos.w = neww;
	s_screen_pos.h = newh;
	if (s_has_OES_draw_texture) {
		int rect[4] = {0, SCREEN_HEIGHT * 8, SCREEN_WIDTH * 8, -SCREEN_HEIGHT *8};
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
	} else {
		s_screen_size.x = 0;
		s_screen_size.y = 0;
		s_screen_size.w = w;
		s_screen_size.h = h;
		glLoadIdentity();
		glViewport(s_screen_pos.x, s_screen_pos.y, s_screen_pos.w, s_screen_pos.h);
		switch_to_hud();
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		static short screen_verts[] = {
			0, 0,
			SCREEN_WIDTH * 8, 0,
			SCREEN_WIDTH * 8, SCREEN_HEIGHT * 8,
			0, SCREEN_HEIGHT * 8,
		};
		static const float texture_coords[] = {
			0, (8.0f * SCREEN_HEIGHT) / TEXTURE_HEIGHT,
			(8.0f * SCREEN_WIDTH) / TEXTURE_WIDTH, (8.0f * SCREEN_HEIGHT) / TEXTURE_HEIGHT,
			(8.0f * SCREEN_WIDTH) /TEXTURE_WIDTH, 0,
			0, 0,
		};
		glVertexPointer(2, GL_SHORT, 0, screen_verts);
		glTexCoordPointer(2, GL_FLOAT, 0, texture_coords);
	}
}

static void JNICALL chaos_gl_render(JNIEnv *env UNUSED, jclass clazz UNUSED)
{
	memset(s_pixels, 0, S_PIXELS_SIZE);
	android_render_screen(s_pixels);
	pthread_cond_signal(&s_vsync_cond);
	if (s_fade_info.pending) {
		int level = s_fade_info.level;
		glColor4x(level, level, level, 1);
		s_fade_info.pending = 0;
	}
	glClear(GL_COLOR_BUFFER_BIT);
	glTexSubImage2D(GL_TEXTURE_2D,		/* target */
			0,			/* level */
			0,			/* xoffset */
			0,			/* yoffset */
			SCREEN_WIDTH * 8 * 2,	/* width */
			SCREEN_HEIGHT * 8 + 1,	/* height */
			GL_RGB,			/* format */
			GL_UNSIGNED_SHORT_5_6_5, /* type */
			s_pixels);		/* pixels */
	glDrawTexiOES(s_screen_pos.x, s_screen_pos.y, 0, s_screen_pos.w, s_screen_pos.h);
}

static JNIEnv *s_env = NULL;
static jclass s_ChaosView_class;


void android_wait_vsync(void)
{
	/* wait for the game_frame to change... */
	pthread_mutex_lock(&s_vsync_mutex);
	pthread_cond_wait(&s_vsync_cond, &s_vsync_mutex);
	pthread_mutex_unlock(&s_vsync_mutex);
}

static void JNICALL chaos_start(JNIEnv *env, jclass clazz)
{
	if (s_already_started)
		return;
#ifdef PROFILE
	setenv("CPUPROFILE", "/data/data/chaos.app/files/gmon.out", 1);
	monstartup("chaos_native.so");
#endif
	s_env = env;
	s_ChaosView_class = clazz;
	s_already_started = 1;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint32_t val = (tv.tv_usec << 8) | (tv.tv_sec & 0xff);
	setSeed(val);
	chaos_main();
	pthread_cond_init(&s_vsync_cond, NULL);
	pthread_mutex_init(&s_vsync_mutex, NULL);

	while (1) {
#ifdef HAS_STRESS_TEST
		stress_mode();
#endif
		chaos_one_frame();
		android_wait_vsync();
	}
}

static void JNICALL chaos_key_down(JNIEnv *env UNUSED, jclass clazz UNUSED, jint key_code)
{
	g_keys_down[key_code] = 1;
}

static void JNICALL chaos_key_up(JNIEnv *env UNUSED, jclass clazz UNUSED, jint key_code)
{
	g_keys_down[key_code] = 0;
}

static void JNICALL chaos_touch_event(JNIEnv *env UNUSED, jclass clazz UNUSED, jint x, jint y, jboolean is_down)
{
	/* take off half a square for android's weird touchiness */
	g_touch_x = ((x - s_screen_pos.x - 4) * SCREEN_WIDTH * 8) / s_screen_pos.w;
	g_touch_y = ((y - s_screen_pos.y - 4) * SCREEN_HEIGHT * 8) / s_screen_pos.h;
	g_keys_down[CHAOS_KEY_TOUCH] = (is_down == JNI_TRUE);
}

static const char s_class_path_name[] = "chaos/app/ChaosView";

static int register_native_methods(JNIEnv* env,
		const char* class_name,
		JNINativeMethod* methods,
		int num_methods)
{
	jclass clazz;

	clazz = (*env)->FindClass(env, class_name);
	if (clazz == NULL) {
		return JNI_FALSE;
	}
	if ((*env)->RegisterNatives(env, clazz, methods, num_methods) < 0) {
		return JNI_FALSE;
	}
	return JNI_TRUE;
}

static int register_natives(JNIEnv *env)
{
	return register_native_methods(env,
			s_class_path_name,
			s_methods,
			NELEM(s_methods));
}

jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved UNUSED)
{
	JNIEnv* env = NULL;
	jint result = -1;

	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		goto bail;
	}
	assert(env != NULL);

	if (register_natives(env) < 0) {
		goto bail;
	}

	/* success -- return valid version number */
	result = JNI_VERSION_1_4;
bail:
	return result;
}

void platform_dprint(const char *s UNUSED)
{
}

void platform_play_soundfx(int soundid)
{
	if (s_env == NULL)
		return;
	JNIEnv *env = s_env;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "playSound", "(I)V");
	if (mid == NULL)
		return;
	(*env)->CallStaticVoidMethod(env, cls, mid, soundid);
}

void platform_set_fade_level(int level)
{
	if (level < 0 || level > 16)
		return;

	s_fade_info.level = FADE_LEVEL((16 - level) * 64);
	s_fade_info.pending = 1;
}

void platform_exit(void)
{
	if (s_env == NULL)
		return;
	JNIEnv *env = s_env;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "exit", "()V");
	if (mid == NULL)
		return;
#ifdef PROFILE
	moncleanup();
#endif
	(*env)->CallStaticVoidMethod(env, cls, mid);
}

char *platform_load_options(void)
{
	jbyteArray jb;
	JNIEnv *env = s_env;
	if (env == NULL)
		return NULL;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "loadOptions", "([B)V");
	if (mid == NULL)
		return NULL;
	int size = 200;
	char *saves = calloc(size, 1);

	jb = (*env)->NewByteArray(env, size);
	(*env)->CallStaticVoidMethod(env, cls, mid, jb);
	(*env)->GetByteArrayRegion(env, jb, 0, size, (jbyte *)saves);
	(*env)->DeleteLocalRef(env, jb);
	return saves;
}

void platform_save_options(const char *saves, unsigned int size)
{
	jbyteArray jb;
	JNIEnv *env = s_env;
	if (env == NULL)
		return;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "saveOptions", "([B)V");
	if (mid == NULL)
		return;
	jb = (*env)->NewByteArray(env, size);
	(*env)->SetByteArrayRegion(env, jb, 0,
			size, (jbyte *)saves);
	(*env)->CallStaticVoidMethod(env, cls, mid, jb);
	(*env)->DeleteLocalRef(env, jb);
}

char *platform_load_game(void)
{
	jbyteArray jb;
	JNIEnv *env = s_env;
	if (env == NULL)
		return NULL;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "loadGame", "([B)V");
	if (mid == NULL)
		return NULL;
	int size = 4200;
	char *saves = calloc(size, 1);

	jb = (*env)->NewByteArray(env, size);
	(*env)->CallStaticVoidMethod(env, cls, mid, jb);
	(*env)->GetByteArrayRegion(env, jb, 0, size, (jbyte *)saves);
	(*env)->DeleteLocalRef(env, jb);
	return saves;
}

int platform_has_saved_game(void)
{
	JNIEnv *env = s_env;
	if (env == NULL)
		return 0;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "hasSaveGame", "()Z");
	if (mid == NULL)
		return 0;
	return (*env)->CallStaticBooleanMethod(env, cls, mid);
}

void platform_save_game(const char *game, unsigned int size)
{
	jbyteArray jb;
	JNIEnv *env = s_env;
	if (env == NULL)
		return;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "saveGame", "([B)V");
	if (mid == NULL)
		return;
	jb = (*env)->NewByteArray(env, size);
	(*env)->SetByteArrayRegion(env, jb, 0,
			size, (jbyte *)game);
	(*env)->CallStaticVoidMethod(env, cls, mid, jb);
	(*env)->DeleteLocalRef(env, jb);
}

const char *platform_get_lang(void)
{
	if (s_language != 0)
		return s_language;
	/* call Locale.getDefault().getLanguage() */
	JNIEnv *env = s_env;
	jclass cls = (*env)->FindClass(env, "java/util/Locale");
	if (cls == NULL)
		return "en";
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "getDefault", "()Ljava/util/Locale;");
	if (mid == NULL)
		return "en";
	jobject locale = (*env)->CallStaticObjectMethod(env, cls, mid);
	mid = (*env)->GetMethodID(env, cls, "getLanguage", "()Ljava/lang/String;");
	if (mid == NULL)
		return "en";
	jobject str = (*env)->CallObjectMethod(env, locale, mid);
	const char *cstr = (*env)->GetStringUTFChars(env, str, NULL);
	s_language = strdup(cstr);
	(*env)->ReleaseStringUTFChars(env, str, cstr);
	return s_language;
}

void platform_more_options(void)
{
	JNIEnv *env = s_env;
	if (env == NULL)
		return;
	jclass cls = s_ChaosView_class;
	jmethodID mid = (*env)->GetStaticMethodID(env, cls, "startPrefs", "()V");
	if (mid == NULL)
		return;
	(*env)->CallStaticVoidMethod(env, cls, mid);
}
