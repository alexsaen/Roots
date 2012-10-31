/*
	Copyright (c) 2012, Alexey Saenko
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

package com.stronggames.roots;

import com.stronggames.roots.R;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.WindowManager.LayoutParams;

public class GameActivity extends Activity {
 
	static {
	    System.loadLibrary("Game");
	}
	
	private GameSurfaceView surfaceView=null;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
  
        setRequestedOrientation (ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		
		setContentView(R.layout.main);
        surfaceView = (GameSurfaceView) findViewById(R.id.mainview);        
    }

	@Override 
	public void onResume() {
	    surfaceView.onResume();		
		super.onResume();
	}

	@Override 
	public void onPause() {
		surfaceView.onPause();
		super.onPause();
	}

	@Override
	public void onBackPressed() {
		surfaceView.onBackPressed();
	}
	
}
	
	