package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

@CrossClass
public class ContactFactory extends CrossBase {
    @CrossInterface
    public static native void SetLogLevel(int level);

    @CrossInterface
    public static native void SetDeviceId(String devId);

    @CrossInterface
    public static native int SetLocalDataDir(String dir);

    protected ContactFactory() {
        super(ContactFactory.class.getName(), 0);
    }

    static {
        Log.d(ContactBridge.TAG, "Loading libElastos.SDK.Contact.Jni.so ...");
        System.loadLibrary("Elastos.SDK.Contact.Jni");
        Log.d(ContactBridge.TAG, "Loaded libElastos.SDK.Contact.Jni.so ...");
    }
} // class Factory
