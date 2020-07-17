package chaos.app;

import android.annotation.TargetApi;
import android.os.Build;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnKeyListener;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.os.Bundle;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.WindowManager;

public class KeyPreference extends DialogPreference implements OnKeyListener {

	private int mOldValue;
	private int mNewValue;

	public KeyPreference(Context context) {
		this(context, null);
	}

	public KeyPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		setPositiveButtonText(R.string.key_clear);
		setDefaultValue(0);
	}

	@Override
	protected void onPrepareDialogBuilder(AlertDialog.Builder builder) {
		super.onPrepareDialogBuilder(builder);
		builder.setMessage(R.string.press_key_prompt).setOnKeyListener(this);
	}

	@Override
	protected void showDialog(Bundle state) {
		super.showDialog(state);

		final Dialog dialog = getDialog();
		if (dialog != null) {
			dialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);
		}
	}

	@Override
	public void onClick(DialogInterface dialog, int which) {
		// clear key binding
		if (which == DialogInterface.BUTTON_POSITIVE)
			mNewValue = 0;

		super.onClick(dialog, which);
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);

		if (!positiveResult)
			mNewValue = mOldValue;
		else {
			mOldValue = mNewValue;
			persistInt(mNewValue);
			updateSummary();
		}
	}

	@Override
	protected Object onGetDefaultValue(TypedArray a, int index) {
		return a.getInteger(index, 0);
	}

	@Override
	protected void onSetInitialValue(boolean restoreValue, Object defaultValue) {
		mOldValue = (restoreValue ?  getPersistedInt(0) : ((Integer) defaultValue).intValue());
		mNewValue = mOldValue;
		updateSummary();
	}

	private static boolean isKeyConfigurable(int keyCode) {
		return !(keyCode == KeyEvent.KEYCODE_HOME ||
				keyCode == KeyEvent.KEYCODE_MENU ||
				keyCode == KeyEvent.KEYCODE_POWER);
	}

	@Override
	public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
		if (!isKeyConfigurable(keyCode))
			return false;

		mNewValue = keyCode;
		super.onClick(dialog, DialogInterface.BUTTON_POSITIVE);
		dialog.dismiss();
		return true;
	}

	private void updateSummary() {
		setSummary(getKeyName(mNewValue));
	}

	@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
	private String getKeyName(int keyCode) {
		final int sdkVersion = Integer.parseInt(Build.VERSION.SDK);
		if (sdkVersion < Build.VERSION_CODES.HONEYCOMB_MR1)
			return "KEYCODE_#" + keyCode;
		else
			return KeyEvent.keyCodeToString(keyCode);
	}
}
