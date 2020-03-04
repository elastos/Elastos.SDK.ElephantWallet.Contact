package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import org.elastos.sdk.elephantwallet.contact.Utils;
import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

@CrossClass
class ContactFactory extends CrossBase {
    @CrossInterface
    public static native void SetLogLevel(int level);

    @CrossInterface
    public static native void SetDeviceId(String devId);

    @CrossInterface
    public static native int SetLocalDataDir(String dir);

    protected ContactFactory() {
        super(ContactFactory.class.getName(), 0);
    }

    static { Utils.EnsureNativeLibrary(); }
} // class Factory
