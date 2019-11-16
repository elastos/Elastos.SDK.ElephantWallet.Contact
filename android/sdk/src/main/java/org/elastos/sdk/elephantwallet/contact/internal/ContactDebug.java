package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import org.elastos.tools.crosspl.CrossBase;
import org.elastos.tools.crosspl.annotation.CrossClass;
import org.elastos.tools.crosspl.annotation.CrossInterface;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;

import static org.elastos.sdk.elephantwallet.contact.Contact.TAG;

@CrossClass
public class ContactDebug extends CrossBase {
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
            Log.i(TAG,"exception="+e.getMessage());
        }
    }

    @CrossInterface
    public static native int GetCachedDidProp(StringBuffer value);
} // class Factory
