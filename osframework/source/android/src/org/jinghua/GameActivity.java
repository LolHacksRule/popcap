package org.jinghua;

import org.jinghua.GameView;
import org.jinghua.ACPManager;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;

public class GameActivity extends Activity
{
    private static String TAG = "GameActivity";

    protected GameView mView;

    /** Called when the activity is first created. */
    @Override protected void onCreate(Bundle icicle) {
	ACPManager.getInstance().setAssets(getAssets(), "files/");

	getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
			     WindowManager.LayoutParams.FLAG_FULLSCREEN);
	requestWindowFeature(Window.FEATURE_NO_TITLE);

        super.onCreate(icicle);
        mView = new GameView(getApplication(), this);
        setContentView(mView);
    }

    public void shutdown()
    {
	this.finish();
    }

    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	super.onActivityResult(requestCode, resultCode, data);
	if (requestCode == 123)
	    this.finish();
    }
}
