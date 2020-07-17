package chaos.app;

import android.preference.PreferenceGroup;
import android.preference.PreferenceActivity;
import android.os.Bundle;

public class SetPreferencesActivity extends PreferenceActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
		PreferenceGroup group = (PreferenceGroup) findPreference("input_settings");
		for (int i = 0; i < ChaosView.KEY_CODE_PREFS.length; i++) {
			KeyPreference pref = new KeyPreference(this);
			pref.setKey(ChaosView.KEY_CODE_PREFS[i]);
			pref.setTitle(ChaosView.KEY_CODE_TITLES[i]);
			pref.setDefaultValue(ChaosView.DEFAULT_KEY_CODES[i]);
			group.addPreference(pref);
		}

	}

}
