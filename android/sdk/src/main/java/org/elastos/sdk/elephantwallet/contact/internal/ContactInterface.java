package org.elastos.sdk.elephantwallet.contact.internal;

import org.elastos.sdk.elephantwallet.contact.Utils;

import java.io.File;

public interface ContactInterface {
    public static class Factory extends ContactFactory {
        static { Utils.EnsureNativeLibrary(); }
    } // class Factory

    public static class Bridge extends ContactBridge {
        static {
            Utils.EnsureNativeLibrary();
            Channel channel = Channel.Carrier; // make sure Channel generated
            Status status = Status.Invalid; // make sure Status generated
        }
    } // class Bridge

    public abstract static class Listener extends ContactListener {
        static { Utils.EnsureNativeLibrary(); }
    } // class Listener

    public abstract static class DataListener extends ContactDataListener {
        static { Utils.EnsureNativeLibrary(); }
    } // class DataListener

    public static final class Status extends ContactStatus<Status> {
        public static final Status Invalid = new Status(0x0, "Invalid");
        public static final Status WaitForAccept = new Status(0x1, "WaitForAccept");
        public static final Status Offline = new Status(0x2, "Offline");
        public static final Status Online = new Status(0x4, "Online");
        public static final Status Removed = new Status(0x8, "Removed");

        private Status(int id, String name) {
            super(id, name);
        }
        static { Utils.EnsureNativeLibrary(); }
    } // class Channel

    public static class HumanInfo extends org.elastos.sdk.elephantwallet.contact.internal.HumanInfo {
        static { Utils.EnsureNativeLibrary(); }
    } // class HumanInfo

    public static final class UserInfo extends org.elastos.sdk.elephantwallet.contact.internal.UserInfo {
        static { Utils.EnsureNativeLibrary(); }
    } // class UserInfo

    public static final class FriendInfo extends org.elastos.sdk.elephantwallet.contact.internal.FriendInfo {
        static { Utils.EnsureNativeLibrary(); }
    } // class FriendInfo

    public static final class Channel extends ContactChannel<Channel> {
        public static final Channel Carrier = new Channel(1, "Carrier");
        public static final Channel ElaChain = new Channel(2, "ElaChain");
        public static final Channel Email = new Channel(3, "Email");

        private Channel(int id, String name) {
            super(id, name);
        }
        static { Utils.EnsureNativeLibrary(); }
    } // class Channel

    public static class Message extends ContactMessage {
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

        static { Utils.EnsureNativeLibrary(); }
    } // class Message

    public abstract static class Debug extends ContactDebug {
        static { Utils.EnsureNativeLibrary(); }
    } // class Debug
}
