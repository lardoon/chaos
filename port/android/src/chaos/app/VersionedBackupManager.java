package chaos.app;

import android.annotation.TargetApi;
import android.app.backup.BackupManager;
import android.content.Context;
import android.os.Build;

/**
 * Use BackupManager on 2.2+ without breaking 2.1 and earlier.
 */
public abstract class VersionedBackupManager {

	public abstract void dataChanged();

	private static class FroyoBackupManager extends VersionedBackupManager {
		@TargetApi(Build.VERSION_CODES.FROYO)
		FroyoBackupManager(Context context) {
			m_backupManager = new BackupManager(context);
		}

		@TargetApi(Build.VERSION_CODES.FROYO)
		public void dataChanged() {
			m_backupManager.dataChanged();
		}

		private BackupManager m_backupManager;
	}

	private static class EclairBackupManager extends VersionedBackupManager {
		public void dataChanged() {
			/* not supported */
		}
	}

	public static VersionedBackupManager newInstance(Context context) {
		final int sdkVersion = Integer.parseInt(Build.VERSION.SDK);
		VersionedBackupManager backupManager = null;
		if (sdkVersion < Build.VERSION_CODES.FROYO) {
			backupManager = new EclairBackupManager();
		} else {
			backupManager = new FroyoBackupManager(context);
		}
		return backupManager;
	}
}
