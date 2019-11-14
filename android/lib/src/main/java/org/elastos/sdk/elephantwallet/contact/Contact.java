package org.elastos.sdk.elephantwallet.contact;

import org.elastos.sdk.elephantwallet.contact.internal.ContactBridge;
import org.elastos.sdk.elephantwallet.contact.internal.ContactChannel;
import org.elastos.sdk.elephantwallet.contact.internal.ContactDataListener;
import org.elastos.sdk.elephantwallet.contact.internal.ContactDebug;
import org.elastos.sdk.elephantwallet.contact.internal.ContactFactory;
import org.elastos.sdk.elephantwallet.contact.internal.ContactListener;

import java.io.File;

public final class Contact extends ContactBridge {
    public static final String TAG = "elastos";

    public static final class Factory extends ContactFactory {
        public static Contact Create() {
            return new Contact();
        }

        public static void SetDeviceId(String devId) {
            ContactFactory.SetDeviceId(devId);
            UserInfo.SetCurrDevId(devId);
        }

        private Factory() { }

        static { ensureNativeLibrary(); }
    } // class Factory

    public abstract static class Listener extends ContactListener {
        static { ensureNativeLibrary(); }
    } // class Listener

    public abstract static class DataListener extends ContactDataListener {
        static { ensureNativeLibrary(); }
    } // class DataListener

    public static final class UserInfo extends org.elastos.sdk.elephantwallet.contact.internal.UserInfo {
        static { ensureNativeLibrary(); }
    } // class UserInfo

    public static final class FriendInfo extends org.elastos.sdk.elephantwallet.contact.internal.FriendInfo {
        static { ensureNativeLibrary(); }
    } // class FriendInfo

//    public abstract static class Channel extends ContactChannel {
//        static { ensureNativeLibrary(); }
//    } // class Channel

    public static class Message extends org.elastos.sdk.elephantwallet.contact.internal.ContactMessage {
        public Message(String text, String cryptoAlgorithm) {
            super(text, cryptoAlgorithm);
        }

        public Message(byte[] binary, String cryptoAlgorithm) {
            super(binary, cryptoAlgorithm);
        }

        public Message(File file, String cryptoAlgorithm) {
            super(file, cryptoAlgorithm);
        }

        public Message(Type type, byte[] data, String cryptoAlgorithm) {
            super(type, data, cryptoAlgorithm);
        }

        static { ensureNativeLibrary(); }
    } // class Message

    public abstract static class Debug extends ContactDebug {
        static { ensureNativeLibrary(); }
    } // class Listener

    private Contact() {
    }

    static { ensureNativeLibrary(); }

    private static void ensureNativeLibrary() {
        System.loadLibrary("Elastos.SDK.Contact.Jni");
    }

}
