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

import java.io.DataInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.WindowManager;

public class GameSurfaceView extends GLSurfaceView {

	private Engine engine;
	
	Context context;
	
	public GameSurfaceView(Context context, AttributeSet attrs) {
		super(context, attrs);		
		this.context = context;
		
	    setEGLContextClientVersion(2);        
        engine = new Engine();
        setRenderer(engine);
        
	}
	
	class Engine implements Renderer {
	   	 
    	public Engine() {
    		String apkFilePath = null;
    		ApplicationInfo appInfo = null;
    		PackageManager packMgmr = context.getPackageManager();
    		try {
    			appInfo = packMgmr.getApplicationInfo(getClass().getPackage().getName(), 0);
    		} catch (NameNotFoundException e) {
    			e.printStackTrace();
    			throw new RuntimeException("Unable to locate assets, aborting...");
    		}
    		apkFilePath = appInfo.sourceDir;
    		init(apkFilePath);
    	}    		

        public void onDrawFrame(GL10 gl) {
        	draw();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
	    	WindowManager windowManager =(WindowManager)context.getSystemService( Context.WINDOW_SERVICE );
	    	Display display = windowManager.getDefaultDisplay();	    	
	    	DisplayMetrics displayMetrics=new DisplayMetrics();
	    	display.getMetrics(displayMetrics);	    	
	    	
        	reshape(display.getOrientation(), width, height, displayMetrics.density);
        }

		public void onSurfaceLost() {
			release();
		}  

		public void onSurfaceCreated(GL10 gl, EGLConfig config) 	{
        }
				
		public native void 		init(String apkPath);
		public native void 		reshape(int orientation, int width, int height, float density);	        		
		public native void 		release();	        
		public native void 		suspend();	        
		public native void 		resume();	        
		public native void 		destroy();	        
		public native void 		draw();	        
		public native void 		onTouch(int id, int action, float x, float y);	
		public native void 		onKey(int action, int keyCode);	
		
		final String	SettingsFileName = "settings";
		
		public	void	saveSettings(byte[] s) {
			try {
				FileOutputStream fos = context.openFileOutput(SettingsFileName, Context.MODE_PRIVATE);
				fos.write(s);
				fos.close();			
			} catch(IOException e) {}
		}

		public byte[] loadSettings() {
		   try {
		        DataInputStream dis = new DataInputStream(context.openFileInput(SettingsFileName));
		        byte[] theBytes = new byte[dis.available()];
		        dis.read(theBytes, 0, dis.available());
		        dis.close();
		        return theBytes;
		    } catch (IOException ex) {}	
		   	return null;
		}
		
		public void onExit() {
			((android.app.Activity)context).finish();
		}
	}

	@Override
    public void onPause() {
		super.onPause();
		engine.suspend();
    }

	@Override
    public void onResume() {
		engine.resume();
		super.onResume();
    }

	@Override
	protected void onDetachedFromWindow() {
		super.onDetachedFromWindow();
    	engine.destroy();
    }
    
	@Override
	public boolean onTouchEvent(MotionEvent event) {
//		MotionEvent.ACTION_DOWN = 0
//		MotionEvent.ACTION_UP 	= 1
//		MotionEvent.ACTION_MOVE = 2
				
		class TouchEvent implements Runnable {
			MotionEvent event;
	
			public 		TouchEvent(MotionEvent e) { 
				event = e; 
			}
			
			public void run() { 
				for(int i=0; i<event.getPointerCount(); ++i) {
					int action = event.getAction() & MotionEvent.ACTION_MASK;
					int id = event.getPointerId(i);
					if(action > 2)
						continue;
					engine.onTouch( id, action, event.getX(i), event.getY(i) );	
				}
			}
		};
		
		queueEvent( new TouchEvent(event) );
		
		return true;
	}

	class OnKeyEvent implements Runnable {
		KeyEvent 	event;

		public 		OnKeyEvent(KeyEvent e) 	{ 
			event = e; 
		}

		public void run() { 
			engine.onKey( event.getAction(), event.getKeyCode() );	
		}
	};
	
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		queueEvent( new OnKeyEvent(event) );
		return super.onKeyDown(keyCode, event);		
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		queueEvent( new OnKeyEvent(event) );
		return super.onKeyUp(keyCode, event);		
	}
	
	public void onBackPressed() {
		queueEvent( new OnKeyEvent( new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_BACK) ) );
	}
}
