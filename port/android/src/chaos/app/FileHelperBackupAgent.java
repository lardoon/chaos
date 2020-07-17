
package chaos.app;

import java.io.IOException;

import android.annotation.TargetApi;
import android.app.backup.BackupAgentHelper;
import android.app.backup.BackupDataInput;
import android.app.backup.BackupDataOutput;
import android.app.backup.FileBackupHelper;
import android.os.Build;
import android.os.ParcelFileDescriptor;

/**
 * Backup game and option files.
 * Only used on 2.2+, aka android-8, aka Froyo.
 */
@TargetApi(Build.VERSION_CODES.FROYO)
public class FileHelperBackupAgent extends BackupAgentHelper {
    static final String FILE_DATA_KEY = "myfiles";

    @Override
    public void onCreate() {
        FileBackupHelper gameHelper = new FileBackupHelper(this, ChaosView.GAME_FILE, ChaosView.OPTIONS_FILE);
        addHelper(FILE_DATA_KEY, gameHelper);
    }

    @Override
    public void onBackup(ParcelFileDescriptor oldState, BackupDataOutput data,
             ParcelFileDescriptor newState) throws IOException {
        synchronized (ChaosView.s_dataLock) {
            super.onBackup(oldState, data, newState);
        }
    }

    @Override
    public void onRestore(BackupDataInput data, int appVersionCode,
            ParcelFileDescriptor newState) throws IOException {
        synchronized (ChaosView.s_dataLock) {
            super.onRestore(data, appVersionCode, newState);
        }
    }
}
