package chaos.app;



import android.annotation.TargetApi;
import android.content.Intent;
import android.app.Activity;
import android.os.Bundle;
import android.os.Build;
import android.view.Window;
import android.view.View;
import android.media.AudioManager;

public class ChaosActivity extends Activity {
	private ChaosView m_chaosView;

	@Override
	public final void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		updateImmersiveMode();
		setContentView(R.layout.main);
		m_chaosView = ((ChaosView) findViewById(R.id.chaos_view));
		setVolumeControlStream(AudioManager.STREAM_MUSIC);
	}

	@Override
	public void onWindowFocusChanged( boolean hasFocus ) {
		super.onWindowFocusChanged( hasFocus );
		if (hasFocus) {
			updateImmersiveMode();
		}
	}

	@Override
	public final void onPause() {
		m_chaosView.onPause();
		super.onPause();
	}

	@Override
	public final void onResume() {
		super.onResume();
		if (m_chaosView != null)
			m_chaosView.onResume();
		updateImmersiveMode();
	}

	public void startPrefs() {
		Intent intent = new Intent(this, SetPreferencesActivity.class);
		startActivityForResult(intent, 1);
	}

	public static boolean hasImmersiveMode() {
		return true;
	}

	@TargetApi(Build.VERSION_CODES.KITKAT)
	public void updateImmersiveMode() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
			this.getWindow().getDecorView().setSystemUiVisibility(
					hasImmersiveMode() ?
					View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
					View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
					View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
					View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
					View.SYSTEM_UI_FLAG_FULLSCREEN |
					View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
					:
					0 );
		}
	}

}
