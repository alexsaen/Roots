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

#include <jni.h>
#include "Chapter.h"
#include "ResourceManager.h"
#include "Sound.h"
#include "Settings.h"

static Main *game = 0;

static JavaVM *javaVM = 0;
static jobject javaObj = 0;

static jmethodID loadSettingsMethod = 0, saveSettingsMethod = 0, onExitMethod = 0;

struct JNIEnvironment {
	JNIEnv *env;
	bool isAttached;
	JNIEnvironment(): env(0), isAttached(false) {
		if(javaVM->GetEnv((void **)&env, JNI_VERSION_1_4) < 0) {
			if(javaVM->AttachCurrentThread(&env, 0) < 0)
				return;
			isAttached = true;
		}
	}
	~JNIEnvironment() {
		if(isAttached)
			javaVM->DetachCurrentThread();
	}
	JNIEnv *operator->() 	{	return env;		}
	bool	operator!() 	{	return env==0;	}
};

JNIEXPORT void JNICALL nativeInit(JNIEnv *env, jobject obj, jstring apkPath) {
	env->GetJavaVM(&javaVM);
	javaObj = env->NewGlobalRef(obj);

	const char* str;
	jboolean isCopy;
	str = env->GetStringUTFChars(apkPath, &isCopy);
	game = new Main(str);
}

JNIEXPORT void JNICALL nativeDraw(JNIEnv *, jobject) {
	if(game->finished()) {
		if(!onExitMethod)
			return;
		JNIEnvironment env;
		if(!env)
			return;
		env->CallVoidMethod(javaObj, onExitMethod);
	}
	game->draw();
}

JNIEXPORT void JNICALL nativeRelease(JNIEnv *, jobject) {
	game->release();
}

JNIEXPORT void JNICALL nativeSuspend(JNIEnv *, jobject) {
	game->suspend();
}

JNIEXPORT void JNICALL nativeResume(JNIEnv *, jobject) {
	game->resume();
}

JNIEXPORT void JNICALL nativeDestroy(JNIEnv *, jobject) {
	delete game;
	game = 0;
}

JNIEXPORT void JNICALL nativeReshape(JNIEnv *env, jobject obj, jint orientation, jint width, jint height, jfloat density) {
	game->reshape(width, height, density);
}

JNIEXPORT void JNICALL nativeOnTouch(JNIEnv *env, jobject obj, jint id, jint action, jfloat x, jfloat y) {
	switch(action) {
		case 0:
			game->touchBegan(id, x, y);
			break;
		case 1:
			game->touchEnded(id);
			break;
		case 2:
			game->touchMove(id, x, y);
			break;
	}
}

JNIEXPORT void JNICALL nativeOnKey(JNIEnv *env, jobject obj, jint action, jint keyCode) {
	if(action == 0 )
		game->keyDown(keyCode);
}

#define JAVA_NATIVE_CLASS "com/stronggames/roots/GameSurfaceView$Engine"

static JNINativeMethod methods[] = {
	 { "init",				"(Ljava/lang/String;)V",(void*) nativeInit	},
	 { "draw",				"()V", 		(void*)nativeDraw 				},
	 { "release",			"()V", 		(void*)nativeRelease 			},
	 { "suspend",			"()V", 		(void*)nativeSuspend 			},
	 { "resume",			"()V", 		(void*)nativeResume 			},
	 { "destroy",			"()V", 		(void*)nativeDestroy 			},
	 { "reshape", 			"(IIIF)V", 	(void*)nativeReshape  			},
	 { "onTouch", 			"(IIFF)V", 	(void*)nativeOnTouch  			},
	 { "onKey", 			"(II)V", 	(void*)nativeOnKey  			}
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	javaVM = vm;
	JNIEnv *env = 0;
	if( vm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK)
		return -1;

	jclass classNative = env->FindClass(JAVA_NATIVE_CLASS);
	if( !classNative )
		return -1;

	if( env->RegisterNatives(classNative, methods, sizeof(methods)/sizeof(methods[0])) < 0)
		return -1;

	saveSettingsMethod = env->GetMethodID(classNative, "saveSettings", "([B)V");
	if(!saveSettingsMethod)
		return -1;

	loadSettingsMethod = env->GetMethodID(classNative, "loadSettings", "()[B");
	if(!loadSettingsMethod)
		return -1;

	onExitMethod = env->GetMethodID(classNative, "onExit", "()V");
	if(!onExitMethod)
		return -1;

	return JNI_VERSION_1_4;
}

namespace platform {

std::string	loadSettings() {
	std::string result;

	if(!loadSettingsMethod)
		return result;
	JNIEnvironment env;
	if(!env)
		return result;

	jbyteArray bytes = (jbyteArray)env->CallObjectMethod(javaObj, loadSettingsMethod);

	if(bytes != NULL) {
		int len = env->GetArrayLength(bytes);
        char *value = (char*)env->GetByteArrayElements(bytes, 0);
        result = value;
        env->ReleaseByteArrayElements(bytes, (jbyte*)value, 0);
    }
	return result;
}

void saveSettings(const std::string &data) {
	if(!saveSettingsMethod)
		return;
	JNIEnvironment env;
	if(!env)
		return;
	size_t sz = data.length()+1;
	jbyteArray bytes = env->NewByteArray(sz);
	env->SetByteArrayRegion(bytes, 0, sz, (const jbyte*)data.c_str());

	env->CallVoidMethod(javaObj, saveSettingsMethod, bytes);
}

};


