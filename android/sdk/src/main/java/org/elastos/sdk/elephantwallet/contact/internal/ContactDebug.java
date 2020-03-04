package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

import org.elastos.sdk.elephantwallet.contact.Utils;

@CrossClass
class ContactDebug extends CrossBase {
    public static final class Keypair extends KeypairWrapper {
        private Keypair() {}
    } // class Listener

    protected ContactDebug() {
        super(ContactDebug.class.getName(), 0);
    }

    public static void DumpLocalRefTables() {
        try {
            Class cls = Class.forName("android.os.Debug");
            Method method = cls.getDeclaredMethod("dumpReferenceTables");
            Constructor con= cls.getDeclaredConstructor();
            con.setAccessible(true);
            method.invoke(con.newInstance());
        }
        catch(Exception e){
            Log.i(Utils.TAG,"exception="+e.getMessage());
        }
    }

    @CrossInterface
    public static native int GetCachedDidProp(StringBuffer value);
} // class Factory
