package chaos.app;

import android.preference.PreferenceManager;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import java.io.Closeable;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.util.Log;
import android.content.Context;
import android.content.pm.ResolveInfo;
import android.media.SoundPool;
import android.media.AudioManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.SparseIntArray;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.Toast;

public class ChaosView extends GLSurfaceView implements OnSharedPreferenceChangeListener {
	private static native void chaos_start();
	private static native void chaos_key_down(int keyCode);
	private static native void chaos_key_up(int keyCode);
	private static native void chaos_touch_event(int x, int y, boolean is_down);
	private static native void chaos_gl_created(boolean is_arc, int filter);
	private static native void chaos_gl_filter(int filter);
	private static native void chaos_gl_resize(int w, int h);
	private static native void chaos_gl_render();

	static final String OPTIONS_FILE = "options.txt";
	static final String GAME_FILE = "game.txt";
	static final Object[] s_dataLock = new Object[0];
	private static final int SOUND_PRIORITY = 1;
	private static final int NUMBER_OF_CHANNELS = 4;
	private static final int STREAM_MUSIC = 3;
	/* limit to 30 FPS */
	private static final int RATE = 30;
	private final ChaosRenderer mRenderer;

	private static ChaosView s_currentInstance;
	private SoundPool m_soundPool = new SoundPool(NUMBER_OF_CHANNELS, STREAM_MUSIC, 0);
	private SparseIntArray m_resourceIdToSoundIdMap = new SparseIntArray();
	private VersionedBackupManager m_backupManager;

	public static final int[] DEFAULT_KEY_CODES = {
		KeyEvent.KEYCODE_DPAD_CENTER,	/* KEY_A */
		KeyEvent.KEYCODE_BACK,		/* KEY_B */
		KeyEvent.KEYCODE_S,		/* KEY_SELECT */
		KeyEvent.KEYCODE_MENU,		/* KEY_START */
		KeyEvent.KEYCODE_DPAD_RIGHT,	/* KEY_RIGHT  */
		KeyEvent.KEYCODE_DPAD_LEFT,	/* KEY_LEFT   */
		KeyEvent.KEYCODE_DPAD_UP,	/* KEY_UP     */
		KeyEvent.KEYCODE_DPAD_DOWN,	/* KEY_DOWN   */
		KeyEvent.KEYCODE_SEARCH,	/* KEY_R */
		KeyEvent.KEYCODE_L		/* KEY_L */
	};

	public static final String[] KEY_CODE_PREFS = {
		"key_a",
		"key_b",
		"key_select",
		"key_start",
		"key_right",
		"key_left",
		"key_up",
		"key_down",
		"key_r",
		"key_l",
	};

	public static final int[] KEY_CODE_TITLES = {
		R.string.key_a,
		R.string.key_b,
		R.string.key_select,
		R.string.key_start,
		R.string.key_right,
		R.string.key_left,
		R.string.key_up,
		R.string.key_down,
		R.string.key_r,
		R.string.key_l,
	};

	private int[] mKeyCodes = new int[DEFAULT_KEY_CODES.length];

	static {
		int expected = KEY_CODE_PREFS.length;
		if (expected != DEFAULT_KEY_CODES.length)
			throw new AssertionError(String.format("Unexpected length %d for DEFAULT_KEY_CODES (expected %d)",
						DEFAULT_KEY_CODES.length, expected));
		if (expected != KEY_CODE_TITLES.length)
			throw new AssertionError(String.format("Unexpected length %d for KEY_CODE_TITLES (expected %d)",
						KEY_CODE_TITLES.length, expected));
	}

	private static class ChaosRenderer implements GLSurfaceView.Renderer {
		private long m_lastFrame = 0;
		private final boolean m_isARC;
		private int mFilter = GL10.GL_NEAREST;

		public ChaosRenderer(boolean isARC) {
			m_isARC = isARC;
		}

		public void setFilter(int filter) {
			mFilter = filter;
			chaos_gl_filter(mFilter);
		}

		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int w, int h) {
			chaos_gl_created(m_isARC, mFilter);
			chaos_gl_resize(w, h);
		}

		@Override
		public void onDrawFrame(GL10 gl) {
			long currentFrame = SystemClock.uptimeMillis();
			long diff = currentFrame - m_lastFrame;
			m_lastFrame = currentFrame;
			chaos_gl_render();
			try {
				long sleepfor = (1000 / RATE) - diff;
				if (sleepfor > 0) {
					Thread.sleep(sleepfor);
				}
			} catch (InterruptedException ex) { }
		}
	}

	private static void setInstance(ChaosView view) {
		s_currentInstance = view;
	}

	/**
	 * Check if running on the Android Runtime for Chrome.
	 * MODEL is "App Runtime for Chrome", and PRODUCT is "arc".
	 */
	private boolean isChrome() {
		if (Build.PRODUCT.equalsIgnoreCase("arc"))
			return true;
		if (Build.MODEL.contains("Chrome"))
			return true;
		return false;
	}

	/* instantiated from resource */
	public ChaosView(final Context context, AttributeSet attrs) {
		super(context, attrs);
		setInstance(this);
		startNativeLoop();
		m_handler = new TrackballHandler();
		m_backupManager = VersionedBackupManager.newInstance(getContext());
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		mRenderer = new ChaosRenderer(isChrome());
		mRenderer.setFilter(prefs.getBoolean("pixels_enabled", true) ? GL10.GL_NEAREST : GL10.GL_LINEAR);
		setRenderer(mRenderer);
		requestFocus();
		/* android:focusableInTouchMode in layout XML doesn't seem to work */
		setFocusableInTouchMode(true);

		(new Thread() {
			@Override
			public void run() {
				/* load sound effects */
				for (int i = R.raw.attack; i <= R.raw.walk; i++) {
					Integer soundId = m_soundPool.load(context, i, SOUND_PRIORITY);
					m_resourceIdToSoundIdMap.put(i, soundId);
				}
			}
		}).start();
		for (int i = 0; i < DEFAULT_KEY_CODES.length; ++i) {
			mKeyCodes[i] = prefs.getInt(KEY_CODE_PREFS[i], DEFAULT_KEY_CODES[i]);
		}
		prefs.registerOnSharedPreferenceChangeListener(this);
	}

	private void updateKeyCodes(SharedPreferences prefs, String key) {
		for (int i = 0; i < KEY_CODE_PREFS.length; i++) {
			if (!key.equals(KEY_CODE_PREFS[i]))
				continue;
			mKeyCodes[i] = prefs.getInt(key, DEFAULT_KEY_CODES[i]);
			break;
		}
	}

	@Override
	public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
		if (key.equals("pixels_enabled")) {
			mRenderer.setFilter(prefs.getBoolean(key, true) ? GL10.GL_NEAREST : GL10.GL_LINEAR);
		}
		updateKeyCodes(prefs, key);
	}

	private void startNativeLoop() {
		(new Thread() {
			@Override
			public void run() {
				chaos_start();
			}
		}).start();
	}

	private void playSoundImpl(int sound) {
		int resourceId = R.raw.attack + sound;
		Integer soundId = m_resourceIdToSoundIdMap.get(resourceId);
		if (soundId == null)
			return;
		AudioManager audioManager = (AudioManager) getContext().getSystemService( Context.AUDIO_SERVICE);
		float actualVolume = (float) audioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
		float maxVolume = (float) audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
		float volume = actualVolume / (2 * maxVolume);
		m_soundPool.play(soundId, volume, volume, 0, 0, 1.0f);
	}

	/** Called from C code */
	public static void playSound(int sound) {
		s_currentInstance.playSoundImpl(sound);
	}

	public static void exit() {
		android.os.Process.killProcess(android.os.Process.myPid());
	}

	private TrackballHandler m_handler;
	public static class TrackballHandler extends Handler {
		public float dx = 0;
		public float dy = 0;

		public void doMove() {
			float limit = 0.15f;
			if (dx >= limit)
				chaos_key_down(4);
			if (dx <= -limit)
				chaos_key_down(5);
			if (dy >= limit)
				chaos_key_down(7);
			if (dy <= -limit)
				chaos_key_down(6);
			long delay = 100;
			keysUp(delay);
			dx = 0;
			dy = 0;
		}
		@Override
		public void handleMessage(Message msg) {
			if (msg.what == 0) {
				doMove();
			} else {
				for (int i = 0; i < DEFAULT_KEY_CODES.length; i++)
					chaos_key_up(i);
			}
		}

		public void schedule(long delayMillis) {
			this.removeMessages(0);
			this.removeMessages(1);
			sendMessageDelayed(obtainMessage(0), delayMillis);
		}

		public void keysUp(long delayMillis) {
			this.removeMessages(1);
			sendMessageDelayed(obtainMessage(1), delayMillis);
		}
	}

	@Override
	public boolean onTrackballEvent(MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_MOVE) {
			float x = event.getX();
			float y = event.getY();
			m_handler.dx += x;
			m_handler.dy += y;
			m_handler.doMove();
			/*else
				m_handler.schedule(200);*/
		}
		if (event.getAction() == MotionEvent.ACTION_DOWN) {
			chaos_key_down(0);
		}
		if (event.getAction() == MotionEvent.ACTION_UP) {
			chaos_key_up(0);
		}
		return true;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent msg) {
		/* set in key array   */
		for (int i = 0; i < mKeyCodes.length; i++) {
			if (mKeyCodes[i] == keyCode) {
				chaos_key_down(i);
				return true;
			}
		}
		return super.onKeyDown(keyCode, msg);
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent msg) {
		for (int i = 0; i < mKeyCodes.length; i++) {
			if (mKeyCodes[i] == keyCode) {
				chaos_key_up(i);
				return true;
			}
		}
		return super.onKeyUp(keyCode, msg);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		float x = event.getX();
		float y = event.getY();
		if (event.getAction() == MotionEvent.ACTION_DOWN)
			chaos_touch_event((int) x, (int) y, true);
		else if (event.getAction() == MotionEvent.ACTION_UP)
			chaos_touch_event((int) x, (int) y, false);
		return true;
	}

	public static void saveOptions(byte []data) {
		synchronized (s_dataLock) {
			s_currentInstance.saveOptionsImpl(data);
		}
	}

	public static void saveGame(byte []data) {
		synchronized (s_dataLock) {
			s_currentInstance.saveGameImpl(data);
		}
	}

	public static void loadOptions(byte []data) {
		synchronized (s_dataLock) {
			s_currentInstance.loadOptionsImpl(data);
		}
	}

	public static void loadGame(byte []data) {
		synchronized (s_dataLock) {
			s_currentInstance.loadGameImpl(data);
		}
	}

	public static boolean hasSaveGame() {
		return s_currentInstance.hasSaveGameImpl();
	}

	public static void startPrefs() {
		((ChaosActivity)s_currentInstance.getContext()).startPrefs();
	}

	private void closeSilently(Closeable c) {
		if (c == null)
			return;
		try {
			c.close();
		} catch (Throwable t) { }
	}

	private void toast(String s) {
		Toast.makeText(getContext(), s, Toast.LENGTH_SHORT).show();
	}

	private void saveOptionsImpl(byte []data) {
		savePrivateFile(data, OPTIONS_FILE);
	}

	private void saveGameImpl(byte []data) {
		savePrivateFile(data, GAME_FILE);
	}

	private void savePrivateFile(byte[] data, final String filename) {
		FileOutputStream tempStream = null;
		try {
			tempStream = getContext().openFileOutput(filename, Context.MODE_PRIVATE);
			tempStream.write(data);
			tempStream.flush();
			tempStream.close();
			m_backupManager.dataChanged();
		} catch (IOException io) {
			toast("Error saving file " + filename);
		} finally {
			closeSilently(tempStream);
		}
	}

	private boolean hasSaveGameImpl() {
		FileInputStream stream = null;
		try {
			stream = getContext().openFileInput(GAME_FILE);
			return true;
		} catch (FileNotFoundException ex) {
			return false;
		} finally {
			closeSilently(stream);
		}
	}

	private void loadGameImpl(byte []data) {
		loadPrivateFile(data, GAME_FILE);
	}

	private void loadOptionsImpl(byte []data) {
		loadPrivateFile(data, OPTIONS_FILE);
	}

	private void loadPrivateFile(byte[] data, final String fileName) {
		DataInputStream in = null;
		try {
			in = new DataInputStream(getContext().openFileInput(fileName));
			int read = 0;
			int len = data.length;
			while (read < len && in.available() > 0)
				read += in.read(data, read, len - read);
			in.close();
		} catch (IOException e) {
			Log.d("CHAOS", "IOException loading file:" + e.getMessage());
		} finally {
			closeSilently(in);
		}
	}

	static {
		System.loadLibrary("chaos_native");
	}

}
