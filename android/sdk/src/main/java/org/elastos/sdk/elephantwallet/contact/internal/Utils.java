package org.elastos.sdk.elephantwallet.contact.internal;

import android.util.Log;

import org.elastos.sdk.elephantwallet.contact.Contact;

import java.io.File;
import java.io.FileInputStream;
import java.lang.reflect.Field;
import java.math.BigInteger;
import java.security.MessageDigest;

public final class Utils {
    private Utils() { }

    public static String ToString(Object obj) {
        StringBuilder result = new StringBuilder();

        result.append(obj.getClass().getName());
        result.append("{");

        Field[] fields = obj.getClass().getDeclaredFields();
        for ( Field field : fields  ) {
            field.setAccessible(true);
            result.append(field.getName());
            result.append("=");
            try {
                Object val = field.get(obj);
                result.append(val);
            } catch (Exception e) {
                Log.e(Contact.TAG, "Failed to print object to string", e);
                result.append("unknown");
            }
            result.append(";");
        }

        result.append("}");

        return result.toString();
    }

    public static String getMD5Sum(File file) {
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            FileInputStream fis = new FileInputStream(file);
            byte[] buffer = new byte[8192];
            int num;

            while( (num = fis.read(buffer)) > 0) {
                md.update(buffer, 0, num);
            }
            fis.close();
            String md5 = new BigInteger(1, md.digest()).toString(16);
            return fillMD5(md5);
        } catch (Exception e) {
            Log.e(Contact.TAG, "Failed to get md5. file:" + file.getAbsolutePath(), e);
        }

        return null;
    }

    public static String getMd5Sum(String str) {
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            md.update(str.getBytes());
            String md5 = new BigInteger(1, md.digest()).toString(16);
            return fillMD5(md5);
        } catch (Exception e) {
            throw new RuntimeException("Failed to get md5.", e);
        }
    }

    private static String fillMD5(String md5) {
        return md5.length() == 32 ? md5 : fillMD5("0" + md5);
    }
}

